#pragma once
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

class EventBus {
public:
    using Callback = std::function<void()>;

    static EventBus& instance() {
        static EventBus inst;
        return inst;
    }

    void subscribe(const std::string& topic, Callback cb) {
        std::lock_guard<std::mutex> g(mu_);
        listeners_[topic].push_back(std::move(cb));
    }

    void publish(const std::string& topic) {
        std::vector<Callback> copy;
        {
            std::lock_guard<std::mutex> g(mu_);
            auto it = listeners_.find(topic);
            if (it == listeners_.end()) return;
            copy = it->second;
        }
        for (auto &cb : copy) if (cb) cb();
    }

private:
    EventBus() = default;
    std::mutex mu_;
    std::unordered_map<std::string, std::vector<Callback>> listeners_;
};
