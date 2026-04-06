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
#include <sstream>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace
{

// --------- subprocess::to_cstr_vec ---------

auto test_to_cstr_vec_empty() -> bool
{
    auto v = subprocess::to_cstr_vec({});
    return v.size() == 0 && v.at(0) == nullptr;
}

auto test_to_cstr_vec_populated() -> bool
{
    const std::vector<std::string> in{"a", "bb", "ccc"};
    auto v = subprocess::to_cstr_vec(in);
    expect::fail_test_when(v.size() != 3 || v.at(3) != nullptr, "to_cstr_vec: wrong size or missing nullptr sentinel");
    // Each pointer must reference the owned storage's string data.
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        expect::fail_test_when(std::string_view{v.at(i)} != in.at(i),
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
    (void)::write(fds.at(1), msg.data(), msg.size());
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
    (void)::write(fds.at(1), big.data(), big.size());
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
    auto r = subprocess::run("/bin/true");
    return r.exit_code == 0 && r.out.empty() && r.err.empty();
}

auto test_run_false_exits_nonzero() -> bool
{
    auto r = subprocess::run("/bin/false");
    return r.exit_code != 0 && r.exit_code != -1;
}

auto test_run_exec_failure_returns_127() -> bool
{
    auto r = subprocess::run("/nonexistent/binary/that/cannot/possibly/exist");
    return r.exit_code == subprocess::exec_failed_exit_code;
}

auto test_run_signal_termination_returns_minus_one() -> bool
{
    // Self-kill via SIGKILL; waitpid status has !WIFEXITED.
    auto r = subprocess::run("/bin/sh", {"-c", "kill -9 $$"});
    return r.exit_code == -1;
}

auto test_run_stdin_piped_to_cat() -> bool
{
    const std::string_view payload = "piped through cat\n";
    auto r = subprocess::run("/bin/cat", {}, {}, payload);
    return r.exit_code == 0 && r.out == payload && r.err.empty();
}

auto test_run_captures_stdout_and_stderr() -> bool
{
    auto r = subprocess::run("/bin/sh", {"-c", "printf OUT; printf ERR >&2"});
    return r.exit_code == 0 && r.out == "OUT" && r.err == "ERR";
}

auto test_run_env_is_passed_to_child() -> bool
{
    // Explicit env uses execve branch; PATH included so /bin/sh resolves echo.
    auto r = subprocess::run("/bin/sh", {"-c", "printf %s \"$FOO\""}, {"FOO=hello-from-env", "PATH=/usr/bin:/bin"});
    return r.exit_code == 0 && r.out == "hello-from-env";
}

auto test_run_default_env_inherits_parent() -> bool
{
    // Empty env uses execv branch; child inherits parent's environment.
    // Parent's PATH is set by ctest, so /bin/sh works without needing env.
    auto r = subprocess::run("/bin/sh", {"-c", "exit 0"});
    return r.exit_code == 0;
}

auto test_run_args_are_forwarded() -> bool
{
    auto r = subprocess::run("/bin/sh", {"-c", R"(printf '%s;%s;%s' "$0" "$1" "$2")", "one", "two", "three"});
    // argv[0] passed to sh -c is "one" (from the first positional after -c),
    // $1="two", $2="three". Verifies args survive the to_cstr_vec round-trip.
    return r.exit_code == 0 && r.out == "one;two;three";
}

// --------- expect::Suite ---------

auto test_expect_check_pass_silent() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite s("inner", out, err);
    const bool rv = s.check(true, "should-pass");
    return rv && s.failures() == 0 && out.str().empty() && err.str().empty();
}

auto test_expect_check_fail_logs() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite s("inner", out, err);
    const bool rv = s.check(false, "deliberate-fail");
    return !rv && s.failures() == 1 && out.str().empty() && err.str() == "FAIL: deliberate-fail\n";
}

auto test_expect_finish_success_path() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite s("inner", out, err);
    s.check(true, "a");
    s.check(true, "b");
    const int rc = s.finish();
    return rc == EXIT_SUCCESS && err.str().empty() && out.str() == "PASS: all inner tests\n";
}

auto test_expect_finish_failure_path() -> bool
{
    std::ostringstream out;
    std::ostringstream err;
    expect::Suite s("inner", out, err);
    s.check(false, "one");
    s.check(false, "two");
    const int rc = s.finish();
    const std::string e = err.str();
    return rc == EXIT_FAILURE && out.str().empty() && e.contains("FAIL: one\n") && e.contains("FAIL: two\n") &&
           e.contains("2 inner test(s) failed\n");
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
    return expect::expect_throws([] { throw std::runtime_error("boom"); });
}

auto test_expect_throws_calls_fail_test_on_no_throw() -> bool
{
    bool caught = false;
    try
    {
        expect::expect_throws([] {});
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
    expect::Suite s("test_support");
    s.check(test_to_cstr_vec_empty(), "to_cstr_vec/empty");
    s.check(test_to_cstr_vec_populated(), "to_cstr_vec/populated");
    s.check(test_drain_reads_pipe_content(), "drain/reads_pipe_content");
    s.check(test_drain_empty_pipe_returns_empty(), "drain/empty_pipe_returns_empty");
    s.check(test_drain_reads_large_payload(), "drain/reads_large_payload");
    s.check(test_write_all_happy_path(), "write_all/happy_path");
    s.check(test_write_all_broken_pipe_returns_early(), "write_all/broken_pipe_returns_early");
    s.check(test_run_true_exits_zero(), "run/true_exits_zero");
    s.check(test_run_false_exits_nonzero(), "run/false_exits_nonzero");
    s.check(test_run_exec_failure_returns_127(), "run/exec_failure_returns_127");
    s.check(test_run_signal_termination_returns_minus_one(), "run/signal_termination_returns_minus_one");
    s.check(test_run_stdin_piped_to_cat(), "run/stdin_piped_to_cat");
    s.check(test_run_captures_stdout_and_stderr(), "run/captures_stdout_and_stderr");
    s.check(test_run_env_is_passed_to_child(), "run/env_is_passed_to_child");
    s.check(test_run_default_env_inherits_parent(), "run/default_env_inherits_parent");
    s.check(test_run_args_are_forwarded(), "run/args_are_forwarded");
    s.check(test_expect_check_pass_silent(), "expect/check_pass_silent");
    s.check(test_expect_check_fail_logs(), "expect/check_fail_logs");
    s.check(test_expect_finish_success_path(), "expect/finish_success_path");
    s.check(test_expect_finish_failure_path(), "expect/finish_failure_path");
    s.check(test_fail_test_throws_test_failure(), "fail_test/throws_TestFailure");
    s.check(test_fail_test_when_true_throws(), "fail_test_when/true_throws");
    s.check(test_fail_test_when_false_does_nothing(), "fail_test_when/false_does_nothing");
    s.check(test_expect_throws_returns_true_on_throw(), "expect_throws/returns_true_on_throw");
    s.check(test_expect_throws_calls_fail_test_on_no_throw(), "expect_throws/calls_fail_test_on_no_throw");
    return s.finish();
}
