#include <order.hpp>
#include <utils/id_generator.hpp>

Order *createOrder(const OrderRequest &req) {
    const std::uint64_t rawId = utils::IdGenerator::next();
    return new Order(
            std::to_string(rawId), req.clientOrderId, req.price, req.quantity, req.side, req.type);
}
