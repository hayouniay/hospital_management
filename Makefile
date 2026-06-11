# ==============================================================================
#  Hospital Management System — Makefile
#  Targets: configure, build, run, clean, rebuild, lint, format, install
#  Requirements: cmake, ninja (optional), clang-format, cppcheck
# ==============================================================================

# ── Project ───────────────────────────────────────────────────────────────────
PROJECT     := HospitalMS
VERSION     := 1.0.0
BUILD_DIR   := build
RELEASE_DIR := build-release
DEBUG_DIR   := build-debug

# ── Compiler ──────────────────────────────────────────────────────────────────
CXX   := g++
CMAKE := cmake

# ── Qt6 auto-detection ────────────────────────────────────────────────────────
# Override with: make build QT_PATH=/your/qt/path
QT_SEARCH_PATHS := \
    /usr/lib/x86_64-linux-gnu/cmake/Qt6 \
    /usr/lib/aarch64-linux-gnu/cmake/Qt6 \
    /usr/local/lib/cmake/Qt6 \
    /opt/Qt/6.7.0/gcc_64 \
    /opt/Qt/6.6.0/gcc_64 \
    /opt/Qt/6.5.0/gcc_64 \
    $(HOME)/Qt/6.7.0/gcc_64 \
    $(HOME)/Qt/6.6.0/gcc_64 \
    $(HOME)/Qt/6.5.0/gcc_64

QT_PATH ?= $(firstword $(foreach p,$(QT_SEARCH_PATHS),$(if $(wildcard $(p)),$(p),)))

ifneq ($(QT_PATH),)
    CMAKE_QT_FLAG := -DCMAKE_PREFIX_PATH="$(QT_PATH)"
else
    CMAKE_QT_FLAG :=
endif

# ── Build type ────────────────────────────────────────────────────────────────
BUILD_TYPE ?= Debug

# ── Ninja (faster builds if available) ───────────────────────────────────────
NINJA := $(shell command -v ninja 2>/dev/null)
ifneq ($(NINJA),)
    CMAKE_GENERATOR := -G Ninja
    BUILD_CMD        = ninja -C $(BUILD_DIR)
else
    CMAKE_GENERATOR :=
    BUILD_CMD        = $(CMAKE) --build $(BUILD_DIR) --parallel $(shell nproc)
endif

# ── Colours (use printf + \n — works in VSCode terminal, bash, sh) ────────────
# printf interprets escape sequences reliably; echo -e does not work everywhere.
RESET  := \033[0m
BOLD   := \033[1m
GREEN  := \033[32m
YELLOW := \033[33m
BLUE   := \033[34m
CYAN   := \033[36m
RED    := \033[31m

# Portable colour print macro: $(call log, COLOR, message)
define log
	@printf "$(1)$(2)$(RESET)\n"
endef

# ── Source files for lint/format ─────────────────────────────────────────────
SRCS := $(shell find src -name "*.cpp" -o -name "*.h" 2>/dev/null)

# ── WSL: suppress noisy runtime-dir permission warning ───────────────────────
# This is a harmless WSL2 quirk — Qt checks permissions on /mnt/wslg/runtime-dir
export QT_LOGGING_RULES := qt.qpa.*=false;*.warning=false
export XDG_RUNTIME_DIR  := /tmp/runtime-$(shell id -u)

# ==============================================================================
#  DEFAULT
# ==============================================================================
.DEFAULT_GOAL := help
.PHONY: help configure configure-release configure-debug build release debug \
        rebuild run run-release clean clean-all format format-check lint \
        check-deps install package info

# ==============================================================================
#  HELP
# ==============================================================================
help:
	@printf "\n"
	@printf "$(BOLD)$(BLUE)  ╔══════════════════════════════════════════════════╗$(RESET)\n"
	@printf "$(BOLD)$(BLUE)  ║   Hospital Management System — Build System      ║$(RESET)\n"
	@printf "$(BOLD)$(BLUE)  ║   Qt6 · C++23 · SQLite · CMake                  ║$(RESET)\n"
	@printf "$(BOLD)$(BLUE)  ╚══════════════════════════════════════════════════╝$(RESET)\n"
	@printf "\n"
	@printf "  $(BOLD)Build targets:$(RESET)\n"
	@printf "    $(GREEN)make configure$(RESET)       Configure cmake (Debug)\n"
	@printf "    $(GREEN)make build$(RESET)           Build the project (Debug)\n"
	@printf "    $(GREEN)make release$(RESET)         Configure + build in Release mode\n"
	@printf "    $(GREEN)make debug$(RESET)           Configure + build in Debug mode\n"
	@printf "    $(GREEN)make rebuild$(RESET)         Clean + configure + build\n"
	@printf "\n"
	@printf "  $(BOLD)Run targets:$(RESET)\n"
	@printf "    $(GREEN)make run$(RESET)             Build and run the application\n"
	@printf "    $(GREEN)make run-release$(RESET)     Build and run in Release mode\n"
	@printf "\n"
	@printf "  $(BOLD)Code quality:$(RESET)\n"
	@printf "    $(GREEN)make format$(RESET)          Format all source files (clang-format)\n"
	@printf "    $(GREEN)make format-check$(RESET)    Check formatting without modifying\n"
	@printf "    $(GREEN)make lint$(RESET)            Static analysis (cppcheck)\n"
	@printf "    $(GREEN)make check-deps$(RESET)      Verify all build dependencies\n"
	@printf "\n"
	@printf "  $(BOLD)Utility:$(RESET)\n"
	@printf "    $(GREEN)make clean$(RESET)           Remove build directory\n"
	@printf "    $(GREEN)make clean-all$(RESET)       Remove all build directories\n"
	@printf "    $(GREEN)make install$(RESET)         Install to /usr/local\n"
	@printf "    $(GREEN)make package$(RESET)         Create release tarball\n"
	@printf "    $(GREEN)make info$(RESET)            Show build configuration\n"
	@printf "\n"
	@printf "  $(BOLD)Options:$(RESET)\n"
	@printf "    $(YELLOW)QT_PATH$(RESET)=/path/to/qt    Override Qt6 path\n"
	@printf "    $(YELLOW)BUILD_TYPE$(RESET)=Release      Set cmake build type\n"
	@printf "\n"

# ==============================================================================
#  DEPENDENCY CHECK
# ==============================================================================
check-deps:
	@printf "\n$(BOLD)$(CYAN)Checking build dependencies...$(RESET)\n\n"
	@if command -v cmake >/dev/null 2>&1; then \
	    printf "  $(GREEN)✔$(RESET)  cmake        $$(cmake --version | head -1)\n"; \
	else \
	    printf "  $(RED)✘$(RESET)  cmake        NOT FOUND  →  sudo apt install cmake\n"; \
	fi
	@if command -v g++ >/dev/null 2>&1; then \
	    printf "  $(GREEN)✔$(RESET)  g++          $$(g++ --version | head -1)\n"; \
	else \
	    printf "  $(RED)✘$(RESET)  g++          NOT FOUND  →  sudo apt install g++-13\n"; \
	fi
	@if command -v ninja >/dev/null 2>&1; then \
	    printf "  $(GREEN)✔$(RESET)  ninja        $$(ninja --version)\n"; \
	else \
	    printf "  $(YELLOW)–$(RESET)  ninja        not found (optional)  →  sudo apt install ninja-build\n"; \
	fi
	@if [ -n "$(QT_PATH)" ]; then \
	    printf "  $(GREEN)✔$(RESET)  Qt6          $(QT_PATH)\n"; \
	else \
	    printf "  $(RED)✘$(RESET)  Qt6          NOT FOUND  →  sudo apt install qt6-base-dev libqt6charts6-dev\n"; \
	fi
	@if command -v clang-format >/dev/null 2>&1; then \
	    printf "  $(GREEN)✔$(RESET)  clang-format $$(clang-format --version)\n"; \
	else \
	    printf "  $(YELLOW)–$(RESET)  clang-format not found (optional)  →  sudo apt install clang-format\n"; \
	fi
	@if command -v cppcheck >/dev/null 2>&1; then \
	    printf "  $(GREEN)✔$(RESET)  cppcheck     $$(cppcheck --version)\n"; \
	else \
	    printf "  $(YELLOW)–$(RESET)  cppcheck     not found (optional)  →  sudo apt install cppcheck\n"; \
	fi
	@printf "\n"

# ==============================================================================
#  CONFIGURE
# ==============================================================================
configure:
	@printf "$(BOLD)$(CYAN)Configuring $(PROJECT) [$(BUILD_TYPE)]...$(RESET)\n"
	@mkdir -p $(BUILD_DIR)
	@$(CMAKE) \
	    -S . \
	    -B $(BUILD_DIR) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	    $(CMAKE_GENERATOR) \
	    $(CMAKE_QT_FLAG)
	@ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json 2>/dev/null || true
	@printf "$(GREEN)✔ Configuration complete$(RESET)\n"

# ==============================================================================
#  BUILD
# ==============================================================================
build: configure
	@printf "$(BOLD)$(CYAN)Building $(PROJECT) [$(BUILD_TYPE)]...$(RESET)\n"
	@$(BUILD_CMD)
	@printf "$(GREEN)✔ Build complete  →  $(BUILD_DIR)/$(PROJECT)$(RESET)\n"

release:
	@printf "$(BOLD)$(CYAN)Configuring $(PROJECT) [Release]...$(RESET)\n"
	@mkdir -p $(RELEASE_DIR)
	@$(CMAKE) \
	    -S . \
	    -B $(RELEASE_DIR) \
	    -DCMAKE_BUILD_TYPE=Release \
	    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	    $(CMAKE_GENERATOR) \
	    $(CMAKE_QT_FLAG)
	@ln -sf $(RELEASE_DIR)/compile_commands.json compile_commands.json 2>/dev/null || true
	@printf "$(BOLD)$(CYAN)Building $(PROJECT) [Release]...$(RESET)\n"
	@$(CMAKE) --build $(RELEASE_DIR) --parallel $(shell nproc)
	@printf "$(GREEN)✔ Release build complete  →  $(RELEASE_DIR)/$(PROJECT)$(RESET)\n"

debug:
	@printf "$(BOLD)$(CYAN)Configuring $(PROJECT) [Debug]...$(RESET)\n"
	@mkdir -p $(DEBUG_DIR)
	@$(CMAKE) \
	    -S . \
	    -B $(DEBUG_DIR) \
	    -DCMAKE_BUILD_TYPE=Debug \
	    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
	    $(CMAKE_GENERATOR) \
	    $(CMAKE_QT_FLAG)
	@printf "$(BOLD)$(CYAN)Building $(PROJECT) [Debug]...$(RESET)\n"
	@$(CMAKE) --build $(DEBUG_DIR) --parallel $(shell nproc)
	@printf "$(GREEN)✔ Debug build complete  →  $(DEBUG_DIR)/$(PROJECT)$(RESET)\n"

# ==============================================================================
#  RUN
# ==============================================================================
run: build
	@printf "$(BOLD)$(CYAN)Launching $(PROJECT) [$(BUILD_TYPE)]...$(RESET)\n"
	@mkdir -p /tmp/runtime-$(shell id -u)
	@chmod 700 /tmp/runtime-$(shell id -u)
	@./$(BUILD_DIR)/bin/$(PROJECT)

run-release: release
	@printf "$(BOLD)$(CYAN)Launching $(PROJECT) [Release]...$(RESET)\n"
	@mkdir -p /tmp/runtime-$(shell id -u)
	@chmod 700 /tmp/runtime-$(shell id -u)
	@./$(RELEASE_DIR)/bin/$(PROJECT)

# ==============================================================================
#  REBUILD
# ==============================================================================
rebuild: clean build
	@printf "$(GREEN)✔ Rebuild complete$(RESET)\n"

# ==============================================================================
#  CLEAN
# ==============================================================================
clean:
	@printf "$(YELLOW)Cleaning $(BUILD_DIR)/...$(RESET)\n"
	@rm -rf $(BUILD_DIR) compile_commands.json
	@printf "$(GREEN)✔ Clean complete$(RESET)\n"

clean-all:
	@printf "$(YELLOW)Cleaning all build directories...$(RESET)\n"
	@rm -rf $(BUILD_DIR) $(RELEASE_DIR) $(DEBUG_DIR) compile_commands.json dist/
	@printf "$(GREEN)✔ All build artifacts removed$(RESET)\n"

# ==============================================================================
#  FORMAT
# ==============================================================================
format:
	@if ! command -v clang-format >/dev/null 2>&1; then \
	    printf "$(RED)clang-format not found.$(RESET)  Install: sudo apt install clang-format\n"; \
	    exit 1; \
	fi
	@printf "$(BOLD)$(CYAN)Formatting source files...$(RESET)\n"
	@find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i --style=file
	@printf "$(GREEN)✔ Formatting complete$(RESET)\n"

format-check:
	@if ! command -v clang-format >/dev/null 2>&1; then \
	    printf "$(RED)clang-format not found$(RESET)\n"; exit 1; fi
	@printf "$(BOLD)$(CYAN)Checking formatting...$(RESET)\n"
	@find src -name "*.cpp" -o -name "*.h" | \
	    xargs clang-format --style=file --dry-run --Werror 2>&1 && \
	    printf "$(GREEN)✔ All files correctly formatted$(RESET)\n" || \
	    printf "$(RED)✘ Files need formatting  →  run: make format$(RESET)\n"

# ==============================================================================
#  LINT
# ==============================================================================
lint:
	@if ! command -v cppcheck >/dev/null 2>&1; then \
	    printf "$(RED)cppcheck not found.$(RESET)  Install: sudo apt install cppcheck\n"; \
	    exit 1; \
	fi
	@printf "$(BOLD)$(CYAN)Running static analysis...$(RESET)\n"
	@cppcheck \
	    --enable=all \
	    --std=c++23 \
	    --suppress=missingIncludeSystem \
	    --suppress=unusedFunction \
	    --suppress=unmatchedSuppression \
	    --error-exitcode=1 \
	    --quiet \
	    -I src \
	    src/ 2>&1 | tee lint-report.txt && \
	    printf "$(GREEN)✔ Lint passed. Report: lint-report.txt$(RESET)\n" || \
	    printf "$(RED)✘ Lint issues found. See lint-report.txt$(RESET)\n"

# ==============================================================================
#  INSTALL
# ==============================================================================
install: release
	@printf "$(BOLD)$(CYAN)Installing $(PROJECT)...$(RESET)\n"
	@$(CMAKE) --install $(RELEASE_DIR) --prefix /usr/local
	@printf "$(GREEN)✔ Installed to /usr/local/bin/$(PROJECT)$(RESET)\n"

# ==============================================================================
#  PACKAGE
# ==============================================================================
package: release
	@printf "$(BOLD)$(CYAN)Creating release package...$(RESET)\n"
	@mkdir -p dist
	@tar -czf dist/$(PROJECT)-$(VERSION)-linux-x86_64.tar.gz \
	    -C $(RELEASE_DIR) $(PROJECT) \
	    --transform 's|^|$(PROJECT)-$(VERSION)/|'
	@printf "$(GREEN)✔ Package: dist/$(PROJECT)-$(VERSION)-linux-x86_64.tar.gz$(RESET)\n"

# ==============================================================================
#  INFO
# ==============================================================================
info:
	@printf "\n"
	@printf "  $(BOLD)$(BLUE)$(PROJECT) v$(VERSION)$(RESET)\n\n"
	@printf "  Build type   : $(BOLD)$(BUILD_TYPE)$(RESET)\n"
	@printf "  Build dir    : $(BUILD_DIR)\n"
	@printf "  Generator    : $(if $(NINJA),Ninja,Unix Makefiles)\n"
	@printf "  Qt6 path     : $(if $(QT_PATH),$(QT_PATH),$(YELLOW)auto-detect$(RESET))\n"
	@printf "  Compiler     : $(shell $(CXX) --version 2>/dev/null | head -1)\n"
	@printf "  CMake        : $(shell cmake --version 2>/dev/null | head -1)\n"
	@printf "  CPU cores    : $(shell nproc)\n"
	@printf "  Sources      : $(shell find src -name '*.cpp' 2>/dev/null | wc -l) .cpp  /  $(shell find src -name '*.h' 2>/dev/null | wc -l) .h\n"
	@printf "\n"
