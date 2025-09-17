#pragma once

#include <unistd.h>

#include <chrono>
#include <map>
#include <memory>
#include <string>

struct Session {
    int                                   fd;
    std::string                           inbuf;
    std::string                           outbuf;
    std::chrono::seconds                  timeout;
    std::chrono::steady_clock::time_point last_active;

    Session(int fd_, std::chrono::seconds timeout_)
        : fd(fd_), timeout(timeout_), last_active(std::chrono::steady_clock::now()) {}

    void touch() { last_active = std::chrono::steady_clock::now(); }

    bool is_stale() { return (std::chrono::steady_clock::now() - last_active) > timeout; }

    void close_fd() {
        if (fd >= 0) {
            close(fd);
            fd = -1;
        }
    }
};

class Server {
   public:
    Server(uint16_t port, int max_events = 64)
        : port_(port), epoll_fd_(-1), listen_fd_(-1), max_events_(max_events) {}

    ~Server() { stop(); }

    bool start();
    void stop();
    void run();

   private:
    uint16_t                                port_;
    int                                     epoll_fd_;
    int                                     listen_fd_;
    int                                     max_events_;
    std::map<int, std::shared_ptr<Session>> sessions_;

    void accept_new();
    bool handle_read(int fd);
    bool handle_write(int fd);
    void modify_epoll_out(int fd, bool enable);
    void remove_session(int fd);
    void cleanup_stale();
};