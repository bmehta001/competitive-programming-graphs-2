#pragma once
/**
 * A general-purpose testing and benchmarking harness for competitive programming problems.
 *
 * Usage:
 *   #include "test_harness.h"
 *
 *   harness::TestHarness<ReturnType>::forProblem("Problem Name")
 *       .addTest("test name", [](){ return solve(input); }, expected)
 *       .addBenchmark("large input", [&](){ return solve(large); })
 *       .addScaling("N", {100,1000,10000}, generator, solver)
 *       .addRandomTests("Stress", 1000, caseGenerator)
 *       .run();
 *
 * Compile & run any question:
 *   cl /std:c++17 /EHsc /O2 question1.cpp /Fe:q1.exe && q1.exe
 */

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <utility>
#include <cstdlib>
#include <new>

namespace harness {

// -- Heap allocation tracker -----------------------------

struct AllocTracker {
    static inline size_t bytes_allocated = 0;
    static inline bool active = false;

    static void reset()  { bytes_allocated = 0; active = true; }
    static size_t finish() { active = false; return bytes_allocated; }
};

} // namespace harness

// Global operator new/delete replacements for memory tracking.
// Only counts allocations while AllocTracker::active is true.
void* operator new(std::size_t size) {
    if (harness::AllocTracker::active) {
        harness::AllocTracker::bytes_allocated += size;
    }
    void* p = std::malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}
void* operator new[](std::size_t size) {
    if (harness::AllocTracker::active) {
        harness::AllocTracker::bytes_allocated += size;
    }
    void* p = std::malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

namespace harness {

// -- Format helpers (overload for each result type) -------

inline std::string to_str(bool v) { return v ? "true" : "false"; }
inline std::string to_str(int v) { return std::to_string(v); }
inline std::string to_str(long long v) { return std::to_string(v); }

template<typename A, typename B>
std::string to_str(const std::pair<A, B>& p) {
    return "{" + to_str(p.first) + ", " + to_str(p.second) + "}";
}

template<typename T>
std::string to_str(const std::vector<T>& v) {
    std::string s = "[";
    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) {
            s += ", ";
        }
        s += to_str(v[i]);
    }
    return s + "]";
}

// -- Case (for random stress tests) ---------------------

template<typename T>
struct Case {
    std::string description;
    std::function<T()> fn;
    T expected;
};

// -- TestHarness -----------------------------------------

template<typename T>
class TestHarness {
    static constexpr int WARMUP_RUNS = 3;
    static constexpr int BENCHMARK_RUNS = 10;
    static constexpr int SCALING_RUNS = 5;
    static constexpr int MAX_FAILURES_SHOWN = 3;

    std::string title_;
    std::vector<std::function<void(TestHarness&)>> steps_;
    std::vector<bool> step_is_quick_;
    int test_count_ = 0;
    int pass_count_ = 0;
    int step_index_ = 0;

    TestHarness() = default;

public:
    static TestHarness forProblem(const std::string& title) {
        TestHarness h;
        h.title_ = title;
        return h;
    }

    /** Add a correctness test with an expected result. */
    TestHarness& addTest(const std::string& name, std::function<T()> fn, const T& expected) {
        auto fn_copy = std::move(fn);
        steps_.push_back([name, fn_copy, expected](TestHarness& h) {
            h.test_count_++;
            if (h.executeTest(name, fn_copy, expected)) {
                h.pass_count_++;
            }
        });
        step_is_quick_.push_back(true);
        return *this;
    }

    /** Add a correctness test (auto-named "Test N"). */
    TestHarness& addTest(std::function<T()> fn, const T& expected) {
        return addTest("", std::move(fn), expected);
    }

    /** Add a benchmark (no correctness check, just timing). */
    TestHarness& addBenchmark(const std::string& name, std::function<T()> fn) {
        auto fn_copy = std::move(fn);
        steps_.push_back([name, fn_copy](TestHarness& h) {
            h.executeBenchmark(name, fn_copy);
        });
        step_is_quick_.push_back(false);
        return *this;
    }

    /** Add a benchmark (auto-named "Benchmark N"). */
    TestHarness& addBenchmark(std::function<T()> fn) {
        return addBenchmark("", std::move(fn));
    }

    /**
     * Add a scaling analysis that measures how solve time grows with input size.
     *
     * @param label     description of the size parameter
     * @param sizes     input sizes to test, in ascending order
     * @param generator function(int) -> Input
     * @param solver    function(Input) -> T
     */
    template<typename I>
    TestHarness& addScaling(const std::string& label,
            const std::vector<int>& sizes,
            std::function<I(int)> generator,
            std::function<T(const I&)> solver) {
        steps_.push_back([label, sizes, generator, solver](TestHarness& h) {
            h.executeScaling(label, sizes, generator, solver,
                             std::function<T(const I&)>(nullptr));
        });
        step_is_quick_.push_back(false);
        return *this;
    }

    /** Add scaling with a reference solution for side-by-side comparison. */
    template<typename I>
    TestHarness& addScaling(const std::string& label,
            const std::vector<int>& sizes,
            std::function<I(int)> generator,
            std::function<T(const I&)> solver,
            std::function<T(const I&)> reference) {
        steps_.push_back([label, sizes, generator, solver, reference](TestHarness& h) {
            h.executeScaling(label, sizes, generator, solver, reference);
        });
        step_is_quick_.push_back(false);
        return *this;
    }

    /**
     * Add randomised stress tests comparing student solution vs reference.
     *
     * @param label          description
     * @param count          number of random cases
     * @param caseGenerator  callable returning a fresh Case<T>
     */
    TestHarness& addRandomTests(const std::string& label, int count,
            std::function<Case<T>()> caseGenerator) {
        steps_.push_back([label, count, caseGenerator](TestHarness& h) {
            h.executeRandomTests(label, count, caseGenerator);
        });
        step_is_quick_.push_back(true);
        return *this;
    }

    /** Run all steps and return true if every test passed. */
    bool run() {
        return runInternal(false);
    }

    /** Run only correctness and random tests (skip benchmarks and scaling). */
    bool runQuick() {
        return runInternal(true);
    }

private:
    bool runInternal(bool quickOnly) {
        printHeader();
        if (quickOnly) {
            std::cout << "  (quick mode -- skipping benchmarks & scaling)\n\n";
        }
        test_count_ = 0;
        pass_count_ = 0;
        step_index_ = 0;

        for (size_t i = 0; i < steps_.size(); i++) {
            if (quickOnly && !step_is_quick_[i]) {
                continue;
            }
            step_index_++;
            steps_[i](*this);
            std::cout << "\n";
        }

        printSummary(pass_count_, test_count_);
        return test_count_ > 0 && pass_count_ == test_count_;
    }
    // -- Execution: correctness test -------------------------

    bool executeTest(const std::string& name, const std::function<T()>& fn, const T& expected) {
        if (!name.empty()) {
            std::cout << "  Test " << step_index_ << ": " << name << "\n";
        } else {
            std::cout << "  Test " << step_index_ << "\n";
        }

        for (int w = 0; w < WARMUP_RUNS; w++) {
            fn();
        }

        auto start = std::chrono::high_resolution_clock::now();
        T result = fn();
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        bool passed = (result == expected);

        if (passed) {
            std::cout << "    Status:   PASS\n";
        } else {
            std::cout << "    Status:   FAIL\n";
            std::cout << "    Expected: " << to_str(expected) << "\n";
            std::cout << "    Got:      " << to_str(result) << "\n";
        }
        std::cout << "    Time:     " << std::fixed << std::setprecision(3) << ms << " ms\n";

        return passed;
    }

    // -- Execution: benchmark --------------------------------

    void executeBenchmark(const std::string& name, const std::function<T()>& fn) {
        if (!name.empty()) {
            std::cout << "  Benchmark " << step_index_ << ": " << name << "\n";
        } else {
            std::cout << "  Benchmark " << step_index_ << "\n";
        }

        for (int w = 0; w < WARMUP_RUNS; w++) {
            fn();
        }

        std::vector<double> times(BENCHMARK_RUNS);
        for (int r = 0; r < BENCHMARK_RUNS; r++) {
            auto start = std::chrono::high_resolution_clock::now();
            fn();
            auto end = std::chrono::high_resolution_clock::now();
            times[r] = std::chrono::duration<double, std::milli>(end - start).count();
        }

        double min_t = *std::min_element(times.begin(), times.end());
        double max_t = *std::max_element(times.begin(), times.end());
        double avg_t = std::accumulate(times.begin(), times.end(), 0.0) / BENCHMARK_RUNS;

        std::cout << "    Avg time: " << std::fixed << std::setprecision(3) << avg_t
                  << " ms  (" << BENCHMARK_RUNS << " runs)\n";
        std::cout << "    Min time: " << std::fixed << std::setprecision(3) << min_t << " ms\n";
        std::cout << "    Max time: " << std::fixed << std::setprecision(3) << max_t << " ms\n";
    }

    // -- Execution: scaling analysis -------------------------

    template<typename I>
    void executeScaling(const std::string& label,
            const std::vector<int>& sizes,
            const std::function<I(int)>& generator,
            const std::function<T(const I&)>& solver,
            const std::function<T(const I&)>& reference) {
        bool hasRef = static_cast<bool>(reference);
        if (hasRef) {
            std::cout << "  Scaling " << step_index_ << ": " << label << " [vs reference]\n";
        } else {
            std::cout << "  Scaling " << step_index_ << ": " << label << "\n";
        }

        double prev_ms = -1.0;
        int prev_size = -1;

        for (int size : sizes) {
            I input = generator(size);

            // Warmup solve
            for (int w = 0; w < WARMUP_RUNS; w++) {
                solver(input);
            }

            // Time solve
            double solve_total = 0;
            for (int r = 0; r < SCALING_RUNS; r++) {
                auto start = std::chrono::high_resolution_clock::now();
                solver(input);
                auto end = std::chrono::high_resolution_clock::now();
                solve_total += std::chrono::duration<double, std::milli>(end - start).count();
            }
            double solve_ms = solve_total / SCALING_RUNS;

            if (!hasRef) {
                if (prev_ms <= 0) {
                    std::cout << "    N = " << std::setw(10) << size
                              << " | " << std::setw(10) << std::fixed << std::setprecision(3)
                              << solve_ms << " ms\n";
                } else {
                    double time_ratio = solve_ms / prev_ms;
                    double size_ratio = static_cast<double>(size) / prev_size;
                    std::cout << "    N = " << std::setw(10) << size
                              << " | " << std::setw(10) << std::fixed << std::setprecision(3)
                              << solve_ms << " ms"
                              << " | " << std::setw(5) << std::setprecision(1) << time_ratio
                              << "x time for " << std::setw(5) << std::setprecision(1)
                              << size_ratio << "x N\n";
                }
            } else {
                // Warmup + time reference
                for (int w = 0; w < WARMUP_RUNS; w++) {
                    reference(input);
                }
                double ref_total = 0;
                for (int r = 0; r < SCALING_RUNS; r++) {
                    auto start = std::chrono::high_resolution_clock::now();
                    reference(input);
                    auto end = std::chrono::high_resolution_clock::now();
                    ref_total += std::chrono::duration<double, std::milli>(end - start).count();
                }
                double ref_ms = ref_total / SCALING_RUNS;

                // Memory measurement via allocation tracker
                AllocTracker::reset();
                solver(input);
                size_t solve_mem = AllocTracker::finish();

                AllocTracker::reset();
                reference(input);
                size_t ref_mem = AllocTracker::finish();

                double speedup = (solve_ms > 0) ? ref_ms / solve_ms : 0;

                std::cout << "    N = " << std::setw(10) << size
                          << " | solve: " << std::setw(8) << std::fixed << std::setprecision(3)
                          << solve_ms << " ms"
                          << " | ref: " << std::setw(8) << ref_ms << " ms"
                          << " | " << std::setw(5) << std::setprecision(1) << speedup << "x"
                          << " | mem: " << formatBytes(solve_mem) << " vs "
                          << formatBytes(ref_mem) << "\n";
            }

            prev_ms = solve_ms;
            prev_size = size;
        }

        std::cout << "\n";
        if (!hasRef) {
            std::cout << "    Interpretation guide:\n";
            std::cout << "      ~1x time for kx N  =>  O(1)\n";
            std::cout << "      ~kx time for kx N  =>  O(N)\n";
            std::cout << "      ~k^2x for kx N     =>  O(N^2)\n";
        } else {
            std::cout << "    speedup = ref_time / solve_time (higher is better)\n";
            std::cout << "    mem = approx. heap bytes allocated per call\n";
        }
    }

    // -- Execution: random stress tests ----------------------

    void executeRandomTests(const std::string& label, int count,
            const std::function<Case<T>()>& caseGenerator) {
        std::cout << "  Random Tests " << step_index_ << ": " << label
                  << " (" << count << " cases)\n";

        int passed = 0;
        std::vector<std::string> failures;

        for (int i = 0; i < count; i++) {
            Case<T> tc = caseGenerator();
            T result = tc.fn();
            if (result == tc.expected) {
                passed++;
            } else if (static_cast<int>(failures.size()) < MAX_FAILURES_SHOWN) {
                std::ostringstream oss;
                oss << "    FAIL #" << (failures.size() + 1) << ": " << tc.description << "\n"
                    << "      Expected: " << to_str(tc.expected) << "\n"
                    << "      Got:      " << to_str(result);
                failures.push_back(oss.str());
            }
        }

        std::cout << "    Passed: " << passed << " / " << count << "\n";
        test_count_ += count;
        pass_count_ += passed;

        for (const auto& f : failures) {
            std::cout << f << "\n";
        }
        if (count - passed > MAX_FAILURES_SHOWN) {
            std::cout << "    ... and " << (count - passed - MAX_FAILURES_SHOWN) << " more failures\n";
        }
    }

    // -- Memory formatting ---------------------------------

    static std::string formatBytes(size_t bytes) {
        std::ostringstream oss;
        if (bytes < 1024) {
            oss << bytes << " B";
        } else if (bytes < 1024 * 1024) {
            oss << std::fixed << std::setprecision(1) << (bytes / 1024.0) << " KB";
        } else {
            oss << std::fixed << std::setprecision(1) << (bytes / (1024.0 * 1024)) << " MB";
        }
        return oss.str();
    }

    // -- Formatting ------------------------------------------

    void printHeader() {
        std::string border(55, '=');
        std::cout << border << "\n";
        std::cout << "  Problem: " << title_ << "\n";
        std::cout << border << "\n\n";
    }

    void printSummary(int passed, int total) {
        std::string border(55, '=');
        std::cout << border << "\n";
        if (total == 0) {
            std::cout << "  No correctness tests defined (benchmarks only).\n";
        } else {
            std::cout << "  Summary: " << passed << " / " << total << " tests passed\n";
        }
        std::cout << border << "\n";
    }
};

} // namespace harness
