set dotenv-load := false

default_profile := "debug"
opener := if os() == "macos" { "open" } else if os() == "windows" { "start" } else { "xdg-open" }


# List available recipes
default:
    @just --list
    @echo ""
    @echo "Profiles: debug (default), release, coverage, clang-coverage, asan, tsan"
    @echo "Example:  just test asan"

# Configure a build (debug, release, coverage, asan, tsan)
configure profile=default_profile:
    cmake --preset {{profile}}

# Build the project
build profile=default_profile: (configure profile)
    cmake --build --preset {{profile}}

# Run all tests
test profile=default_profile: (build profile)
    ctest --preset {{profile}}

# Run a single test by name
test-one name profile=default_profile: (build profile)
    ctest --preset {{profile}} -R {{name}}

# Run tests and generate HTML coverage report
coverage: (test "coverage")
    cmake --build build/coverage --target coverage-html
    @echo "Report: build/coverage/coverage/index.html"

# Run tests and generate machine-readable coverage report
coverage-report: (test "coverage")
    cmake --build build/coverage --target coverage-report
    @echo "Report: build/coverage/coverage.info"

# Run tests and generate Clang source-based HTML coverage report
clang-coverage: (test "clang-coverage")
    cmake --build build/clang-coverage --target clang-coverage-html
    @echo "Report: build/clang-coverage/coverage/index.html"

# Run tests and generate Clang source-based coverage summary
clang-coverage-report: (test "clang-coverage")
    cmake --build build/clang-coverage --target clang-coverage-report
    @echo "Report: build/clang-coverage/coverage.lcov"

# Open the Clang coverage HTML report in a browser
open-clang-coverage: clang-coverage
    {{opener}} build/clang-coverage/coverage/index.html

# Format all source files
format:
    find libs bin -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i

# Check formatting without modifying files
format-check:
    find libs bin -name '*.cpp' -o -name '*.hpp' | xargs -n 1 -t clang-format --dry-run --Werror

# Run clang-tidy on the codebase
lint profile=default_profile: (build profile)
    find libs bin -name '*.cpp' | xargs clang-tidy -p build/{{profile}}

# Generate HTML and PDF documentation
docs:
    mkdir -p build
    doxygen Doxyfile
    make -C build/docs/latex
    @echo "HTML: build/docs/html/index.html"
    @echo "PDF:  build/docs/latex/refman.pdf"

# Generate HTML documentation only
docs-html:
    mkdir -p build
    doxygen Doxyfile
    @echo "HTML: build/docs/html/index.html"

# Generate CMake target dependency graph as SVG
deps profile=default_profile: (configure profile)
    cmake --graphviz=build/deps.dot --preset {{profile}}
    dot -Tsvg build/deps.dot -o build/deps.svg
    @echo "Graph: build/deps.svg"

# Open the coverage HTML report in a browser
open-coverage: coverage
    {{opener}} build/coverage/coverage/index.html

# Open the Doxygen HTML docs in a browser
open-docs: docs-html
    {{opener}} build/docs/html/index.html

# Open the CMake dependency graph
open-deps: deps
    {{opener}} build/deps.svg

# Install pre-commit hooks and other dev tooling
setup:
    install -m 755 scripts/pre-commit .git/hooks/pre-commit
    @echo "Pre-commit hook installed"

# Install to system prefix (default: /usr/local)
install profile="release": (build profile)
    cmake --install build/{{profile}}

# Install to a custom prefix
install-to prefix profile="release": (build profile)
    cmake --install build/{{profile}} --prefix {{prefix}}

# Create distribution packages (TGZ, DEB, RPM)
package profile="release": (build profile)
    cd build/{{profile}} && cpack

# Verify release artifacts reproduce byte-for-byte (via Debian reprotest)
verify-reproducibility:
    scripts/verify-reproducibility.sh

# Full check: build all profiles, test, coverage, sanitizers, lint, format, docs, deps
check:
    @echo "===== format-check ====="
    just format-check
    @echo ""
    @echo "===== build + test (debug) ====="
    just test debug
    @echo ""
    @echo "===== build + test (release) ====="
    just test release
    @echo ""
    @echo "===== build + test (asan) ====="
    just test asan
    @echo ""
    @echo "===== build + test (tsan) ====="
    just test tsan
    @echo ""
    @echo "===== coverage ====="
    just coverage
    @echo ""
    @echo "===== lint ====="
    just lint
    @echo ""
    @echo "===== docs ====="
    just docs
    @echo ""
    @echo "===== deps ====="
    just deps
    @echo ""
    @echo "All checks passed."

# Build a CI container image (trixie, etc.)
ci-build distro="trixie":
    podman build -t scaffold-ci-{{distro}} -f ci/{{distro}}/Dockerfile .

# Remove all build artifacts
clean:
    rm -rf build
