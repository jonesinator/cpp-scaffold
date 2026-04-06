// SPDX-License-Identifier: MIT
/**
 * @file subprocess.hpp
 * @brief Lightweight subprocess runner for binary integration tests.
 *
 * Launches a child process with arbitrary arguments and environment,
 * captures stdout, stderr, and the exit code.
 */

#ifndef TEST_SUPPORT_SUBPROCESS_HPP
#define TEST_SUPPORT_SUBPROCESS_HPP

// Clang source-based coverage instruments at the AST level, so we can use
// no_profile_instrument_function to exclude functions whose code runs in a
// forked child (invisible to coverage tools). GCC's gcov coverage uses
// LCOV_EXCL_LINE markers instead; applying the attribute under GCC causes
// build failures when always_inline std::allocator destructors are involved.
#ifdef __clang__
#define SUBPROCESS_NO_COVERAGE __attribute__((no_profile_instrument_function))
#else
#define SUBPROCESS_NO_COVERAGE
#endif

#include <array>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace subprocess
{

/// Exit code returned when exec fails in the child process.
constexpr int exec_failed_exit_code = 127;

/// Read buffer size for draining file descriptors.
constexpr std::size_t read_buffer_size = 4096;

/**
 * @brief Result of a completed subprocess.
 */
struct Result
{
    int exit_code;   ///< Exit code of the child process.
    std::string out; ///< Captured stdout.
    std::string err; ///< Captured stderr.
};

/**
 * @brief Read the entire contents of a file descriptor into a string.
 * @param pipe_fd File descriptor to read from.
 * @return The contents as a string.
 */
inline auto drain(int pipe_fd) -> std::string
{
    std::string buf;
    std::array<char, read_buffer_size> chunk{};
    for (;;)
    {
        auto count = ::read(pipe_fd, chunk.data(), chunk.size());
        if (count <= 0)
        {
            break;
        }
        buf.append(chunk.data(), static_cast<std::size_t>(count));
    }
    return buf;
}

/**
 * @brief Owns a vector of strings and a parallel null-terminated char* array.
 *
 * Keeps the string storage alive so the pointers in @c ptrs never dangle.
 */
class OwnedCStrVec
{
  public:
    /// Return the raw pointer array suitable for execv/execve.
    auto data() -> char**
    {
        return ptrs_.data();
    }

    /// Number of entries (excluding the null terminator).
    [[nodiscard]] auto size() const -> std::size_t
    {
        return storage_.size();
    }

    /// Access the nth pointer (bounds-checked).
    auto at(std::size_t idx) -> char*
    {
        return ptrs_.at(idx);
    }

  private:
    std::vector<std::string> storage_; ///< Owned copies of the strings.
    std::vector<char*> ptrs_;          ///< Null-terminated pointer array into @c storage_.

    friend auto to_cstr_vec(std::vector<std::string> strings) -> OwnedCStrVec;
};

/**
 * @brief Build an owning C-string vector from a vector of strings.
 * @param strings The input strings (taken by value to own them).
 * @return An OwnedCStrVec whose @c ptrs point into its own @c storage.
 */
inline auto to_cstr_vec(std::vector<std::string> strings) -> OwnedCStrVec
{
    OwnedCStrVec result;
    result.storage_ = std::move(strings);
    result.ptrs_.reserve(result.storage_.size() + 1);
    for (auto& str : result.storage_)
    {
        result.ptrs_.push_back(str.data());
    }
    result.ptrs_.push_back(nullptr);
    return result;
}

/**
 * @brief Child-side fork logic: redirect stdio to pipes, then exec.
 *
 * Marked no_profile_instrument_function because coverage tools cannot
 * observe this code: on success execv replaces the process image (wiping
 * coverage state); on failure _exit() bypasses the atexit flush that
 * writes .gcda / .profraw data. The behaviour IS tested end-to-end by
 * test_support_test — the lines just don't register in coverage output.
 */
// LCOV_EXCL_START
SUBPROCESS_NO_COVERAGE __attribute__((noreturn)) inline void
child_exec(std::array<int, 2>& in_pipe, std::array<int, 2>& out_pipe, std::array<int, 2>& err_pipe,
           const std::string& exe, char** argv, char** envp, bool inherit_env)
{
    ::close(in_pipe.at(1));
    ::close(out_pipe.at(0));
    ::close(err_pipe.at(0));
    ::dup2(in_pipe.at(0), STDIN_FILENO);
    ::dup2(out_pipe.at(1), STDOUT_FILENO);
    ::dup2(err_pipe.at(1), STDERR_FILENO);
    ::close(in_pipe.at(0));
    ::close(out_pipe.at(1));
    ::close(err_pipe.at(1));

    if (inherit_env)
    {
        ::execv(exe.c_str(), argv);
    }
    else
    {
        ::execve(exe.c_str(), argv, envp);
    }
    ::_exit(exec_failed_exit_code);
} // LCOV_EXCL_STOP

/**
 * @brief Write @p data to @p pipe_fd, retrying on partial writes.
 * @param pipe_fd File descriptor to write to.
 * @param data    Bytes to write.
 */
inline void write_all(int pipe_fd, std::string_view data)
{
    while (!data.empty())
    {
        const auto bytes_written = ::write(pipe_fd, data.data(), data.size());
        if (bytes_written <= 0)
        {
            break;
        }
        data.remove_prefix(static_cast<std::size_t>(bytes_written));
    }
}

/**
 * @brief Parent-side of a fork: pipe stdin to the child, drain stdout/stderr,
 *        wait for exit.
 *
 * Separated from run() so that this code — which IS reachable in the parent
 * process — can be instrumented for coverage, while run() (which contains the
 * fork + child dispatch) is marked no_profile_instrument_function.
 */
inline auto parent_wait(pid_t pid, std::array<int, 2>& in_pipe, std::array<int, 2>& out_pipe,
                        std::array<int, 2>& err_pipe, std::string_view stdin_data) -> Result
{
    ::close(in_pipe.at(0));
    ::close(out_pipe.at(1));
    ::close(err_pipe.at(1));

    // If the child exits before consuming all stdin, write() will get EPIPE
    // unless we ignore SIGPIPE. This is safe because write_all() also bails
    // on short/error returns.
    auto* prev_sigpipe = std::signal(SIGPIPE, SIG_IGN);
    write_all(in_pipe.at(1), stdin_data);
    ::close(in_pipe.at(1));
    (void)std::signal(SIGPIPE, prev_sigpipe);

    auto out = drain(out_pipe.at(0));
    auto err = drain(err_pipe.at(0));

    ::close(out_pipe.at(0));
    ::close(err_pipe.at(0));

    int status = 0;
    ::waitpid(pid, &status, 0);

    return {
        .exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1,
        .out = std::move(out),
        .err = std::move(err),
    };
}

/**
 * @brief Run a subprocess and capture its output.
 * @param exe        Path to the executable.
 * @param args       Command-line arguments (argv[0] is set to @p exe automatically).
 * @param env        Environment variables as "KEY=VALUE" strings.
 *                   If empty, the child inherits the parent environment.
 * @param stdin_data Bytes to feed to the child's stdin, then EOF. If empty,
 *                   the child sees immediate EOF on stdin.
 * @return A Result containing exit code, stdout, and stderr.
 *
 * Marked no_profile_instrument_function because this function contains
 * fork() + the child dispatch — code paths that coverage tools cannot
 * observe. The parent-side logic is in parent_wait(), which IS instrumented.
 */
SUBPROCESS_NO_COVERAGE inline auto run(const std::string& exe, const std::vector<std::string>& args = {},
                                       const std::vector<std::string>& env = {}, std::string_view stdin_data = {})
    -> Result
{
    // Build argv and envp
    std::vector<std::string> argv_strings;
    argv_strings.reserve(args.size() + 1);
    argv_strings.push_back(exe);
    argv_strings.insert(argv_strings.end(), args.begin(), args.end());

    auto argv = to_cstr_vec(argv_strings);
    auto envp = to_cstr_vec(env);

    // Create pipes for stdin, stdout, and stderr
    std::array<int, 2> in_pipe{};
    std::array<int, 2> out_pipe{};
    std::array<int, 2> err_pipe{};
    if (::pipe(in_pipe.data()) != 0 || ::pipe(out_pipe.data()) != 0 || ::pipe(err_pipe.data()) != 0)
    {
        return {.exit_code = -1, .out = {}, .err = "pipe() failed"}; // LCOV_EXCL_LINE
    }

    const pid_t pid = ::fork();
    if (pid == 0)
    {
        child_exec(in_pipe, out_pipe, err_pipe, exe, argv.data(), envp.data(), env.empty()); // LCOV_EXCL_LINE
    }

    return parent_wait(pid, in_pipe, out_pipe, err_pipe, stdin_data);
}

} // namespace subprocess

#endif // TEST_SUPPORT_SUBPROCESS_HPP
