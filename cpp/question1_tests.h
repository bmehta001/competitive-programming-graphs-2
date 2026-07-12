#pragma once

#include "test_harness.h"

#include <cctype>
#include <iostream>
#include <set>
#include <string>
#include <vector>

std::vector<long long> shortestDistances(const std::vector<Edge>& edges, int numNodes, int start);
std::vector<int> shortestPath(const std::vector<Edge>& edges, int numNodes, int start, int end);
bool hasNegativeCycle(const std::vector<Edge>& edges, int numNodes, int start);
std::string timeComplexity();
std::string spaceComplexity();

namespace q1_tests {

inline std::vector<Edge> dungeonEdges() {
    return {{0,1,4},{0,2,7},{1,3,2},{2,4,3},{3,4,-5},{4,5,2},{1,5,20}};
}

inline harness::TestHarness<bool> buildHarness() {
    harness::TestHarness<bool> h = harness::TestHarness<bool>::forProblem("Question 1 -- Cursed Dungeon Bellman-Ford");

    h.addTest("Dungeon: boosts beat the obvious route", []() {
        return shortestDistances(dungeonEdges(), 6, 0) == std::vector<long long>{0,4,7,6,1,3};
    }, true);

    h.addTest("Dungeon: best route avoids trap hall", []() {
        return shortestPath(dungeonEdges(), 6, 0, 5) == std::vector<int>{0,1,3,4,5};
    }, true);

    h.addTest("Classic: negative edge but no negative cycle", []() {
        std::vector<Edge> graph = {{0,1,4},{0,2,5},{1,2,-3},{2,3,2}};
        return shortestDistances(graph, 4, 0) == std::vector<long long>{0,4,1,3};
    }, true);

    h.addTest("Locked side chamber stays unreachable", []() {
        std::vector<Edge> graph = {{0,1,7},{1,2,1}};
        auto dist = shortestDistances(graph, 4, 0);
        return dist[0] == 0 && dist[1] == 7 && dist[2] == 8 && dist[3] == INF;
    }, true);

    h.addTest("Unreachable shortest path is empty", []() {
        std::vector<Edge> graph = {{0,1,1}};
        return shortestPath(graph, 3, 0, 2).empty();
    }, true);

    h.addTest("Infinite boost loop is detected", []() {
        std::vector<Edge> graph = {{0,1,1},{1,2,-2},{2,1,-2}};
        return hasNegativeCycle(graph, 3, 0);
    }, true);

    h.addTest("Unreachable curse loop is ignored", []() {
        std::vector<Edge> graph = {{0,1,1},{2,3,-2},{3,2,-2}};
        return !hasNegativeCycle(graph, 4, 0);
    }, true);

    h.addTest("Infinite boost loop makes route undefined", []() {
        std::vector<Edge> graph = {{0,1,1},{1,2,-2},{2,1,-2},{2,3,1}};
        return shortestPath(graph, 4, 0, 3).empty();
    }, true);

    h.addTest("B1: time complexity contains V and E", []() {
        std::string answer = timeComplexity();
        for (char& ch : answer) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        return answer.find('V') != std::string::npos && answer.find('E') != std::string::npos;
    }, true);

    h.addTest("B2: space complexity contains V", []() {
        std::string answer = spaceComplexity();
        for (char& ch : answer) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        return answer.find('V') != std::string::npos;
    }, true);

    return h;
}

inline void runDungeonDemo() {
    std::vector<int> path = shortestPath(dungeonEdges(), 6, 0, 5);
    std::set<int> onPath(path.begin(), path.end());
    std::vector<std::string> rooms = {"S", ".", "T", "B", "G", "E"};
    std::cout << "=== Cursed Dungeon Route Demo ===\n";
    std::cout << "Legend: S=start, T=trap, B=boost shrine, G=gate, E=exit, *=chosen route\n";
    for (int i = 0; i < static_cast<int>(rooms.size()); i++) {
        std::cout << "  " << (onPath.count(i) ? '*' : ' ') << " Room " << i << " [" << rooms[i] << "]\n";
    }
    if (path.empty()) {
        std::cout << "No safe route yet (or an infinite boost loop exists). Implement Bellman-Ford!\n";
    } else {
        std::cout << "Best route: [0, 1, 3, 4, 5] with total cost 3\n";
    }
}

} // namespace q1_tests
