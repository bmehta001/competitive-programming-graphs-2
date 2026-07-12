import java.util.*;
import java.util.function.*;
import java.lang.management.ManagementFactory;

/**
 * A general-purpose testing and benchmarking harness for competitive programming problems.
 *
 * Usage:
 *   TestHarness.&lt;ReturnType&gt;forProblem("Problem Name")
 *       .addTest("test name", () -&gt; solve(input), expectedOutput)
 *       .addBenchmark("large input", () -&gt; solve(largeInput))
 *       .addScaling("N", sizes, n -&gt; genInput(n), input -&gt; solve(input))
 *       .addRandomTests("Stress", 1000, () -&gt; new Case&lt;&gt;(...))
 *       .run();
 *
 * Compile & run any question:
 *   javac *.java
 *   java Question1_CowSightings
 */
public class TestHarness<T> {

    private static final int WARMUP_RUNS = 3;
    private static final int BENCHMARK_RUNS = 10;
    private static final int SCALING_RUNS = 5;
    private static final int MAX_FAILURES_SHOWN = 3;

    private final String problemTitle;
    private final List<Runnable> steps = new ArrayList<>();
    private final List<Boolean> stepIsQuick = new ArrayList<>();
    private int testCount = 0;
    private int passCount = 0;
    private int stepIndex = 0;

    /** A random test case generated at runtime. */
    public static class Case<T> {
        public final String description;
        public final Supplier<T> fn;
        public final T expected;

        public Case(String description, Supplier<T> fn, T expected) {
            this.description = description;
            this.fn = fn;
            this.expected = expected;
        }
    }

    // -- Factory ----------------------------------------------

    public static <T> TestHarness<T> forProblem(String title) {
        return new TestHarness<>(title);
    }

    private TestHarness(String title) {
        this.problemTitle = title;
    }

    // -- Adding tests & benchmarks ----------------------------

    /** Add a correctness test with an expected result. */
    public TestHarness<T> addTest(String name, Supplier<T> fn, T expected) {
        steps.add(() -> {
            testCount++;
            if (executeTest(name, fn, expected)) {
                passCount++;
            }
        });
        stepIsQuick.add(true);
        return this;
    }

    /** Add a correctness test (auto-named "Test N"). */
    public TestHarness<T> addTest(Supplier<T> fn, T expected) {
        return addTest(null, fn, expected);
    }

    /** Add a benchmark (no correctness check, just timing). */
    public TestHarness<T> addBenchmark(String name, Supplier<T> fn) {
        steps.add(() -> executeBenchmark(name, fn));
        stepIsQuick.add(false);
        return this;
    }

    /** Add a benchmark (auto-named "Benchmark N"). */
    public TestHarness<T> addBenchmark(Supplier<T> fn) {
        return addBenchmark(null, fn);
    }

    /**
     * Add a scaling analysis that measures how solve time grows with input size.
     *
     * @param label     description of the size parameter (e.g. "N (array size)")
     * @param sizes     array of input sizes to test, in ascending order
     * @param generator function that creates an input of the given size
     * @param solver    function that runs the solution on the generated input
     */
    public <I> TestHarness<T> addScaling(String label, int[] sizes,
            IntFunction<I> generator, Function<I, T> solver) {
        steps.add(() -> executeScaling(label, sizes, generator, solver, null));
        stepIsQuick.add(false);
        return this;
    }

    /**
     * Add a scaling analysis with a reference solution for side-by-side comparison.
     * Shows timing speedup and approximate memory usage at each input size.
     */
    public <I> TestHarness<T> addScaling(String label, int[] sizes,
            IntFunction<I> generator, Function<I, T> solver, Function<I, T> reference) {
        steps.add(() -> executeScaling(label, sizes, generator, solver, reference));
        stepIsQuick.add(false);
        return this;
    }

    /**
     * Add randomised stress tests that compare the student solution against
     * a known-correct reference.
     *
     * @param label         description of the stress test
     * @param count         number of random cases to generate
     * @param caseGenerator supplier that produces a fresh Case each call
     */
    public TestHarness<T> addRandomTests(String label, int count,
            Supplier<Case<T>> caseGenerator) {
        steps.add(() -> executeRandomTests(label, count, caseGenerator));
        stepIsQuick.add(true);
        return this;
    }

    // -- Running ----------------------------------------------

    /** Run all steps and return true if every test passed. */
    public boolean run() {
        return run(false);
    }

    /** Run only correctness and random tests (skip benchmarks and scaling). */
    public boolean runQuick() {
        return run(true);
    }

    private boolean run(boolean quickOnly) {
        printHeader();
        if (quickOnly) {
            System.out.println("  (quick mode -- skipping benchmarks & scaling)\n");
        }
        testCount = 0;
        passCount = 0;
        stepIndex = 0;

        for (int i = 0; i < steps.size(); i++) {
            if (quickOnly && !stepIsQuick.get(i)) {
                continue;
            }
            stepIndex++;
            steps.get(i).run();
            System.out.println();
        }

        printSummary(passCount, testCount);
        return testCount > 0 && passCount == testCount;
    }

    // -- Execution: correctness test -------------------------

    private boolean executeTest(String name, Supplier<T> fn, T expected) {
        if (name != null && !name.isEmpty()) {
            System.out.printf("  Test %d: %s%n", stepIndex, name);
        } else {
            System.out.printf("  Test %d%n", stepIndex);
        }

        for (int w = 0; w < WARMUP_RUNS; w++) {
            fn.get();
        }

        long startNs = System.nanoTime();
        T result = fn.get();
        long elapsedNs = System.nanoTime() - startNs;

        boolean passed = Objects.deepEquals(result, expected);

        if (passed) {
            System.out.printf("    Status:   PASS%n");
        } else {
            System.out.printf("    Status:   FAIL%n");
            System.out.printf("    Expected: %s%n", format(expected));
            System.out.printf("    Got:      %s%n", format(result));
        }
        System.out.printf("    Time:     %.3f ms%n", elapsedNs / 1_000_000.0);

        return passed;
    }

    // -- Execution: benchmark --------------------------------

    private void executeBenchmark(String name, Supplier<T> fn) {
        if (name != null && !name.isEmpty()) {
            System.out.printf("  Benchmark %d: %s%n", stepIndex, name);
        } else {
            System.out.printf("  Benchmark %d%n", stepIndex);
        }

        for (int w = 0; w < WARMUP_RUNS; w++) {
            fn.get();
        }

        long[] times = new long[BENCHMARK_RUNS];
        for (int r = 0; r < BENCHMARK_RUNS; r++) {
            long startNs = System.nanoTime();
            fn.get();
            times[r] = System.nanoTime() - startNs;
        }

        long min = Long.MAX_VALUE, max = Long.MIN_VALUE, sum = 0;
        for (long t : times) {
            min = Math.min(min, t);
            max = Math.max(max, t);
            sum += t;
        }
        double avg = sum / (double) BENCHMARK_RUNS;

        System.out.printf("    Avg time: %.3f ms  (%d runs)%n", avg / 1_000_000.0, BENCHMARK_RUNS);
        System.out.printf("    Min time: %.3f ms%n", min / 1_000_000.0);
        System.out.printf("    Max time: %.3f ms%n", max / 1_000_000.0);
    }

    // -- Execution: scaling analysis -------------------------

    private <I> void executeScaling(String label, int[] sizes,
            IntFunction<I> generator, Function<I, T> solver, Function<I, T> reference) {
        boolean hasRef = reference != null;
        if (hasRef) {
            System.out.printf("  Scaling %d: %s [vs reference]%n", stepIndex, label);
        } else {
            System.out.printf("  Scaling %d: %s%n", stepIndex, label);
        }

        double prevMs = -1;
        int prevSize = -1;

        for (int size : sizes) {
            I input = generator.apply(size);

            // Warmup solve
            for (int w = 0; w < WARMUP_RUNS; w++) {
                solver.apply(input);
            }

            // Time solve
            long solveSum = 0;
            for (int r = 0; r < SCALING_RUNS; r++) {
                long startNs = System.nanoTime();
                solver.apply(input);
                solveSum += System.nanoTime() - startNs;
            }
            double solveMs = (solveSum / (double) SCALING_RUNS) / 1_000_000.0;

            if (!hasRef) {
                if (prevMs <= 0) {
                    System.out.printf("    N = %,10d | %10.3f ms%n", size, solveMs);
                } else {
                    double timeRatio = solveMs / prevMs;
                    double sizeRatio = (double) size / prevSize;
                    System.out.printf("    N = %,10d | %10.3f ms | %5.1fx time for %5.1fx N%n",
                        size, solveMs, timeRatio, sizeRatio);
                }
            } else {
                // Warmup + time reference
                for (int w = 0; w < WARMUP_RUNS; w++) {
                    reference.apply(input);
                }
                long refSum = 0;
                for (int r = 0; r < SCALING_RUNS; r++) {
                    long startNs = System.nanoTime();
                    reference.apply(input);
                    refSum += System.nanoTime() - startNs;
                }
                double refMs = (refSum / (double) SCALING_RUNS) / 1_000_000.0;

                // Memory: single-run measurement for each
                long solveMem = measureAllocatedBytes(() -> solver.apply(input));
                System.gc();
                long refMem = measureAllocatedBytes(() -> reference.apply(input));

                double speedup = (solveMs > 0) ? refMs / solveMs : 0;

                String memStr = "";
                if (solveMem >= 0 && refMem >= 0) {
                    memStr = String.format(" | mem: %s vs %s",
                        formatBytes(solveMem), formatBytes(refMem));
                }

                System.out.printf("    N = %,10d | solve: %8.3f ms | ref: %8.3f ms | %5.1fx%s%n",
                    size, solveMs, refMs, speedup, memStr);
            }

            prevMs = solveMs;
            prevSize = size;
        }

        System.out.println();
        if (!hasRef) {
            System.out.println("    Interpretation guide:");
            System.out.println("      ~1x time for kx N  =>  O(1)");
            System.out.println("      ~kx time for kx N  =>  O(N)");
            System.out.println("      ~k^2x for kx N     =>  O(N^2)");
        } else {
            System.out.println("    speedup = ref_time / solve_time (higher is better)");
            System.out.println("    mem = approx. heap bytes allocated per call");
        }
    }

    // -- Execution: random stress tests ----------------------

    private void executeRandomTests(String label, int count,
            Supplier<Case<T>> caseGenerator) {
        System.out.printf("  Random Tests %d: %s (%,d cases)%n", stepIndex, label, count);

        int passed = 0;
        List<String> failures = new ArrayList<>();

        for (int i = 0; i < count; i++) {
            Case<T> tc = caseGenerator.get();
            T result = tc.fn.get();
            if (Objects.deepEquals(result, tc.expected)) {
                passed++;
            } else if (failures.size() < MAX_FAILURES_SHOWN) {
                failures.add(String.format(
                    "    FAIL #%d: %s%n      Expected: %s%n      Got:      %s",
                    failures.size() + 1, tc.description,
                    format(tc.expected), format(result)));
            }
        }

        System.out.printf("    Passed: %,d / %,d%n", passed, count);
        testCount += count;
        passCount += passed;

        for (String f : failures) {
            System.out.println(f);
        }
        if (count - passed > MAX_FAILURES_SHOWN) {
            System.out.printf("    ... and %,d more failures%n",
                count - passed - MAX_FAILURES_SHOWN);
        }
    }

    // -- Memory measurement helpers --------------------------

    /** Measure heap bytes allocated during a single function call via ThreadMXBean. */
    private static long measureAllocatedBytes(Runnable fn) {
        long before = getThreadAllocatedBytes();
        fn.run();
        long after = getThreadAllocatedBytes();
        return (before >= 0 && after >= 0) ? after - before : -1;
    }

    private static long getThreadAllocatedBytes() {
        try {
            com.sun.management.ThreadMXBean tmx =
                (com.sun.management.ThreadMXBean) ManagementFactory.getThreadMXBean();
            return tmx.getThreadAllocatedBytes(Thread.currentThread().getId());
        } catch (Exception e) {
            return -1;
        }
    }

    private static String formatBytes(long bytes) {
        if (bytes < 0) {
            return "N/A";
        }
        if (bytes < 1024) {
            return bytes + " B";
        }
        if (bytes < 1024 * 1024) {
            return String.format("%.1f KB", bytes / 1024.0);
        }
        return String.format("%.1f MB", bytes / (1024.0 * 1024));
    }

    // -- Formatting helpers -----------------------------------

    private static String repeatStr(String s, int n) {
        StringBuilder sb = new StringBuilder(s.length() * n);
        for (int i = 0; i < n; i++) {
            sb.append(s);
        }
        return sb.toString();
    }

    private void printHeader() {
        String border = repeatStr("=", 55);
        System.out.println(border);
        System.out.printf("  Problem: %s%n", problemTitle);
        System.out.println(border);
        System.out.println();
    }

    private void printSummary(int passed, int total) {
        String border = repeatStr("=", 55);
        System.out.println(border);
        if (total == 0) {
            System.out.println("  No correctness tests defined (benchmarks only).");
        } else {
            System.out.printf("  Summary: %,d / %,d tests passed%n", passed, total);
        }
        System.out.println(border);
    }

    /** Pretty-print any result type, including primitive arrays. */
    private static String format(Object obj) {
        if (obj == null) {
            return "null";
        }
        if (obj instanceof int[]) {
            return Arrays.toString((int[]) obj);
        }
        if (obj instanceof long[]) {
            return Arrays.toString((long[]) obj);
        }
        if (obj instanceof double[]) {
            return Arrays.toString((double[]) obj);
        }
        if (obj instanceof boolean[]) {
            return Arrays.toString((boolean[]) obj);
        }
        if (obj instanceof char[]) {
            return Arrays.toString((char[]) obj);
        }
        if (obj instanceof Object[]) {
            return Arrays.deepToString((Object[]) obj);
        }
        return obj.toString();
    }
}
