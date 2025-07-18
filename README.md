# TradeStack

A simple and fast implementation of an order book in C++ for 
back testing, simulations and real-time trading applications.

### Build
```bash
git clone https://github.com/majorbruteforce/tradestack.git
cd tradestack
mkdir build && cd build
cmake ..
make
```

## Features

### Order Book Engine
- [ ] Submitting limit and market orders
- [ ] Price-time priority matching
- [ ] Order cancellation and modification
- [ ] Real-time top-of-book retrieval

### FIX Engine
- [ ] Basic FIX message parsing
- [ ] Support for New Order Single (35=D)
- [ ] Support for Order Cancel Request (35=F)
- [ ] Support for Execution Reports (35=8)
- [ ] FIX 4.2 message validation
- [ ] Stateless message processing core

### Testing
- [ ] Unit tests for order book
- [ ] Unit tests for FIX parser
- [ ] Benchmarking and profiling utilities
- [X] Build system with CMake
