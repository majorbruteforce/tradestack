# Devlog - TradeStack

## Order Book Design

An order book facilitates trade of commodities between an asking/bidding and a selling/offering party.
The exchange is done with the following principles:
- If the book is empty, new limits orders can be registered in the book.
- For orders with same price, we abide by the **price-time priority** principle.
- An order is submitted with **type**, **price (P)** and **quantity (Q)**.
- For a **limit** buy order, all sell orders with price ≤ P are filled. If any quantity still remains
it is added back to the book. Same applies for all sell orders; we fill all buy orders with price ≥ offer price
and add the remaining back to the order book.
- A **market** order can be fully or partially filled. The leftover quantities, if any, are 
discarded.

### Data Structure Requirements:

 - Constant time lookup for highest priority element
 - Fast Insertion 
 - Fast Deletion
 - Ordered Lookup
 - Allows sharding

A **BST** allows all of these: 
- O(1) lookup for best bid/ask.
- O(log n) insertion and deletion for addition, modification and cancellation.
- In-order traversal for ordered lookup.
- Trees can be isolated.

This can be further combined with a **deque** for each node to store all orders
in the same price level. A deque allows ordered access and FIFO abilities.


Final verdict: **BST** + **deque**

