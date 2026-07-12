# Instructions

## Goal

Implement Bellman-Ford so the tests pass. Use **Java** for GitHub Classroom autograding. Python and C++ versions are included for local practice or instructor-directed use.

## Question 1 -- Bellman-Ford: Cursed Dungeon Routes (20 pts)

Inspired by shortest-path problems such as LeetCode 743, "Network Delay Time", and board/grid problems with traps, portals, and penalties.

You are navigating a cursed dungeon. Each room is a node, and each one-way passage is a weighted directed edge `(from, to, cost)`.

- Positive costs are traps, tolls, or slow terrain.
- Negative costs are boost shrines, shortcuts, or magic portals.
- A reachable negative cycle is an infinite boost loop, so shortest paths are not well-defined.

Implement:

1. `shortestDistances(edges, numNodes, start)` -- return distance from start to every room. Use `INF` for unreachable rooms.
2. `shortestPath(edges, numNodes, start, end)` -- return one best route from start to end, or an empty list if unreachable or if a reachable negative cycle exists.
3. `hasNegativeCycle(edges, numNodes, start)` -- return whether any negative cycle is reachable from start.
4. `timeComplexity()` and `spaceComplexity()` -- return Big-O strings using `V` and `E`.

After the tests, the program prints an ASCII demo route through the dungeon:

```text
S = start, T = trap, B = boost shrine, G = gate, E = exit, * = chosen route
```

## How to run

### Java
```bash
cd java
javac TestHarness.java Question1_Tests.java Question1_BellmanFord.java
java Question1_BellmanFord
```

### Python
```bash
cd python
python question1_bellman_ford.py
```

### C++
```bash
cd cpp
cmake -B build
cmake --build build --config Release
./build/q1_bellman_ford              # Linux/macOS or single-config generators
build\Release\q1_bellman_ford.exe    # Windows Visual Studio generator
```

## Notes

- Nodes are numbered `0..numNodes-1`.
- Edge weights may be negative.
- If there is a reachable negative cycle, shortest distances are not well-defined.
