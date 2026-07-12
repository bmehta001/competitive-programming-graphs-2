# Solutions -- Graph Theory Assignment 2

## Question 1 -- Bellman-Ford: Cursed Dungeon Routes

Bellman-Ford repeatedly relaxes every edge. After `V - 1` passes, every shortest path without a cycle has been considered. One additional pass detects whether a reachable negative cycle can still improve a distance.

In the dungeon story, positive edges are traps or tolls, negative edges are boost shrines, and a reachable negative cycle is an infinite boost loop.

### Java

```java
public static long[] shortestDistances(List<Edge> edges, int numNodes, int start) {
    long[] dist = new long[numNodes];
    Arrays.fill(dist, INF);
    dist[start] = 0;
    for (int pass = 0; pass < numNodes - 1; pass++) {
        boolean changed = false;
        for (Edge edge : edges) {
            if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                changed = true;
            }
        }
        if (!changed) break;
    }
    return dist;
}

public static boolean hasNegativeCycle(List<Edge> edges, int numNodes, int start) {
    long[] dist = shortestDistances(edges, numNodes, start);
    for (Edge edge : edges) {
        if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
            return true;
        }
    }
    return false;
}
```

Use a `parent[]` array during relaxation to reconstruct `shortestPath`.

### Python

```python
def shortest_distances(edges, num_nodes, start):
    dist = [INF] * num_nodes
    dist[start] = 0
    for _ in range(num_nodes - 1):
        changed = False
        for edge in edges:
            if dist[edge.from_node] != INF and dist[edge.from_node] + edge.weight < dist[edge.to]:
                dist[edge.to] = dist[edge.from_node] + edge.weight
                changed = True
        if not changed:
            break
    return dist
```

### C++

```cpp
std::vector<long long> shortestDistances(const std::vector<Edge>& edges, int numNodes, int start) {
    std::vector<long long> dist(numNodes, INF);
    dist[start] = 0;
    for (int pass = 0; pass < numNodes - 1; pass++) {
        bool changed = false;
        for (const Edge& edge : edges) {
            if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                changed = true;
            }
        }
        if (!changed) break;
    }
    return dist;
}
```

### Analysis

- Time complexity: `O(VE)`
- Space complexity: `O(V)` for distances and parent arrays
