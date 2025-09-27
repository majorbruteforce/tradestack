#pragma once

#include <unistd.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "manager.hpp"

struct Session {
    int                                   fd;
    std::string                           inbuf;
    std::string                           outbuf;
    std::chrono::seconds                  timeout;
    std::chrono::steady_clock::time_point last_active;

    bool is_authenticated = false;

    std::string client_id;

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
using Processor = std::function<void(int /*fd*/,
                                     std::shared_ptr<Session>& /*session*/,
                                     std::vector<std::string>& /*parts*/,
                                     std::string /*clientId*/
                                     )>;

class Server {
   public:
    static Manager manager;

    Server(uint16_t port, int max_events = 64) : port_(port), max_events_(max_events) {}

    ~Server() { stop(); }

    bool start();
    void stop();
    void run();

   private:
    uint16_t   port_;
    int        max_events_;
    static int epoll_fd_;
    static int listen_fd_;

    static std::map<int, std::shared_ptr<Session>>         temp_sessions_;
    static std::map<std::string, std::shared_ptr<Session>> sessions_;
    static std::unordered_map<std::string, Processor>      processors_;

    static void accept_new();
    static void cleanup_stale();
    static bool handle_read(int fd);
    static bool handle_write(int fd);
    static void modify_epoll_out(int fd, bool enable);

    static void process_session_messages(int fd, std::string clientId);
    static void remove_session(int fd);
    static void dispatch(std::string&              cmd,
                         int                       fd,
                         std::shared_ptr<Session>& s,
                         std::vector<std::string>& parts,
                         std::string               clientId);
    static void enqueue_reply(int fd, std::shared_ptr<Session>& s, const std::string& reply);
    static void load_processors();
    static void register_processor(std::string cmd, Processor p);

    friend class Notifier;
};