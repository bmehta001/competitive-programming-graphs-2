"""Tests for Question 1 -- Bellman-Ford."""
from test_harness import TestHarness
from question1_bellman_ford import (
    INF,
    Edge,
    shortest_distances,
    shortest_path,
    has_negative_cycle,
    time_complexity,
    space_complexity,
)


def dungeon_edges():
    return [
        Edge(0, 1, 4),
        Edge(0, 2, 7),
        Edge(1, 3, 2),
        Edge(2, 4, 3),
        Edge(3, 4, -5),
        Edge(4, 5, 2),
        Edge(1, 5, 20),
    ]


def run_tests():
    h = TestHarness("Question 1 -- Cursed Dungeon Bellman-Ford")

    h.add_test("Dungeon: boosts beat the obvious route",
        lambda: shortest_distances(dungeon_edges(), 6, 0), [0, 4, 7, 6, 1, 3])

    h.add_test("Dungeon: best route avoids trap hall",
        lambda: shortest_path(dungeon_edges(), 6, 0, 5), [0, 1, 3, 4, 5])

    h.add_test("Classic: negative edge but no negative cycle",
        lambda: shortest_distances([
            Edge(0, 1, 4), Edge(0, 2, 5), Edge(1, 2, -3), Edge(2, 3, 2)
        ], 4, 0), [0, 4, 1, 3])

    def unreachable_ok():
        dist = shortest_distances([Edge(0, 1, 7), Edge(1, 2, 1)], 4, 0)
        return dist[0] == 0 and dist[1] == 7 and dist[2] == 8 and dist[3] == INF
    h.add_test("Locked side chamber stays unreachable", unreachable_ok, True)

    h.add_test("Unreachable shortest path is empty", lambda: shortest_path([Edge(0, 1, 1)], 3, 0, 2), [])
    h.add_test("Infinite boost loop is detected",
        lambda: has_negative_cycle([Edge(0, 1, 1), Edge(1, 2, -2), Edge(2, 1, -2)], 3, 0), True)
    h.add_test("Unreachable curse loop is ignored",
        lambda: has_negative_cycle([Edge(0, 1, 1), Edge(2, 3, -2), Edge(3, 2, -2)], 4, 0), False)
    h.add_test("Infinite boost loop makes route undefined",
        lambda: shortest_path([Edge(0, 1, 1), Edge(1, 2, -2), Edge(2, 1, -2), Edge(2, 3, 1)], 4, 0, 3), [])

    h.add_test("B1: time complexity contains V and E", lambda: "V" in time_complexity().upper() and "E" in time_complexity().upper(), True)
    h.add_test("B2: space complexity contains V", lambda: "V" in space_complexity().upper(), True)

    return h.run()


def run_dungeon_demo():
    path = shortest_path(dungeon_edges(), 6, 0, 5)
    on_path = set(path)
    rooms = ["S", ".", "T", "B", "G", "E"]
    print("=== Cursed Dungeon Route Demo ===")
    print("Legend: S=start, T=trap, B=boost shrine, G=gate, E=exit, *=chosen route")
    for i, room in enumerate(rooms):
        mark = "*" if i in on_path else " "
        print(f"  {mark} Room {i} [{room}]")
    if path:
        print(f"Best route: {path} with total cost 3")
    else:
        print("No safe route yet (or an infinite boost loop exists). Implement Bellman-Ford!")


if __name__ == "__main__":
    passed = run_tests()
    print()
    run_dungeon_demo()
    if not passed:
        raise SystemExit(1)
