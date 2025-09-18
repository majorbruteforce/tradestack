#pragma once

#include <memory>

#include "instrument.hpp"

class Manager {
   public:
    bool new_instrument(std::string symbol);

    std::unordered_map<std::string, std::shared_ptr<Instrument>> instruments_;
};