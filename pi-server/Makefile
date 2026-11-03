BUILD ?= build
CLANG_TIDY ?= $(shell command -v clang-tidy 2>/dev/null || { p=$$(brew --prefix llvm 2>/dev/null) && [ -n "$$p" ] && [ -x "$$p/bin/clang-tidy" ] && printf '%s/bin/clang-tidy' "$$p"; } 2>/dev/null)
VENDOR_FILES := src/minitel/Minitel1B_Hard.cpp inc/Minitel1B_Hard.h
SRC_FILES := $(filter-out $(VENDOR_FILES),$(shell find src inc tests -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null))
CPP_SOURCES := $(filter-out src/minitel/Minitel1B_Hard.cpp,$(shell find src tests -name '*.cpp' 2>/dev/null))

.PHONY: configure build test test-frontend format format-check lint lint-cpp lint-frontend sanitize check

configure:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD) -j

test: build test-frontend
	ctest --test-dir $(BUILD) --output-on-failure

test-frontend:
	node --test tests/frontend/*.test.mjs

format:
	clang-format -i $(SRC_FILES)
	npx --yes @biomejs/biome format --write public tests/frontend

format-check:
	clang-format --dry-run --Werror $(SRC_FILES)
	npx --yes @biomejs/biome format public tests/frontend

lint-cpp: configure
	@test -n "$(CLANG_TIDY)" || { echo "error: clang-tidy not found. Install it or run: make CLANG_TIDY=/path/to/clang-tidy check" >&2; exit 1; }
	$(CLANG_TIDY) -p $(BUILD) $(CPP_SOURCES)

lint-frontend:
	npx --yes @biomejs/biome lint public tests/frontend

lint: lint-cpp lint-frontend

sanitize:
	cmake -S . -B $(BUILD)-san -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
	cmake --build $(BUILD)-san -j
	ctest --test-dir $(BUILD)-san --output-on-failure

check: format-check lint test sanitize
	@echo "ALL CHECKS PASSED"
