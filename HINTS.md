# Hints -- Bellman-Ford: Cursed Dungeon Routes

## Hint 1: Distances
Initialize `dist[start] = 0` and every other distance to `INF`. An unreachable locked room should stay at `INF`.

## Hint 2: Relaxation
Relax every passage `V - 1` times. If taking passage `u -> v` improves the best known cost, update `dist[v]`.

## Hint 3: Boosts
Negative edges are allowed. Think of them as boost shrines or magic shortcuts. This is why Dijkstra is not the right algorithm here.

## Hint 4: Route Reconstruction
To print the actual route, store `parent[v] = u` whenever you improve `dist[v]`. Walk backward from the exit to the start.

## Hint 5: Infinite Boost Loops
After `V - 1` passes, do one more pass. If any reachable passage can still improve a distance, a reachable negative cycle exists. In the dungeon story, that is an infinite boost loop, so there is no well-defined best route.
