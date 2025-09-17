#pragma once

#include <unistd.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

struct Session {
    int                                   fd;
    std::string                           inbuf;
    std::string                           outbuf;
    std::chrono::seconds                  timeout;
    std::chrono::steady_clock::time_point last_active;

    bool is_authenticated = false;

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
using Processor = std::function<void(
        int /*fd*/, std::shared_ptr<Session>& /*session*/, std::vector<std::string>& /*parts*/
        )>;
class Server {
   public:
    Server(uint16_t port, int max_events = 64)
        : port_(port), epoll_fd_(-1), listen_fd_(-1), max_events_(max_events) {}

    ~Server() { stop(); }

    bool start();
    void stop();
    void run();

   private:
    uint16_t                                   port_;
    int                                        epoll_fd_;
    int                                        listen_fd_;
    int                                        max_events_;
    std::map<int, std::shared_ptr<Session>>    sessions_;
    std::unordered_map<std::string, Processor> processors_;

    void accept_new();
    void cleanup_stale();
    bool handle_read(int fd);
    bool handle_write(int fd);
    void modify_epoll_out(int fd, bool enable);

    void process_session_messages(int fd);
    void remove_session(int fd);
    void dispatch(std::string&              cmd,
                  int                       fd,
                  std::shared_ptr<Session>& s,
                  std::vector<std::string>& parts);
    void enqueue_reply(int fd, std::shared_ptr<Session>& s, const std::string& reply);
    void load_processors();
    void register_processor(std::string cmd, Processor p);
};