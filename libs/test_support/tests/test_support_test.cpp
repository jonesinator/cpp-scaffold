// SPDX-License-Identifier: MIT
/**
 * @file test_support_test.cpp
 * @brief Self-tests for subprocess.hpp and expect.hpp.
 */

#include "test_support/expect.hpp"
#include "test_support/subprocess.hpp"

#include <array>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace
{

// --------- subprocess::to_cstr_vec ---------

auto test_to_cstr_vec_empty() -> bool
{
    auto vec = subprocess::to_cstr_vec({});
    return vec.size() == 0 && vec.at(0) == nullptr;
}

auto test_to_cstr_vec_populated() -> bool
{
    const std::vector<std::string> input{"a", "bb", "ccc"};
    auto vec = subprocess::to_cstr_vec(input);
    expect::fail_test_when(vec.size() != 3 || vec.at(3) != nullptr,
                           "to_cstr_vec: wrong size or missing nullptr sentinel");
    // Each pointer must reference the owned storage's string data.
    for (std::size_t idx = 0; idx < vec.size(); ++idx)
    {
        expect::fail_test_when(std::string_view{vec.at(idx)} != input.at(idx),
                               "to_cstr_vec: pointer content does not match input");
    }
    return true;
}

// --------- subprocess::drain ---------

auto test_drain_reads_pipe_content() -> bool
{
    std::array<int, 2> fds{};
    expect::fail_test_when(::pipe(fds.data()) != 0, "pipe() failed");
    const std::string_view msg = "hello drain";
    auto bytes = ::write(fds.at(1), msg.data(), msg.size());
    expect::fail_test_when(bytes < 0, "write() failed"); // LCOV_EXCL_LINE
    ::close(fds.at(1));
    auto result = subprocess::drain(fds.at(0));
    ::close(fds.at(0));
    return result == msg;
}

auto test_drain_empty_pipe_returns_empty() -> bool
{
    std::array<int, 2> fds{};
    expect::fail_test_when(::pipe(fds.data()) != 0, "pipe() failed");
    ::close(fds.at(1)); // immediate EOF
    auto result = subprocess::drain(fds.at(0));
    ::close(fds.at(0));
    return result.empty();
}

auto test_drain_reads_large_payload() -> bool
{
    // Exceed read_buffer_size so the loop iterates more than once.
    const std::string big((subprocess::read_buffer_size * 3) + 17, 'x');
    std::array<int, 2> fds{};
    expect::fail_test_when(::pipe(fds.data()) != 0, "pipe() failed");
    // Write from a background-ish pattern: write then close.
    // For a ~12KB payload, pipe buffer (64KB on Linux) accommodates it in one
    // write without blocking.
    auto bytes = ::write(fds.at(1), big.data(), big.size());
    expect::fail_test_when(bytes < 0, "write() failed"); // LCOV_EXCL_LINE
    ::close(fds.at(1));
    auto result = subprocess::drain(fds.at(0));
    ::close(fds.at(0));
    return result == big;
}

// --------- subprocess::write_all ---------

auto test_write_all_happy_path() -> bool
{
    std::array<int, 2> fds{};
    expect::fail_test_when(::pipe(fds.data()) != 0, "pipe() failed");
    const std::string_view msg = "write_all payload";
    subprocess::write_all(fds.at(1), msg);
    ::close(fds.at(1));
    auto got = subprocess::drain(fds.at(0));
    ::close(fds.at(0));
    return got == msg;
}

auto test_write_all_broken_pipe_returns_early() -> bool
{
    // Close the read end before writing. write() returns -1 with EPIPE;
    // write_all must break out of its loop rather than spin.
    std::array<int, 2> fds{};
    expect::fail_test_when(::pipe(fds.data()) != 0, "pipe() failed");
    ::close(fds.at(0));
    auto* prev = std::signal(SIGPIPE, SIG_IGN);
    subprocess::write_all(fds.at(1), "data that cannot be delivered");
    (void)std::signal(SIGPIPE, prev);
    ::close(fds.at(1));
    return true; // survival = success (no infinite loop, no crash)
}

// --------- subprocess::run ---------

auto test_run_true_exits_zero() -> bool
{
    // Use sh -c instead of /bin/true directly — /bin/true doesn't exist in Nix sandboxes.
    auto result = subprocess::run("/bin/sh", {"-c", "true"});
    return result.exit_code == 0 && result.out.empty() && result.err.empty();
}

auto test_run_false_exits_nonzero() -> bool
{
    auto result = subprocess::run("/bin/sh", {"-c", "false"});
    return result.exit_code != 0 && result.exit_code != -1;
}

auto test_run_exec_failure_returns_127() -> bool
{
    auto result = subprocess::run("/nonexistent/binary/that/cannot/possibly/exist");
    return result.exit_code == subprocess::exec_failed_exit_code;
}

auto test_run_signal_termination_returns_minus_one() -> bool
{
    // Self-kill via SIGKILL; waitpid status has !WIFEXITED.
    auto result = subprocess::run("/bin/sh", {"-c", "kill -9 $$"});
    return result.exit_code == -1;
}

auto test_run_stdin_piped_to_cat() -> bool
{
    // Use sh -c instead of /bin/cat directly — /bin/cat doesn't exist in Nix sandboxes.
    const std::string_view payload = "piped through cat\n";
    auto result = subprocess::run("/bin/sh", {"-c", "cat"}, {}, payload);
    return result.exit_code == 0 && result.out == payload && result.err.empty();
}

auto test_run_captures_stdout_and_stderr() -> bool
{
    auto result = subprocess::run("/bin/sh", {"-c", "printf OUT; printf ERR >&2"});
    return result.exit_code == 0 && result.out == "OUT" && result.err == "ERR";
}

auto test_run_env_is_passed_to_child() -> bool
{
    // Explicit env uses execve branch; PATH included so /bin/sh resolves echo.
    auto result =
        subprocess::run("/bin/sh", {"-c", "printf %s \"$FOO\""}, {"FOO=hello-from-env", "PATH=/usr/bin:/bin"});
    return result.exit_code == 0 && result.out == "hello-from-env";
}

auto test_run_default_env_inherits_parent() -> bool
{
    // Empty env uses execv branch; child inherits parent's environment.
    // Parent's PATH is set by ctest, so /bin/sh works without needing env.
    auto result = subprocess::run("/bin/sh", {"-c", "exit 0"});
    return result.exit_code == 0;
}

auto test_run_args_are_forwarded() -> bool
{
    auto result = subprocess::run("/bin/sh", {"-c", R"(printf '%s;%s;%s' "$0" "$1" "$2")", "one", "two", "three"});
    // argv[0] passed to sh -c is "one" (from the first positional after -c),
    // $1="two", $2="three". Verifies args survive the to_cstr_vec round-trip.
    return result.exit_code == 0 && result.out == "one;two;three";
}

// --------- expect::Suite ---------

auto test_expect_check_pass_silent() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite suite("inner", out, err);
    const bool retval = suite.check(true, "should-pass");
    return retval && suite.failures() == 0 && out.str().empty() && err.str().empty();
}

auto test_expect_check_fail_logs() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite suite("inner", out, err);
    const bool retval = suite.check(false, "deliberate-fail");
    return !retval && suite.failures() == 1 && out.str().empty() && err.str() == "FAIL: deliberate-fail\n";
}

auto test_expect_finish_success_path() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite suite("inner", out, err);
    suite.check(true, "a");
    suite.check(true, "b");
    const int exit_code = suite.finish();
    return exit_code == EXIT_SUCCESS && err.str().empty() && out.str() == "PASS: all inner tests\n";
}

auto test_expect_finish_failure_path() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite suite("inner", out, err);
    suite.check(false, "one");
    suite.check(false, "two");
    const int exit_code = suite.finish();
    const std::string err_output = err.str();
    return exit_code == EXIT_FAILURE && out.str().empty() && err_output.contains("FAIL: one\n") &&
           err_output.contains("FAIL: two\n") && err_output.contains("2 inner test(s) failed\n");
}

// --------- expect::fail_test / expect_throws ---------

auto test_fail_test_throws_test_failure() -> bool
{
    bool caught = false;
    std::string message;
    try
    {
        expect::fail_test("deliberate");
    }
    catch (const expect::TestFailure& e)
    {
        caught = true;
        message = e.what();
    }
    expect::fail_test_when(!caught, "fail_test did not throw TestFailure");
    return message == "deliberate";
}

auto test_fail_test_when_true_throws() -> bool
{
    bool caught = false;
    try
    {
        expect::fail_test_when(true, "condition met");
    }
    catch (const expect::TestFailure&)
    {
        caught = true;
    }
    expect::fail_test_when(!caught, "fail_test_when(true, ...) did not throw");
    return true;
}

auto test_fail_test_when_false_does_nothing() -> bool
{
    expect::fail_test_when(false, "should not fire");
    return true;
}

auto test_expect_throws_returns_true_on_throw() -> bool
{
    return expect::expect_throws([]() -> void { throw std::runtime_error("boom"); });
}

auto test_expect_throws_calls_fail_test_on_no_throw() -> bool
{
    bool caught = false;
    try
    {
        expect::expect_throws([]() -> void {});
    }
    catch (const expect::TestFailure&)
    {
        caught = true;
    }
    expect::fail_test_when(!caught, "expect_throws did not call fail_test");
    return true;
}

} // namespace

auto main() -> int
{
    try
    {
        expect::Suite suite("test_support");
        suite.check(test_to_cstr_vec_empty(), "to_cstr_vec/empty");
        suite.check(test_to_cstr_vec_populated(), "to_cstr_vec/populated");
        suite.check(test_drain_reads_pipe_content(), "drain/reads_pipe_content");
        suite.check(test_drain_empty_pipe_returns_empty(), "drain/empty_pipe_returns_empty");
        suite.check(test_drain_reads_large_payload(), "drain/reads_large_payload");
        suite.check(test_write_all_happy_path(), "write_all/happy_path");
        suite.check(test_write_all_broken_pipe_returns_early(), "write_all/broken_pipe_returns_early");
        suite.check(test_run_true_exits_zero(), "run/true_exits_zero");
        suite.check(test_run_false_exits_nonzero(), "run/false_exits_nonzero");
        suite.check(test_run_exec_failure_returns_127(), "run/exec_failure_returns_127");
        suite.check(test_run_signal_termination_returns_minus_one(), "run/signal_termination_returns_minus_one");
        suite.check(test_run_stdin_piped_to_cat(), "run/stdin_piped_to_cat");
        suite.check(test_run_captures_stdout_and_stderr(), "run/captures_stdout_and_stderr");
        suite.check(test_run_env_is_passed_to_child(), "run/env_is_passed_to_child");
        suite.check(test_run_default_env_inherits_parent(), "run/default_env_inherits_parent");
        suite.check(test_run_args_are_forwarded(), "run/args_are_forwarded");
        suite.check(test_expect_check_pass_silent(), "expect/check_pass_silent");
        suite.check(test_expect_check_fail_logs(), "expect/check_fail_logs");
        suite.check(test_expect_finish_success_path(), "expect/finish_success_path");
        suite.check(test_expect_finish_failure_path(), "expect/finish_failure_path");
        suite.check(test_fail_test_throws_test_failure(), "fail_test/throws_TestFailure");
        suite.check(test_fail_test_when_true_throws(), "fail_test_when/true_throws");
        suite.check(test_fail_test_when_false_does_nothing(), "fail_test_when/false_does_nothing");
        suite.check(test_expect_throws_returns_true_on_throw(), "expect_throws/returns_true_on_throw");
        suite.check(test_expect_throws_calls_fail_test_on_no_throw(), "expect_throws/calls_fail_test_on_no_throw");
        return suite.finish();
    }
    // LCOV_EXCL_START
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    // LCOV_EXCL_STOP
}
