#include <iostream>

#include "order.hpp"

int main()
{
    OrderRequest req{

            "ORD123",

            "RELIANCE-EQ",
            Side::Buy,

            OrderType::Limit,
            263500,
            25};
    Order *o = createOrder(req);
    printOrder(*o);

    return 0;
}