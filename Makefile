# Build directory for CMake
BUILD_DIR = build

# Automatically gather source files
FMT_FILES := $(shell find src include -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.cc' \))
TIDY_FILES := $(shell find src -name '*.cpp')

# ---- Formatting --------------------------------------------------------------

fmt:
	clang-format -i $(FMT_FILES)

fmt-check:
	clang-format --dry-run --Werror $(FMT_FILES)

# ---- Linting (clang-tidy using compile_commands.json) ------------------------

lint: build
	clang-tidy $(TIDY_FILES) -p $(BUILD_DIR)

# ---- Build -------------------------------------------------------------------

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR) --parallel

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

# ---- Run ---------------------------------------------------------------------

run:
	./$(BUILD_DIR)/ai_sandbox


env:
	set -a ; \
	. ./.env ; \
	set +a ; \
	env


run-env:
	set -a ; \
	. ./.env ; \
	set +a ; \
	./$(BUILD_DIR)/ai_sandbox
