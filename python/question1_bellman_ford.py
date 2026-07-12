"""Question 1 -- Bellman-Ford: Cursed Dungeon Routes."""
from dataclasses import dataclass
from typing import List

from test_harness import TestHarness

INF = 10**18


@dataclass(frozen=True)
class Edge:
    from_node: int
    to: int
    weight: int


def shortest_distances(edges: List[Edge], num_nodes: int, start: int) -> List[int]:
    dist = [INF] * num_nodes
    if start < 0 or start >= num_nodes:
        return dist
    dist[start] = 0
    for _ in range(num_nodes - 1):
        changed = False
        for edge in edges:
            if not (0 <= edge.from_node < num_nodes and 0 <= edge.to < num_nodes):
                continue
            if dist[edge.from_node] != INF and dist[edge.from_node] + edge.weight < dist[edge.to]:
                dist[edge.to] = dist[edge.from_node] + edge.weight
                changed = True
        if not changed:
            break
    return dist


def shortest_path(edges: List[Edge], num_nodes: int, start: int, end: int) -> List[int]:
    if start < 0 or start >= num_nodes or end < 0 or end >= num_nodes:
        return []
    if has_negative_cycle(edges, num_nodes, start):
        return []
    dist = [INF] * num_nodes
    parent = [-1] * num_nodes
    dist[start] = 0
    for _ in range(num_nodes - 1):
        changed = False
        for edge in edges:
            if not (0 <= edge.from_node < num_nodes and 0 <= edge.to < num_nodes):
                continue
            if dist[edge.from_node] != INF and dist[edge.from_node] + edge.weight < dist[edge.to]:
                dist[edge.to] = dist[edge.from_node] + edge.weight
                parent[edge.to] = edge.from_node
                changed = True
        if not changed:
            break
    if dist[end] == INF:
        return []
    path = []
    node = end
    while node != -1:
        path.append(node)
        node = parent[node]
    path.reverse()
    return path if path and path[0] == start else []


def has_negative_cycle(edges: List[Edge], num_nodes: int, start: int) -> bool:
    dist = shortest_distances(edges, num_nodes, start)
    for edge in edges:
        if not (0 <= edge.from_node < num_nodes and 0 <= edge.to < num_nodes):
            continue
        if dist[edge.from_node] != INF and dist[edge.from_node] + edge.weight < dist[edge.to]:
            return True
    return False


def time_complexity() -> str:
    return "O(VE)"


def space_complexity() -> str:
    return "O(V)"


if __name__ == "__main__":
    from question1_tests import run_tests, run_dungeon_demo
    passed = run_tests()
    print()
    run_dungeon_demo()
    if not passed:
        raise SystemExit(1)
