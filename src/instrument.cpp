#include "instrument.hpp"
#include "event_bus.hpp"

void Instrument::placeOrder(Order &order) {
    if(order.side == Side::Buy) {
        buy_side.insert(order);
    } else {
        sell_side.insert(order);
    }
}