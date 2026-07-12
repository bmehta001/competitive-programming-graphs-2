import java.util.*;

/**
 * Question 1 -- Bellman-Ford: Cursed Dungeon Routes (20 pts)
 *
 * Inspired by shortest-path problems such as LeetCode 743 (Network Delay Time)
 * and grid/board problems with traps, portals, and boosts.
 *
 * Rooms are nodes and directed passages are weighted edges:
 *   - positive weights are time/energy costs
 *   - negative weights are boosts, shortcuts, or magic portals
 *
 * Bellman-Ford is useful here because Dijkstra cannot safely handle negative
 * edges. It can also detect reachable negative cycles, which represent an
 * infinite boost loop where the best route is not well-defined.
 */
public class Question1_BellmanFord {
    public static final long INF = Long.MAX_VALUE / 4;

    public static class Edge {
        public final int from;
        public final int to;
        public final int weight;

        public Edge(int from, int to, int weight) {
            this.from = from;
            this.to = to;
            this.weight = weight;
        }
    }

    public static long[] shortestDistances(List<Edge> edges, int numNodes, int start) {
        // TODO: Implement Bellman-Ford distances.
        return new long[numNodes];
    }

    public static List<Integer> shortestPath(List<Edge> edges, int numNodes, int start, int end) {
        // TODO: Implement Bellman-Ford with parent reconstruction.
        return new ArrayList<>();
    }

    public static boolean hasNegativeCycle(List<Edge> edges, int numNodes, int start) {
        // TODO: Return true if an infinite boost loop is reachable from start.
        return false;
    }

    public static String timeComplexity() {
        // TODO: Return Bellman-Ford time complexity using V and E.
        return "";
    }

    public static String spaceComplexity() {
        // TODO: Return Bellman-Ford space complexity using V and E.
        return "";
    }

    public static void main(String[] args) {
        boolean passed = Question1_Tests.buildHarness().run();
        System.out.println();
        Question1_Tests.runDungeonDemo();
        if (!passed) {
            System.exit(1);
        }
    }
}
