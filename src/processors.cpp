#include "network.hpp"
#include "notifier.hpp"
#include "utils/string.hpp"
#include "utils/time.hpp"

const std::string DEBUG_SECRET = "123456";
const std::string EASTER_EGG   = "pawy";

void Server::load_processors() {
    const std::string PING  = "PING";
    const std::string DEBUG = "DEBUG";
    const std::string NEWL  = "NEWL";
    const std::string AUTH  = "AUTH";
    const std::string SEND  = "SEND";
    const std::string SUB   = "SUB";

    register_processor(PING,
                       [](int                       fd,
                          std::shared_ptr<Session> &s,
                          std::vector<std::string> &parts,
                          std::string               clientId) {
                           (void)parts;
                           enqueue_reply(fd, s, "PONG\n");
                       });

    register_processor(DEBUG,
                       [&](int                       fd,
                           std::shared_ptr<Session> &s,
                           std::vector<std::string> &parts,
                           std::string               clientId) {
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
                                           << "Authenticated: " << s.second->is_authenticated
                                           << "\n"
                                           << "Client ID: " << s.second->client_id << "\n";
                                   }

                                   enqueue_reply(fd, s, oss.str());
                               }

                               if (parts.size() >= 2 && parts[1] == "ORDERS") {
                                   std::ostringstream oss;
                                   oss << "At: " << now_str() << "\n";
                                   for (auto &[sym, i] : manager.instruments_) {
                                       oss << "SYM: " << sym << "\n";
                                       oss << "    BUY: \n";
                                       auto bs = i->getBuySide();
                                       bs.avl.inorder(
                                               bs.root,
                                               [&](PriceLevelNode *node) {
                                                   oss << "    " << node->price << " ";
                                               },
                                               10);
                                       oss << "\n";

                                       oss << "    SELL: \n";
                                       auto ss = i->getSellSide();
                                       ss.avl.inorder(
                                               ss.root,
                                               [&](PriceLevelNode *node) {
                                                   oss << "    " << node->price << " ";
                                               },
                                               10);
                                       oss << "\n";
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
            NEWL,
            [&](int                       fd,
                std::shared_ptr<Session> &s,
                std::vector<std::string> &parts,
                std::string               clientId) {
                if (!s->is_authenticated) {
                    enqueue_reply(fd, s, "UNAUTHORIZED\n");
                    return;
                }

                if (parts.size() < 5) {
                    enqueue_reply(
                            fd,
                            s,
                            "ERR BAD_COMMAND\n USAGE: NEWL <BUY|SELL> <SYMBOL> <QTY> <PRICE>\n");
                    return;
                }

                std::string side_tok = parts[1];
                Side        side;
                bool        is_buy = false;
                if (side_tok == "BUY") {
                    is_buy = true;
                    side   = Side::Buy;
                } else if (side_tok == "SELL") {
                    is_buy = false;
                    side   = Side::Sell;
                } else {
                    enqueue_reply(fd, s, "ERR BAD_SIDE (expected BUY or SELL)\n");
                    return;
                }

                std::string symbol = parts[2];
                if (symbol.empty() ||
                    (manager.instruments_.find(symbol) == manager.instruments_.end())) {
                    enqueue_reply(fd, s, "ERR BAD_SYMBOL\n");
                    return;
                }

                int qty = 0;
                try {
                    long long tmp = std::stoll(parts[3]);
                    if (tmp <= 0) {
                        enqueue_reply(fd, s, "ERR BAD_QTY\n");
                        return;
                    }
                    qty = static_cast<int>(tmp);
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

                if (clientId.empty()) {
                    enqueue_reply(fd, s, "NOT AUTHENTICATED (NO CID)");
                }

                Order *newOrder = new Order(clientId, price, qty, side, OrderType::Limit);

                manager.instruments_[symbol]->placeOrder(*newOrder);
                enqueue_reply(fd, s, "REQUEST_MADE\n");
            });

    register_processor(
            AUTH,
            [&](int                       fd,
                std::shared_ptr<Session> &s,
                std::vector<std::string> &parts,
                std::string               clientId) {
                if (parts.size() < 3) {
                    enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: AUTH <PASSKEY> <CLIENTID>\n");
                    return;
                }

                std::string passkey = parts[1];
                std::string cid     = parts[2];

                if (!iequals(passkey, EASTER_EGG)) {
                    enqueue_reply(fd, s, "ERR BAD_PASSKEY\n");
                    return;
                }

                if (s->is_authenticated) {
                    if (s->client_id == cid) {
                        enqueue_reply(fd, s, "OK AUTH\n");
                        return;
                    }
                    if (!s->client_id.empty()) {
                        sessions_.erase(s->client_id);
                    }
                }

                auto prev = sessions_.find(cid);
                if (prev != sessions_.end()) {
                    if (prev->second->fd != s->fd) {
                        remove_session(prev->second->fd);
                    }
                }

                s->is_authenticated = true;
                s->client_id        = cid;
                sessions_[cid]      = s;

                enqueue_reply(fd, s, "OK AUTH\n");
            });

    register_processor(
            SEND,
            [&](int                       fd,
                std::shared_ptr<Session> &s,
                std::vector<std::string> &parts,
                std::string               clientId) {
                if (!s->is_authenticated) {
                    enqueue_reply(fd, s, "UNAUTHORIZED\n");
                    return;
                }

                if (parts.size() < 3) {
                    enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: SEND <GROUP_NAME> <MESSAGE>\n");
                    return;
                }

                std::string group = parts[1];
                if (group.empty()) {
                    enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: SEND <GROUP_NAME> <MESSAGE>\n");
                    return;
                }

                std::string message = parts[2];
                if (message.empty()) {
                    enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: SEND <GROUP_NAME> <MESSAGE>\n");
                    return;
                }

                Notifier::instance().notifyGroup(group, message);

                enqueue_reply(fd, s, "MESSAGE SENT");
            });

    register_processor(SUB,
                       [&](int                       fd,
                           std::shared_ptr<Session> &s,
                           std::vector<std::string> &parts,
                           std::string               clientId) {
                           if (!s->is_authenticated) {
                               enqueue_reply(fd, s, "UNAUTHORIZED\n");
                               return;
                           }

                           if (parts.size() < 2) {
                               enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: SUB <GROUP_NAME>\n");
                               return;
                           }

                           std::string group = parts[1];
                           if (group.empty()) {
                               enqueue_reply(fd, s, "ERR BAD_COMMAND\nUSAGE: SUB <GROUP_NAME>\n");
                               return;
                           }

                           Notifier::instance().subscribe(group, clientId);

                           enqueue_reply(fd, s, "SUBSCRIEBED\n");
                       });
}