# Convenience wrapper around CMake. The build is plain CMake; these targets
# just save some typing.
#
#   make              configure + build collider (system dependencies)
#   make gui          build with the SDL GUI (-DENABLE_GUI=ON)
#   make test         build and run the test suite
#   make deps         build the bundled dependencies into .deps/usr
#   make BUNDLED=ON   build collider against the bundled dependencies
#   make clean        remove the build directory
#   make distclean    remove the build and bundled-deps directories

CMAKE          ?= cmake
GENERATOR      ?= Ninja
BUILD_DIR      ?= build
DEPS_BUILD_DIR ?= .deps
BUILD_TYPE     ?= RelWithAsserts
JOBS           ?= 16

# toggles
ENABLE_GUI ?= OFF
BUNDLED    ?= OFF

CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DENABLE_GUI=$(ENABLE_GUI) -DUSE_BUNDLED_DEPS=$(BUNDLED)

.PHONY: all build gui test deps clean distclean

all: build

build:
	$(CMAKE) -S . -B $(BUILD_DIR) -G $(GENERATOR) $(CMAKE_FLAGS)
	$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)

gui:
	$(MAKE) build ENABLE_GUI=ON

test: build
	cd $(BUILD_DIR) && ctest --output-on-failure

deps:
	$(CMAKE) -S cmake.deps -B $(DEPS_BUILD_DIR) -G $(GENERATOR) -DCMAKE_BUILD_TYPE=Release
	$(CMAKE) --build $(DEPS_BUILD_DIR) --parallel $(JOBS)

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)

distclean: clean
	$(CMAKE) -E rm -rf $(DEPS_BUILD_DIR)
