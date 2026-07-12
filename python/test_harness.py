"""
A general-purpose testing and benchmarking harness for competitive programming problems.

Usage:
    from test_harness import TestHarness, Case

    TestHarness("Problem Name") \\
        .add_test("test name", lambda: solve(input), expected_output) \\
        .add_benchmark("large input", lambda: solve(large_input)) \\
        .add_scaling("N", sizes, generator_fn, solver_fn) \\
        .add_random_tests("Stress", 1000, case_generator_fn) \\
        .run()

Run any question:
    python question1_cow_sightings.py
"""

import time
import tracemalloc
from dataclasses import dataclass
from typing import Any, Callable


@dataclass
class Case:
    """A random test case generated at runtime."""
    description: str
    fn: Callable
    expected: Any


class TestHarness:

    WARMUP_RUNS = 3
    BENCHMARK_RUNS = 10
    SCALING_RUNS = 5
    MAX_FAILURES_SHOWN = 3

    def __init__(self, title: str):
        self._title = title
        self._steps: list[Callable] = []
        self._step_is_quick: list[bool] = []
        self._test_count = 0
        self._pass_count = 0
        self._step_index = 0

    # -- Adding tests & benchmarks ----------------------------

    def add_test(self, name_or_fn, fn=None, expected=None) -> "TestHarness":
        """Add a correctness test. Name is optional -- omit for auto-naming.

        Usage:
            .add_test("My test", lambda: solve(...), expected)
            .add_test(lambda: solve(...), expected)        # auto-named "Test N"
        """
        if callable(name_or_fn):
            # Called as add_test(fn, expected) -- no name
            name, fn, expected = None, name_or_fn, fn
        else:
            name = name_or_fn
        def step(name=name, fn=fn, expected=expected):
            self._test_count += 1
            if self._execute_test(name, fn, expected):
                self._pass_count += 1
        self._steps.append(step)
        self._step_is_quick.append(True)
        return self

    def add_benchmark(self, name_or_fn, fn=None) -> "TestHarness":
        """Add a benchmark. Name is optional -- omit for auto-naming.

        Usage:
            .add_benchmark("My bench", lambda: solve(...))
            .add_benchmark(lambda: solve(...))             # auto-named "Benchmark N"
        """
        if callable(name_or_fn):
            name, fn = None, name_or_fn
        else:
            name = name_or_fn
        self._steps.append(lambda name=name, fn=fn: self._execute_benchmark(name, fn))
        self._step_is_quick.append(False)
        return self

    def add_scaling(self, label: str, sizes: list[int],
                    generator, solver, reference=None) -> "TestHarness":
        """
        Add a scaling analysis that measures how solve time grows with input size.
        If reference is provided, shows side-by-side timing and memory comparison.

        Args:
            label:     description of the size parameter (e.g. "N (array size)")
            sizes:     list of input sizes to test, in ascending order
            generator: function(n) -> input of size n
            solver:    function(input) -> result
            reference: optional function(input) -> result for comparison
        """
        self._steps.append(
            lambda: self._execute_scaling(label, sizes, generator, solver, reference))
        self._step_is_quick.append(False)
        return self

    def add_random_tests(self, label: str, count: int,
                         case_generator: Callable[[], Case]) -> "TestHarness":
        """
        Add randomised stress tests comparing student solution vs reference.

        Args:
            label:          description of the stress test
            count:          number of random cases to generate
            case_generator: callable that returns a fresh Case each call
        """
        self._steps.append(
            lambda: self._execute_random_tests(label, count, case_generator))
        self._step_is_quick.append(True)
        return self

    # -- Running ----------------------------------------------

    def run(self) -> bool:
        """Run all steps. Returns True if every test passed."""
        return self._run_internal(quick_only=False)

    def run_quick(self) -> bool:
        """Run only correctness and random tests (skip benchmarks and scaling)."""
        return self._run_internal(quick_only=True)

    def _run_internal(self, quick_only: bool) -> bool:
        self._print_header()
        if quick_only:
            print("  (quick mode -- skipping benchmarks & scaling)\n")
        self._test_count = 0
        self._pass_count = 0
        self._step_index = 0

        for i, step in enumerate(self._steps):
            if quick_only and not self._step_is_quick[i]:
                continue
            self._step_index += 1
            step()
            print()

        self._print_summary(self._pass_count, self._test_count)
        return self._test_count > 0 and self._pass_count == self._test_count

    # -- Execution: correctness test --------------------------

    def _execute_test(self, name, fn, expected) -> bool:
        if name:
            print(f"  Test {self._step_index}: {name}")
        else:
            print(f"  Test {self._step_index}")

        for _ in range(self.WARMUP_RUNS):
            fn()

        start = time.perf_counter_ns()
        result = fn()
        elapsed_ns = time.perf_counter_ns() - start

        passed = result == expected

        if passed:
            print("    Status:   PASS")
        else:
            print("    Status:   FAIL")
            print(f"    Expected: {expected}")
            print(f"    Got:      {result}")
        print(f"    Time:     {elapsed_ns / 1_000_000:.3f} ms")

        return passed

    # -- Execution: benchmark ---------------------------------

    def _execute_benchmark(self, name, fn):
        if name:
            print(f"  Benchmark {self._step_index}: {name}")
        else:
            print(f"  Benchmark {self._step_index}")

        for _ in range(self.WARMUP_RUNS):
            fn()

        times = []
        for _ in range(self.BENCHMARK_RUNS):
            start = time.perf_counter_ns()
            fn()
            elapsed = time.perf_counter_ns() - start
            times.append(elapsed)

        avg_ms = (sum(times) / len(times)) / 1_000_000
        min_ms = min(times) / 1_000_000
        max_ms = max(times) / 1_000_000

        print(f"    Avg time: {avg_ms:.3f} ms  ({self.BENCHMARK_RUNS} runs)")
        print(f"    Min time: {min_ms:.3f} ms")
        print(f"    Max time: {max_ms:.3f} ms")

    # -- Execution: scaling analysis --------------------------

    def _execute_scaling(self, label: str, sizes: list[int],
                         generator, solver, reference=None):
        has_ref = reference is not None
        if has_ref:
            print(f"  Scaling {self._step_index}: {label} [vs reference]")
        else:
            print(f"  Scaling {self._step_index}: {label}")

        prev_ms = -1.0
        prev_size = -1

        for size in sizes:
            inp = generator(size)

            # Warmup solve
            for _ in range(self.WARMUP_RUNS):
                solver(inp)

            # Time solve
            total_ns = 0
            for _ in range(self.SCALING_RUNS):
                start = time.perf_counter_ns()
                solver(inp)
                total_ns += time.perf_counter_ns() - start
            solve_ms = (total_ns / self.SCALING_RUNS) / 1_000_000

            if not has_ref:
                if prev_ms <= 0:
                    print(f"    N = {size:>10,} | {solve_ms:>10.3f} ms")
                else:
                    time_ratio = solve_ms / prev_ms if prev_ms > 0 else 0
                    size_ratio = size / prev_size
                    print(f"    N = {size:>10,} | {solve_ms:>10.3f} ms"
                          f" | {time_ratio:>5.1f}x time for {size_ratio:>5.1f}x N")
            else:
                # Warmup + time reference
                for _ in range(self.WARMUP_RUNS):
                    reference(inp)
                ref_ns = 0
                for _ in range(self.SCALING_RUNS):
                    start = time.perf_counter_ns()
                    reference(inp)
                    ref_ns += time.perf_counter_ns() - start
                ref_ms = (ref_ns / self.SCALING_RUNS) / 1_000_000

                # Memory measurement via tracemalloc
                solve_mem = self._measure_memory(lambda: solver(inp))
                ref_mem = self._measure_memory(lambda: reference(inp))

                speedup = ref_ms / solve_ms if solve_ms > 0 else 0
                mem_str = f" | mem: {self._format_bytes(solve_mem)} vs {self._format_bytes(ref_mem)}"

                print(f"    N = {size:>10,} | solve: {solve_ms:>8.3f} ms"
                      f" | ref: {ref_ms:>8.3f} ms | {speedup:>5.1f}x{mem_str}")

            prev_ms = solve_ms
            prev_size = size

        print()
        if not has_ref:
            print("    Interpretation guide:")
            print("      ~1x time for kx N  =>  O(1)")
            print("      ~kx time for kx N  =>  O(N)")
            print("      ~k^2x for kx N     =>  O(N^2)")
        else:
            print("    speedup = ref_time / solve_time (higher is better)")
            print("    mem = peak memory allocated per call")

    # -- Execution: random stress tests -----------------------

    def _execute_random_tests(self, label: str, count: int,
                              case_generator: Callable[[], Case]):
        print(f"  Random Tests {self._step_index}: {label} ({count:,} cases)")

        passed = 0
        failures: list[str] = []

        for _ in range(count):
            tc = case_generator()
            result = tc.fn()
            if result == tc.expected:
                passed += 1
            elif len(failures) < self.MAX_FAILURES_SHOWN:
                failures.append(
                    f"    FAIL #{len(failures)+1}: {tc.description}\n"
                    f"      Expected: {tc.expected}\n"
                    f"      Got:      {result}")

        print(f"    Passed: {passed:,} / {count:,}")
        self._test_count += count
        self._pass_count += passed

        for f in failures:
            print(f)
        if count - passed > self.MAX_FAILURES_SHOWN:
            print(f"    ... and {count - passed - self.MAX_FAILURES_SHOWN:,} more failures")

    # -- Memory measurement helpers --------------------------

    @staticmethod
    def _measure_memory(fn) -> int:
        """Measure peak memory allocated by fn() using tracemalloc."""
        tracemalloc.start()
        fn()
        _, peak = tracemalloc.get_traced_memory()
        tracemalloc.stop()
        return peak

    @staticmethod
    def _format_bytes(b: int) -> str:
        if b < 1024:
            return f"{b} B"
        if b < 1024 * 1024:
            return f"{b / 1024:.1f} KB"
        return f"{b / (1024 * 1024):.1f} MB"

    # -- Formatting helpers -----------------------------------

    def _print_header(self):
        border = "=" * 55
        print(border)
        print(f"  Problem: {self._title}")
        print(border)
        print()

    def _print_summary(self, passed: int, total: int):
        border = "=" * 55
        print(border)
        if total == 0:
            print("  No correctness tests defined (benchmarks only).")
        else:
            print(f"  Summary: {passed:,} / {total:,} tests passed")
        print(border)
