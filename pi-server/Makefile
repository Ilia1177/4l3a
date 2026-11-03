BUILD ?= build
SRC_FILES := $(shell find src inc tests -name '*.cpp' -o -name '*.hpp' -o -name '*.h' 2>/dev/null)
CPP_SOURCES := $(shell find src -name '*.cpp' 2>/dev/null)

.PHONY: configure build test format format-check lint lint-cpp lint-public sanitize check

configure:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Debug

build: configure
	cmake --build $(BUILD) -j

test: build
	ctest --test-dir $(BUILD) --output-on-failure

format:
	clang-format -i $(SRC_FILES)
	npx --yes @biomejs/biome format --write public

format-check:
	clang-format --dry-run --Werror $(SRC_FILES)
	npx --yes @biomejs/biome format public

lint-cpp: configure
	clang-tidy -p $(BUILD) $(CPP_SOURCES)

lint-frontend:
	npx --yes @biomejs/biome lint public

lint: lint-cpp lint-frontend

sanitize:
	cmake -S . -B $(BUILD)-san -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
	cmake --build $(BUILD)-san -j
	ctest --test-dir $(BUILD)-san --output-on-failure

check: format-check lint test sanitize
	@echo "ALL CHECKS PASSED"
