#pragma once
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "network.hpp"

class Notifier {
   public:
    static Notifier &instance() {
        static Notifier inst;
        return inst;
    }

    void subscribe(std::string group, std::string clientId);
    void unsubscribe(std::string group, std::string clientId);
    void notifyUser(std::string clientId, std::string message);
    void notifyGroup(std::string group, std::string message);
    void registerGroup(std::string group);
    void removeGroup(std::string group);

    std::unordered_map<std::string, std::vector<std::string>> groups;

   private:
    Notifier() = default;
};