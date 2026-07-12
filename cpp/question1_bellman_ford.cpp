/**
 * Question 1 -- Bellman-Ford: Cursed Dungeon Routes (20 pts)
 */
#include <string>
#include <vector>

const long long INF = 1000000000000000000LL;

struct Edge {
    int from;
    int to;
    int weight;
};

std::vector<long long> shortestDistances(const std::vector<Edge>& edges, int numNodes, int start) {
    // TODO: Implement Bellman-Ford distances.
    return std::vector<long long>(numNodes, 0);
}

std::vector<int> shortestPath(const std::vector<Edge>& edges, int numNodes, int start, int end) {
    // TODO: Implement Bellman-Ford with parent reconstruction.
    return {};
}

bool hasNegativeCycle(const std::vector<Edge>& edges, int numNodes, int start) {
    // TODO: Return true if an infinite boost loop is reachable from start.
    return false;
}

std::string timeComplexity() {
    // TODO: Return Bellman-Ford time complexity using V and E.
    return "";
}

std::string spaceComplexity() {
    // TODO: Return Bellman-Ford space complexity using V and E.
    return "";
}

#include "question1_tests.h"

int main() {
    auto h = q1_tests::buildHarness();
    bool passed = h.run();
    std::cout << "\n";
    q1_tests::runDungeonDemo();
    return passed ? 0 : 1;
}
