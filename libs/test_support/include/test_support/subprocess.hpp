// SPDX-License-Identifier: MIT
/**
 * @file subprocess.hpp
 * @brief Lightweight subprocess runner for binary integration tests.
 *
 * Launches a child process with arbitrary arguments and environment,
 * captures stdout, stderr, and the exit code.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
#ifndef TEST_SUPPORT_SUBPROCESS_HPP
#define TEST_SUPPORT_SUBPROCESS_HPP

#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
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
 * @param fd File descriptor to read from.
 * @return The contents as a string.
 */
inline auto drain(int fd) -> std::string
{
    std::string buf;
    std::array<char, read_buffer_size> chunk{};
    for (;;)
    {
        auto n = ::read(fd, chunk.data(), chunk.size());
        if (n <= 0)
        {
            break;
        }
        buf.append(chunk.data(), static_cast<std::size_t>(n));
    }
    return buf;
}

/**
 * @brief Build a vector of C string pointers from a vector of strings.
 * @param strings The input strings.
 * @return A null-terminated vector of const char pointers.
 */
inline auto to_cstr_vec(const std::vector<std::string>& strings) -> std::vector<char*>
{
    std::vector<char*> result;
    result.reserve(strings.size() + 1);
    for (const auto& s : strings)
    {
        // POSIX exec requires char* const[], but does not modify the strings.
        result.push_back(const_cast<char*>(s.c_str())); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }
    result.push_back(nullptr);
    return result;
}

/**
 * @brief Run a subprocess and capture its output.
 * @param exe  Path to the executable.
 * @param args Command-line arguments (argv[0] is set to @p exe automatically).
 * @param env  Environment variables as "KEY=VALUE" strings.
 *             If empty, the child inherits the parent environment.
 * @return A Result containing exit code, stdout, and stderr.
 */
inline auto run(const std::string& exe,
                const std::vector<std::string>& args = {}, // NOLINT(bugprone-easily-swappable-parameters)
                const std::vector<std::string>& env = {}) -> Result
{
    // Build argv and envp
    std::vector<std::string> argv_strings;
    argv_strings.reserve(args.size() + 1);
    argv_strings.push_back(exe);
    argv_strings.insert(argv_strings.end(), args.begin(), args.end());

    auto argv = to_cstr_vec(argv_strings);
    auto envp = to_cstr_vec(env);

    // Create pipes for stdout and stderr
    std::array<int, 2> out_pipe{};
    std::array<int, 2> err_pipe{};
    if (::pipe(out_pipe.data()) != 0 || ::pipe(err_pipe.data()) != 0)
    {
        return {.exit_code = -1, .out = {}, .err = "pipe() failed"};
    }

    const pid_t pid = ::fork();
    if (pid == 0)
    {
        // Child: redirect stdout/stderr to pipes
        ::close(out_pipe[0]);
        ::close(err_pipe[0]);
        ::dup2(out_pipe[1], STDOUT_FILENO);
        ::dup2(err_pipe[1], STDERR_FILENO);
        ::close(out_pipe[1]);
        ::close(err_pipe[1]);

        if (env.empty())
        {
            ::execv(exe.c_str(), argv.data());
        }
        else
        {
            ::execve(exe.c_str(), argv.data(), envp.data());
        }
        ::_exit(exec_failed_exit_code);
    }

    // Parent: close write ends, read output
    ::close(out_pipe[1]);
    ::close(err_pipe[1]);

    auto out = drain(out_pipe[0]);
    auto err = drain(err_pipe[0]);

    ::close(out_pipe[0]);
    ::close(err_pipe[0]);

    int status = 0;
    ::waitpid(pid, &status, 0);

    return {
        .exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1,
        .out = std::move(out),
        .err = std::move(err),
    };
}

} // namespace subprocess

#endif // TEST_SUPPORT_SUBPROCESS_HPP
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
