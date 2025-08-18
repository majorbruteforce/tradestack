### Corrections

- [ ] `SideTree<T>::size()` must return number of nodes and not orders

### Optimizations

- [ ] Instead of recomputing the high/low for every removal, do it locally after removal
- [ ] While removing a level from the side tree, do in O(1) (node is already discovered) instead of O(log n)
- [ ] Remove all patterns of double find
