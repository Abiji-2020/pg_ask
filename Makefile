BUILD_DIR = build

FMT_FILES := $(shell find src include -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \))
TIDY_FILES := $(shell find src -name '*.cpp')

fmt:
	clang-format -i $(FMT_FILES)

fmt-check:
	clang-format --dry-run --Werror $(FMT_FILES)

lint: build
	clang-tidy $(TIDY_FILES) -p $(BUILD_DIR)

build:
	cmake -S . -B $(BUILD_DIR) -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	cmake --build $(BUILD_DIR) -j $(nproc)
clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build
