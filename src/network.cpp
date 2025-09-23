#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <utils/string.hpp>
#include "network.hpp"
#include "utils/time.hpp"

using namespace std::chrono_literals;

const std::chrono::seconds SESSION_TIMEOUT = 60s;

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static bool verify_token(const std::string &token, const std::string &clientId) {
    (void)token;
    (void)clientId;
    return true;
}

bool Server::start() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        perror("socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt SO_REUSEADDR");
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port_);

    if (bind(listen_fd_, (sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return false;
    }

    if (listen(listen_fd_, SOMAXCONN) < 0) {
        perror("listen");
        return false;
    }

    if (set_nonblocking(listen_fd_) < 0) {
        perror("set_nonblocking");
        return false;
    }

    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        perror("epoll_create1");
        return false;
    }

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = listen_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) < 0) {
        perror("epoll_ctl add listen_fd");
        return false;
    }

    load_processors();

    std::cout << now_str() << " Server listening on port " << port_ << "\n";
    return true;
}

void Server::stop() {
    for (auto &p : temp_sessions_) {
        p.second->close_fd();
    }
    temp_sessions_.clear();

    for (auto &p : sessions_) {
        p.second->close_fd();
    }
    sessions_.clear();

    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

void Server::run() {
    std::vector<epoll_event> events(max_events_);

    while (true) {
        int n = epoll_wait(epoll_fd_, events.data(), (int)events.size(), 1000);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; ++i) {
            auto &ev = events[i];
            if (ev.data.fd == listen_fd_) {
                accept_new();
            } else {
                int fd = ev.data.fd;
                if (ev.events & (EPOLLERR | EPOLLHUP)) {
                    std::cerr << now_str() << " EPOLLERR/HUP on fd " << fd << "\n";
                    remove_session(fd);
                    continue;
                }
                if (ev.events & EPOLLIN) {
                    if (!handle_read(fd)) {
                        remove_session(fd);
                        continue;
                    }
                }
                if (ev.events & EPOLLOUT) {
                    if (!handle_write(fd)) {
                        remove_session(fd);
                        continue;
                    }
                }
            }
        }

        cleanup_stale();
    }
}

void Server::accept_new() {
    while (true) {
        sockaddr_in client{};
        socklen_t   clen      = sizeof(client);
        int         client_fd = accept4(listen_fd_, (sockaddr *)&client, &clen, SOCK_NONBLOCK);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            perror("accept4");
            break;
        }

        char ipbuf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, ipbuf, sizeof(ipbuf));
        uint16_t rport = ntohs(client.sin_port);
        std::cout << now_str() << " Accepted " << ipbuf << ":" << rport << " fd=" << client_fd
                  << "\n";

        auto s = std::make_shared<Session>(client_fd, SESSION_TIMEOUT);

        temp_sessions_[client_fd] = s;

        epoll_event ev{};
        ev.events  = EPOLLIN;
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
            perror("epoll_ctl add client_fd");
            s->close_fd();
            temp_sessions_.erase(client_fd);
            continue;
        }
    }
}

bool Server::handle_read(int fd) {
    auto it = temp_sessions_.find(fd);
    if (it == temp_sessions_.end())
        return false;

    auto s = it->second;
    char buf[4096];
    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n > 0) {
            s->inbuf.append(buf, buf + n);
            s->touch();

            process_session_messages(fd);
        } else if (n == 0) {
            std::cout << now_str() << " fd=" << fd << " closed by peer\n";
            return false;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("recv");
                return false;
            }
        }
    }

    if (!s->outbuf.empty()) {
        modify_epoll_out(fd, true);
    }
    return true;
}

bool Server::handle_write(int fd) {
    auto it = temp_sessions_.find(fd);
    if (it == temp_sessions_.end())
        return false;
    auto s = it->second;

    while (!s->outbuf.empty()) {
        ssize_t n = send(fd, s->outbuf.data(), s->outbuf.size(), 0);
        if (n > 0) {
            s->outbuf.erase(0, n);
            s->touch();
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else if (errno == EINTR) {
                continue;
            } else {
                perror("send");
                return false;
            }
        }
    }

    if (s->outbuf.empty()) {
        modify_epoll_out(fd, false);
    }
    return true;
}

void Server::modify_epoll_out(int fd, bool enable) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.events  = EPOLLIN | (enable ? EPOLLOUT : 0);
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        if (errno != ENOENT)
            perror("epoll_ctl mod");
    }
}

void Server::remove_session(int fd) {
    auto it = temp_sessions_.find(fd);
    if (it == temp_sessions_.end())
        return;
    auto s = it->second;
    std::cout << now_str() << " Removing session fd=" << fd << "\n";

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        if (errno != ENOENT)
            perror("epoll_ctl del");
    }

    if (s->is_authenticated && !s->client_id.empty()) {
        auto sit = sessions_.find(s->client_id);
        if (sit != sessions_.end()) {
            // only erase if it's the same session object
            if (sit->second == s) {
                sessions_.erase(sit);
            }
        }
    }

    s->close_fd();
    temp_sessions_.erase(it);
}

void Server::cleanup_stale() {
    std::vector<int> to_close;
    for (auto &p : temp_sessions_) {
        if (p.second->is_stale())
            to_close.push_back(p.first);
    }
    for (int fd : to_close) remove_session(fd);
}

void Server::process_session_messages(int fd) {
    auto it = temp_sessions_.find(fd);
    if (it == temp_sessions_.end())
        return;
    auto s = it->second;

    while (true) {
        auto pos = s->inbuf.find('\n');
        if (pos == std::string::npos)
            break;
        std::string line = s->inbuf.substr(0, pos);
        s->inbuf.erase(0, pos + 1);

        line = trim(line);
        if (line.empty())
            continue;

        auto parts = split_ws(line);
        if (parts.empty())
            continue;

        to_upper(parts[0]);

        for (int i = 0; i < parts.size(); i++) {
            if (i != 0 && iequals(parts[i - 1], "AUTH"))
                continue;
            to_upper(parts[i]);
        }
        dispatch(parts[0], fd, s, parts);
    }
}

void Server::register_processor(std::string cmd, Processor p) {
    processors_[cmd] = std::move(p);
}

void Server::dispatch(std::string              &cmd,
                      int                       fd,
                      std::shared_ptr<Session> &s,
                      std::vector<std::string> &parts) {
    if (parts.empty())
        return;
    to_upper(cmd);
    auto it = processors_.find(cmd);
    if (it != processors_.end())
        it->second(fd, s, parts);
    else
        enqueue_reply(fd, s, "ERR UNKNOWN_CMD\n");
}

void Server::enqueue_reply(int fd, std::shared_ptr<Session> &s, const std::string &reply) {
    s->outbuf += reply;
    modify_epoll_out(fd, true);
}
