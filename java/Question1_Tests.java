import java.util.*;

public class Question1_Tests {
    private static Question1_BellmanFord.Edge e(int u, int v, int w) {
        return new Question1_BellmanFord.Edge(u, v, w);
    }

    private static List<Question1_BellmanFord.Edge> edges(Question1_BellmanFord.Edge... edges) {
        return Arrays.asList(edges);
    }

    private static boolean arraysEqual(long[] actual, long[] expected) {
        return Arrays.equals(actual, expected);
    }

    private static List<Question1_BellmanFord.Edge> dungeonEdges() {
        return edges(
            e(0, 1, 4),    // camp -> bridge
            e(0, 2, 7),    // camp -> trap hall
            e(1, 3, 2),    // bridge -> boost shrine
            e(2, 4, 3),    // trap hall -> gate
            e(3, 4, -5),   // boost shrine -> gate (magic boost)
            e(4, 5, 2),    // gate -> exit
            e(1, 5, 20));  // expensive direct escape
    }

    public static TestHarness<Boolean> buildHarness() {
        TestHarness<Boolean> h = TestHarness.<Boolean>forProblem("Question 1 -- Cursed Dungeon Bellman-Ford");

        h.addTest("Dungeon: boosts beat the obvious route", () -> {
            long[] expected = {0, 4, 7, 6, 1, 3};
            return arraysEqual(Question1_BellmanFord.shortestDistances(dungeonEdges(), 6, 0), expected);
        }, true);

        h.addTest("Dungeon: best route avoids trap hall", () -> {
            return Question1_BellmanFord.shortestPath(dungeonEdges(), 6, 0, 5)
                .equals(Arrays.asList(0, 1, 3, 4, 5));
        }, true);

        h.addTest("Classic: negative edge but no negative cycle", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(
                e(0, 1, 4), e(0, 2, 5), e(1, 2, -3), e(2, 3, 2));
            long[] expected = {0, 4, 1, 3};
            return arraysEqual(Question1_BellmanFord.shortestDistances(graph, 4, 0), expected);
        }, true);

        h.addTest("Locked side chamber stays unreachable", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(e(0, 1, 7), e(1, 2, 1));
            long[] dist = Question1_BellmanFord.shortestDistances(graph, 4, 0);
            return dist[0] == 0 && dist[1] == 7 && dist[2] == 8
                && dist[3] == Question1_BellmanFord.INF;
        }, true);

        h.addTest("Unreachable shortest path is empty", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(e(0, 1, 1));
            return Question1_BellmanFord.shortestPath(graph, 3, 0, 2).isEmpty();
        }, true);

        h.addTest("Infinite boost loop is detected", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(
                e(0, 1, 1), e(1, 2, -2), e(2, 1, -2));
            return Question1_BellmanFord.hasNegativeCycle(graph, 3, 0);
        }, true);

        h.addTest("Unreachable curse loop is ignored", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(
                e(0, 1, 1), e(2, 3, -2), e(3, 2, -2));
            return !Question1_BellmanFord.hasNegativeCycle(graph, 4, 0);
        }, true);

        h.addTest("Infinite boost loop makes route undefined", () -> {
            List<Question1_BellmanFord.Edge> graph = edges(
                e(0, 1, 1), e(1, 2, -2), e(2, 1, -2), e(2, 3, 1));
            return Question1_BellmanFord.shortestPath(graph, 4, 0, 3).isEmpty();
        }, true);

        h.addTest("B1: time complexity contains V and E", () -> {
            String answer = Question1_BellmanFord.timeComplexity().toUpperCase(Locale.ROOT);
            return answer.contains("V") && answer.contains("E");
        }, true);

        h.addTest("B2: space complexity contains V", () -> {
            String answer = Question1_BellmanFord.spaceComplexity().toUpperCase(Locale.ROOT);
            return answer.contains("V");
        }, true);

        return h;
    }

    public static void runDungeonDemo() {
        List<Integer> path = Question1_BellmanFord.shortestPath(dungeonEdges(), 6, 0, 5);
        Set<Integer> onPath = new HashSet<>(path);
        String[] rooms = {"S", ".", "T", "B", "G", "E"};
        System.out.println("=== Cursed Dungeon Route Demo ===");
        System.out.println("Legend: S=start, T=trap, B=boost shrine, G=gate, E=exit, *=chosen route");
        for (int i = 0; i < rooms.length; i++) {
            String mark = onPath.contains(i) ? "*" : " ";
            System.out.printf("  %s Room %d [%s]%n", mark, i, rooms[i]);
        }
        if (path.isEmpty()) {
            System.out.println("No safe route yet (or an infinite boost loop exists). Implement Bellman-Ford!");
        } else {
            System.out.println("Best route: " + path + " with total cost 3");
        }
    }
}
