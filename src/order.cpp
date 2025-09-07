#include "order.hpp"

#include "utils/id_generator.hpp"

namespace tradestack {

    std::unique_ptr<Order> createOrder(const OrderRequest& req) {
        const uint64_t rawId = utils::IdGenerator::next();
        return std::make_unique<Order>(
            std::to_string(rawId), req.clientOrderId, req.price, req.quantity, req.side, req.type);
    }
}  // namespace tradestack
