#include "notifier.hpp"

void Notifier::subscribe(std::string group, std::string clientId) {
    groups[group].push_back(clientId);
}

void Notifier::unsubscribe(std::string group, std::string clientId) {
    auto g  = groups[group];
    auto it = find(g.begin(), g.end(), clientId);

    if (it == g.end())
        return;

    g.erase(it);
}

void Notifier::notifyUser(std::string clientId, std::string message) {
    auto &sessions = Server::sessions_;
    if (sessions.find(clientId) == sessions.end()) {
        return;
    }

    auto s = sessions[clientId];
    Server::enqueue_reply(s->fd, s, message);
}

void Notifier::notifyGroup(std::string group, std::string message) {
    auto &sessions = Server::sessions_;
    if (groups.find(group) == groups.end())
        return;

    for (auto &cid : groups[group]) {
        if (sessions.find(cid) == sessions.end()) {
            continue;
        }
        auto s = sessions[cid];
        Server::enqueue_reply(s->fd, s, message);
    }
}

void Notifier::registerGroup(std::string group) {
    if (groups.find(group) == groups.end()) {
        groups[group] = {};
    }
}