import java.util.*;

/**
 * Question 1 -- Bellman-Ford: Cursed Dungeon Routes (20 pts)
 *
 * Inspired by shortest-path problems such as LeetCode 743 (Network Delay Time)
 * and grid/board problems with traps, portals, and boosts.
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
        long[] dist = new long[numNodes];
        Arrays.fill(dist, INF);
        if (start < 0 || start >= numNodes) {
            return dist;
        }
        dist[start] = 0;
        for (int pass = 0; pass < numNodes - 1; pass++) {
            boolean changed = false;
            for (Edge edge : edges) {
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

    public static List<Integer> shortestPath(List<Edge> edges, int numNodes, int start, int end) {
        if (start < 0 || start >= numNodes || end < 0 || end >= numNodes) {
            return new ArrayList<>();
        }
        if (hasNegativeCycle(edges, numNodes, start)) {
            return new ArrayList<>();
        }
        long[] dist = new long[numNodes];
        int[] parent = new int[numNodes];
        Arrays.fill(dist, INF);
        Arrays.fill(parent, -1);
        dist[start] = 0;
        for (int pass = 0; pass < numNodes - 1; pass++) {
            boolean changed = false;
            for (Edge edge : edges) {
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
            return new ArrayList<>();
        }
        List<Integer> path = new ArrayList<>();
        for (int at = end; at != -1; at = parent[at]) {
            path.add(at);
        }
        Collections.reverse(path);
        return path.get(0) == start ? path : new ArrayList<>();
    }

    public static boolean hasNegativeCycle(List<Edge> edges, int numNodes, int start) {
        long[] dist = shortestDistances(edges, numNodes, start);
        for (Edge edge : edges) {
            if (edge.from < 0 || edge.from >= numNodes || edge.to < 0 || edge.to >= numNodes) {
                continue;
            }
            if (dist[edge.from] != INF && dist[edge.from] + edge.weight < dist[edge.to]) {
                return true;
            }
        }
        return false;
    }

    public static String timeComplexity() {
        return "O(VE)";
    }

    public static String spaceComplexity() {
        return "O(V)";
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
