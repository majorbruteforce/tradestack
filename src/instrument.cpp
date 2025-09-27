#include "instrument.hpp"
#include "notifier.hpp"

void Instrument::placeOrder(Order &order) {
    if (order.side == Side::Buy) {
        buy_side.insert(order);
    } else {
        sell_side.insert(order);
    }

    execute_limit_if_match();
}

void Instrument::execute_limit_if_match() {
    while (true) {
        if (!buy_side.high || !sell_side.low)
            break;

        auto buyLevel  = buy_side.high;
        auto sellLevel = sell_side.low;

        if (!buyLevel || !sellLevel)
            break;
        if (buyLevel->price < sellLevel->price)
            break;

        auto bestBuyPtr  = buyLevel->level.front();
        auto bestSellPtr = sellLevel->level.front();

        if (!bestBuyPtr || !bestSellPtr)
            break;

        int fillQty =
                std::min(bestBuyPtr->getRemainingQuantity(), bestSellPtr->getRemainingQuantity());
        double fillPrice = static_cast<double>(sellLevel->price);

        std::string buyerId  = bestBuyPtr->clientId;
        std::string sellerId = bestSellPtr->clientId;

        bestBuyPtr->setFilledQuantity(bestBuyPtr->getFilledQuantity() + fillQty);
        bestBuyPtr->setRemainingQuantity(bestBuyPtr->getRemainingQuantity() - fillQty);

        bestSellPtr->setFilledQuantity(bestSellPtr->getFilledQuantity() + fillQty);
        bestSellPtr->setRemainingQuantity(bestSellPtr->getRemainingQuantity() - fillQty);

        if (bestBuyPtr->getRemainingQuantity() == 0) {
            buy_side.remove(*bestBuyPtr);
        }
        if (bestSellPtr->getRemainingQuantity() == 0) {
            sell_side.remove(*bestSellPtr);
        }

        updatePrices(fillPrice);
        last_trade_size = fillQty;

        std::ostringstream oss;
        oss << "EXEC " << symbol << " " << fillQty << "@" << fillPrice << "\n";
        std::string message = oss.str();

        Notifier::instance().notifyUser(buyerId, message);
        Notifier::instance().notifyUser(sellerId, message);
    }
}

void Instrument::updatePrices(double fillPrice) {
    last_trade_price = fillPrice;

    high = std::max(high, last_trade_price);
    low  = std::min(low, last_trade_price);

    if (open == 0) {
        open = last_trade_price;
    }
    close = last_trade_price;

    std::ostringstream oss;
    oss << "F1_UPDATE\n"
        << "LTP: " << last_trade_price << "\n"
        << "HIGH: " << high << "\n"
        << "LOW: " << low << "\n"
        << "OPEN: " << open << "\n"
        << "CLOSE: " << close << "\n";

    Notifier::instance().notifyGroup("L1", oss.str());
}

void Instrument::fetchPrices(std::string clientId) {
    std::ostringstream oss;
    oss << "F1_SNAPSHOT\n"
        << "LTP: " << last_trade_price << "\n"
        << "HIGH: " << high << "\n"
        << "LOW: " << low << "\n"
        << "OPEN: " << open << "\n"
        << "CLOSE: " << close << "\n";

    Notifier::instance().notifyUser(clientId, oss.str());
}