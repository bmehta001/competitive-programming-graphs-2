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
    """Return shortest distance from start to every room, using INF if unreachable."""
    # TODO: Implement Bellman-Ford distances.
    return [0] * num_nodes


def shortest_path(edges: List[Edge], num_nodes: int, start: int, end: int) -> List[int]:
    """Return one best route, or [] if unreachable or an infinite boost loop exists."""
    # TODO: Implement Bellman-Ford with parent reconstruction.
    return []


def has_negative_cycle(edges: List[Edge], num_nodes: int, start: int) -> bool:
    """Return True if an infinite boost loop is reachable from start."""
    # TODO: Detect reachable negative cycle.
    return False


def time_complexity() -> str:
    # TODO: Return Bellman-Ford time complexity using V and E.
    return ""


def space_complexity() -> str:
    # TODO: Return Bellman-Ford space complexity using V and E.
    return ""


if __name__ == "__main__":
    from question1_tests import run_tests, run_dungeon_demo
    passed = run_tests()
    print()
    run_dungeon_demo()
    if not passed:
        raise SystemExit(1)
