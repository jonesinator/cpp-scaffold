// SPDX-License-Identifier: MIT
/**
 * @file expect.hpp
 * @brief Minimal assertion/suite helper for unit and integration tests.
 *
 * Replaces the per-test `int failures = 0; if (!cond) { cerr << "FAIL"; ++failures; }`
 * pattern with a single Suite object that tracks failures, prints per-check
 * diagnostics, and returns an appropriate process exit code from finish().
 */

#ifndef TEST_SUPPORT_EXPECT_HPP
#define TEST_SUPPORT_EXPECT_HPP

#include <cstdlib>
#include <functional>
#include <iostream>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>

namespace expect
{

/**
 * @brief Groups a set of checks under a named suite and tracks failures.
 *
 * PASS summary is written to @p out_stream, FAIL messages and failure summary
 * are written to @p err_stream. Both default to std::cout / std::cerr; they
 * can be overridden to ostringstreams for self-testing.
 */
class Suite
{
  public:
    explicit Suite(std::string_view suite_name, std::ostream& out_stream = std::cout,
                   std::ostream& err_stream = std::cerr)
        : name_(suite_name), out_(&out_stream), err_(&err_stream)
    {
    }

    /// Log `FAIL: <label>` to err and increment failures if !condition.
    /// Returns @p condition so callers can short-circuit dependent checks.
    auto check(bool condition, std::string_view label) -> bool
    {
        if (!condition)
        {
            std::println(*err_, "FAIL: {}", label);
            ++failures_;
        }
        return condition;
    }

    /// Print summary and return EXIT_SUCCESS / EXIT_FAILURE.
    auto finish() -> int
    {
        if (failures_ != 0)
        {
            std::println(*err_, "{} {} test(s) failed", failures_, name_);
            return EXIT_FAILURE;
        }
        std::println(*out_, "PASS: all {} tests", name_);
        return EXIT_SUCCESS;
    }

    /// Number of failed checks so far (exposed for tests that need to inspect
    /// state without calling finish()).
    [[nodiscard]] auto failures() const -> int
    {
        return failures_;
    }

  private:
    std::string_view name_;
    std::ostream* out_;
    std::ostream* err_;
    int failures_ = 0;
};

/**
 * @brief Exception thrown by fail_test(). Inherits from std::logic_error
 * (not std::runtime_error) so that catch blocks for library errors
 * (which throw runtime_error) won't accidentally swallow test failures.
 */
class TestFailure : public std::logic_error
{
    using std::logic_error::logic_error;
};

/**
 * @brief Unconditionally fail the current test with a diagnostic message.
 * @param msg Description of why the test failed.
 */
[[noreturn]] inline void fail_test(const std::string& msg)
{
    throw TestFailure(msg);
}

/**
 * @brief Fail the current test if @p condition is true.
 *
 * Unlike an `if (cond) { fail_test(...); }` block, this is a single
 * function call — the call itself always executes, so gcov counts
 * the call-site line as covered regardless of whether the condition
 * fires.
 */
inline void fail_test_when(bool condition, const std::string& msg)
{
    if (condition)
    {
        throw TestFailure(msg);
    }
}

/**
 * @brief Assert that @p fn throws a std::runtime_error.
 *
 * Calls fail_test() if @p fn returns without throwing. Returns true
 * on a caught runtime_error so callers can write
 * `return expect_throws([] { ... });`.
 *
 * Takes std::function (not a template) so there is a single function body
 * in the binary. Both the "fn throws" and "fn doesn't throw" paths get
 * exercised through that one body, giving 100% coverage per instantiation.
 */
inline auto expect_throws(const std::function<void()>& fn) -> bool
{
    try
    {
        fn();
    }
    catch (const std::runtime_error&)
    {
        return true;
    }
    fail_test("expected exception was not thrown");
}

} // namespace expect

#endif // TEST_SUPPORT_EXPECT_HPP
