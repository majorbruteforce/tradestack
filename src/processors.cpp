#include "network.hpp"
#include "utils/string.hpp"
#include "utils/time.hpp"

const std::string DEBUG_SECRET = "123456";
const std::string EASTER_EGG   = "passkey";

void Server::load_processors() {
    const std::string PING  = "PING";
    const std::string DEBUG = "DEBUG";
    const std::string NEWL  = "NEWL";
    const std::string AUTH  = "AUTH";

    register_processor(
            PING, [this](int fd, std::shared_ptr<Session> &s, std::vector<std::string> &parts) {
                (void)parts;
                enqueue_reply(fd, s, "PONG\n");
            });

    register_processor(
            DEBUG, [&, this](int fd, std::shared_ptr<Session> &s, std::vector<std::string> &parts) {
                if (parts.size() >= 3 && parts[1] == "AUTH") {
                    if (parts[2] == DEBUG_SECRET) {
                        s->is_authenticated = true;
                        enqueue_reply(fd, s, "AUTHORIZED\n");
                    } else {
                        enqueue_reply(fd, s, "BAD_SECRET\n");
                    }

                } else if (s->is_authenticated) {
                    if (parts.size() >= 2 && parts[1] == "LIST") {
                        std::ostringstream oss;
                        oss << "At: " << now_str() << "\n";
                        oss << "Sessions(" << sessions_.size() << ")\n";
                        for (auto &s : sessions_) {
                            oss << s.first << " "
                                << "Authenticated: " << s.second->is_authenticated << "\n"
                                << "Client ID: " << s.second->client_id << "\n";
                        }

                        enqueue_reply(fd, s, oss.str());
                    }

                    if (parts.size() >= 2 && parts[1] == "INSTRUMENTS") {
                        std::ostringstream oss;
                        oss << "At: " << now_str() << "\n";
                        oss << "Instruments(" << manager.instruments_.size() << ")\n";
                        for (auto &i : manager.instruments_) {
                            oss << "--------------------------------------\n";
                            oss << i.first << ":\n";
                            oss << "    "
                                << "LTP: " << i.second->getLastTradePrice() << "\n";
                            oss << "    "
                                << "LTS: " << i.second->getLastTradeSize() << "\n";
                            oss << "    "
                                << "LTT: " << i.second->getLastTradeTimestamp() << "\n";
                            oss << "    "
                                << "High: " << i.second->getHigh() << "\n";
                            oss << "    "
                                << "Low: " << i.second->getLow() << "\n";
                            oss << "    "
                                << "Open: " << i.second->getOpen() << "\n";
                            oss << "    "
                                << "Close: " << i.second->getClose() << "\n";
                            oss << "--------------------------------------\n";
                        }

                        enqueue_reply(fd, s, oss.str());
                    }

                } else {
                    enqueue_reply(fd, s, "UNAUTHORIZED\n");
                }
            });

    register_processor(
            NEWL, [&](int fd, std::shared_ptr<Session> &s, std::vector<std::string> &parts) {
                if (parts.size() < 5) {
                    enqueue_reply(
                            fd,
                            s,
                            "ERR BAD_COMMAND\n USAGE: NEWL <BUY|SELL> <SYMBOL> <QTY> <PRICE>\n");
                    return;
                }

                std::string side_tok = parts[1];
                bool        is_buy   = false;
                if (side_tok == "BUY") {
                    is_buy = true;
                } else if (side_tok == "SELL") {
                    is_buy = false;
                } else {
                    enqueue_reply(fd, s, "ERR BAD_SIDE (expected BUY or SELL)\n");
                    return;
                }

                std::string symbol = parts[2];
                if (symbol.empty()) {
                    enqueue_reply(fd, s, "ERR BAD_SYMBOL\n");
                    return;
                }

                uint64_t qty = 0;
                try {
                    long long tmp = std::stoll(parts[3]);
                    if (tmp <= 0) {
                        enqueue_reply(fd, s, "ERR BAD_QTY\n");
                        return;
                    }
                    qty = static_cast<uint64_t>(tmp);
                } catch (...) {
                    enqueue_reply(fd, s, "ERR BAD_QTY\n");
                    return;
                }

                double price = 0.0;
                try {
                    price = std::stod(parts[4]);
                    if (!(price > 0.0)) {
                        enqueue_reply(fd, s, "ERR BAD_PRICE\n");
                        return;
                    }
                } catch (...) {
                    enqueue_reply(fd, s, "ERR BAD_PRICE\n");
                    return;
                }

                struct NewOrder {
                    bool        buy;
                    std::string symbol;
                    uint64_t    qty;
                    double      price;
                };

                NewOrder no{is_buy, symbol, qty, price};

                std::ostringstream oss;
                oss << "OK NEWL PARSED " << (no.buy ? "BUY" : "SELL") << " " << no.symbol << " "
                    << no.qty << " " << no.price << "\n";
                enqueue_reply(fd, s, oss.str());
            });

    register_processor(
            AUTH, [&](int fd, std::shared_ptr<Session> &s, std::vector<std::string> &parts) {
                if (parts.size() < 3) {
                    enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: AUTH <PASSKEY> <CLIENTID>\n");
                    return;
                }

                std::string passkey  = parts[1];
                std::string clientId = parts[2];

                if (!iequals(passkey, EASTER_EGG)) {
                    enqueue_reply(fd, s, "ERR BAD_PASSKEY\n");
                    return;
                }

                if (s->is_authenticated) {
                    if (s->client_id == clientId) {
                        enqueue_reply(fd, s, "OK AUTH\n");
                        return;
                    }
                    if (!s->client_id.empty()) {
                        sessions_.erase(s->client_id);
                    }
                }

                auto prev = sessions_.find(clientId);
                if (prev != sessions_.end()) {
                    if (prev->second->fd != s->fd) {
                        remove_session(prev->second->fd);
                    }
                }

                s->is_authenticated = true;
                s->client_id        = clientId;
                sessions_[clientId] = s;

                enqueue_reply(fd, s, "OK AUTH\n");
            });
}