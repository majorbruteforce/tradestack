#include "manager.hpp"

bool Manager::new_instrument(std::string symbol) {
    if (instruments_.find(symbol) != instruments_.end())
        return false;

    instruments_[symbol] = std::make_shared<Instrument>(symbol);

    return true;
}