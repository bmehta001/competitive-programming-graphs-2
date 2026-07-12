/**
 * Question 1 -- Bellman-Ford: Cursed Dungeon Routes (20 pts)
 */
#include <algorithm>
#include <string>
#include <vector>

const long long INF = 1000000000000000000LL;

struct Edge {
    int from;
    int to;
    int weight;
};

std::vector<long long> shortestDistances(const std::vector<Edge>& edges, int numNodes, int start) {
    std::vector<long long> dist(numNodes, INF);
    if (start < 0 || start >= numNodes) {
        return dist;
    }
    dist[start] = 0;
    for (int pass = 0; pass < numNodes - 1; pass++) {
        bool changed = false;
        for (const Edge& edge : edges) {
            if (edge.from < 0 || edge.from >= numNodes || edge.to < 0 || edge.to >= numNodes) {
                continue;
            }
            if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
    }
    return dist;
}

bool hasNegativeCycle(const std::vector<Edge>& edges, int numNodes, int start) {
    std::vector<long long> dist = shortestDistances(edges, numNodes, start);
    for (const Edge& edge : edges) {
        if (edge.from < 0 || edge.from >= numNodes || edge.to < 0 || edge.to >= numNodes) {
            continue;
        }
        if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
            return true;
        }
    }
    return false;
}

std::vector<int> shortestPath(const std::vector<Edge>& edges, int numNodes, int start, int end) {
    std::vector<int> empty;
    if (start < 0 || start >= numNodes || end < 0 || end >= numNodes) {
        return empty;
    }
    if (hasNegativeCycle(edges, numNodes, start)) {
        return empty;
    }
    std::vector<long long> dist(numNodes, INF);
    std::vector<int> parent(numNodes, -1);
    dist[start] = 0;
    for (int pass = 0; pass < numNodes - 1; pass++) {
        bool changed = false;
        for (const Edge& edge : edges) {
            if (edge.from < 0 || edge.from >= numNodes || edge.to < 0 || edge.to >= numNodes) {
                continue;
            }
            if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
                dist[edge.to] = dist[edge.from] + edge.weight;
                parent[edge.to] = edge.from;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
    }
    if (dist[end] == INF) {
        return empty;
    }
    std::vector<int> path;
    for (int node = end; node != -1; node = parent[node]) {
        path.push_back(node);
    }
    std::reverse(path.begin(), path.end());
    return !path.empty() && path.front() == start ? path : empty;
}

std::string timeComplexity() {
    return "O(VE)";
}

std::string spaceComplexity() {
    return "O(V)";
}

#include "question1_tests.h"

int main() {
    auto h = q1_tests::buildHarness();
    bool passed = h.run();
    std::cout << "\n";
    q1_tests::runDungeonDemo();
    return passed ? 0 : 1;
}
