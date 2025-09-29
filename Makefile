ifeq ($(OS),Windows_NT)
  ifeq '$(findstring ;,$(PATH))' ';'
    UNIX_LIKE := FALSE
  else
    UNIX_LIKE := TRUE
  endif
else
  UNIX_LIKE := TRUE
endif

ifeq ($(UNIX_LIKE),FALSE)
  SHELL := powershell.exe
  .SHELLFLAGS := -NoProfile -NoLogo
  MKDIR := @$$null = new-item -itemtype directory -force
  TOUCH := @$$null = new-item -force
  RM := remove-item -force
  CMAKE := cmake
  CMAKE_GENERATOR := Ninja
  #CMAKE_GENERATOR := Unix Makefiles
  define rmdir
    if (Test-Path $1) { remove-item -recurse $1 }
  endef
else
  MKDIR := mkdir -p
  TOUCH := touch
  RM := rm -rf
  CMAKE := $(shell (command -v cmake3 || command -v cmake || echo cmake))
  #CMAKE_GENERATOR ?= "$(shell (command -v ninja > /dev/null 2>&1 && echo "Ninja") || echo "Unix Makefiles")"
  CMAKE_GENERATOR := Unix Makefiles
  define rmdir
    rm -rf $1
  endef
endif

# run parallel jobs
# run with 1 for debug
PARALLEL := --parallel 16

#CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Debug

MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR  := $(dir $(MAKEFILE_PATH))

filter-false = $(strip $(filter-out 0 off OFF false FALSE,$1))
filter-true = $(strip $(filter-out 1 on ON true TRUE,$1))

# See contrib/local.mk.example
-include local.mk

#all: collider
all: main

# Extra CMake flags which extend the default set
# @hey: add toolchain here?
CMAKE_EXTRA_FLAGS ?=
GOOSE_PRG := $(MAKEFILE_DIR)/build/bin/collider

# CMAKE_INSTALL_PREFIX
#   - May be passed directly or as part of CMAKE_EXTRA_FLAGS.
#   - `checkprefix` target checks that it matches the CMake-cached value. #9615
ifneq (,$(CMAKE_INSTALL_PREFIX)$(CMAKE_EXTRA_FLAGS))
CMAKE_INSTALL_PREFIX := $(shell echo $(CMAKE_EXTRA_FLAGS) | 2>/dev/null \
    grep -o 'CMAKE_INSTALL_PREFIX=[^ ]\+' | cut -d '=' -f2)
endif
ifneq (,$(CMAKE_INSTALL_PREFIX))
override CMAKE_EXTRA_FLAGS += -DCMAKE_INSTALL_PREFIX=$(CMAKE_INSTALL_PREFIX)

checkprefix:
	@if [ -f build/.ran-cmake ]; then \
	  cached_prefix=$(shell $(CMAKE) -L -N build | 2>/dev/null grep 'CMAKE_INSTALL_PREFIX' | cut -d '=' -f2); \
	  if ! [ "$(CMAKE_INSTALL_PREFIX)" = "$$cached_prefix" ]; then \
	    printf "Re-running CMake: CMAKE_INSTALL_PREFIX '$(CMAKE_INSTALL_PREFIX)' does not match cached value '%s'.\n" "$$cached_prefix"; \
	    $(RM) build/.ran-cmake; \
	  fi \
	fi
else
checkprefix: ;
endif

DEPS_BUILD_DIR ?= ".deps"
COLLIDER_BUILD_DIR ?= ".collider"
BUILD_DIR ?= "build"

ifneq (1,$(words [$(DEPS_BUILD_DIR)]))
  $(error DEPS_BUILD_DIR must not contain whitespace)
endif
ifneq (1,$(words [$(COLLIDER_BUILD_DIR)]))
  $(error COLLIDER_BUILD_DIR must not contain whitespace)
endif

DEPS_CMAKE_FLAGS ?= $(CMAKE_FLAGS) 
COLLIDER_CMAKE_FLAGS = $(DEPS_CMAKE_FLAGS)
USE_BUNDLED ?=

ifneq (,$(USE_BUNDLED))
  BUNDLED_CMAKE_FLAG := -DUSE_BUNDLED=$(USE_BUNDLED)
endif

ifneq (,$(findstring functionaltest-lua,$(MAKECMDGOALS)))
  BUNDLED_LUA_CMAKE_FLAG := -DUSE_BUNDLED_LUA=ON
  $(shell [ -x $(DEPS_BUILD_DIR)/usr/bin/lua ] || $(RM) build/.ran-*)
endif

# For use where we want to make sure only a single job is run.  This does issue 
# a warning, but we need to keep SCRIPTS argument.
SINGLE_MAKE = export MAKEFLAGS= ; $(MAKE)

#collider: build/.ran-cmake deps
main: $(BUILD_DIR)
	#$(CMAKE) -B $(BUILD_DIR) -G "$(CMAKE_GENERATOR)" $(CMAKE_FLAGS) $(CMAKE_EXTRA_FLAGS) $(MAKEFILE_DIR);
	$(CMAKE) --build build $(PARALLEL)

$(BUILD_DIR):
ifeq ($(UNIX_LIKE),FALSE)
	$(CMAKE) -B $(BUILD_DIR) -G "$(CMAKE_GENERATOR)" $(CMAKE_FLAGS) $(CMAKE_EXTRA_FLAGS) $(MAKEFILE_DIR);
else
	@echo "Checking if build directory '$(BUILD_DIR)' exists..."
	@if [ ! -d "$(BUILD_DIR)" ]; then \
		echo "Build directory not found. Running CMake configure..."; \
		$(CMAKE) -B $(BUILD_DIR) -G "$(CMAKE_GENERATOR)" $(CMAKE_FLAGS) $(CMAKE_EXTRA_FLAGS) $(MAKEFILE_DIR); \
	else \
		echo "Build directory '$(BUILD_DIR)' already exists. Skipping CMake configure."; \
	fi
endif

libcollider: build/.ran-cmake cmake.deps
	$(CMAKE) --build build --target libcollider

cmake:
	$(TOUCH) CMakeLists.txt
	$(MAKE) build/.ran-cmake

build/.ran-cmake: | deps
	$(CMAKE) -B build -G $(CMAKE_GENERATOR) $(CMAKE_FLAGS) $(CMAKE_EXTRA_FLAGS) $(MAKEFILE_DIR)
	$(TOUCH) $@

deps: | build/.ran-deps-cmake
ifeq ($(call filter-true,$(USE_BUNDLED)),)
	$(CMAKE) --build $(DEPS_BUILD_DIR) $(PARALLEL)
endif

ifeq ($(call filter-true,$(USE_BUNDLED)),)
$(DEPS_BUILD_DIR):
	$(MKDIR) $@

build/.ran-deps-cmake:: $(DEPS_BUILD_DIR)
	$(CMAKE) -S $(MAKEFILE_DIR)/cmake.deps -B $(DEPS_BUILD_DIR) -G $(CMAKE_GENERATOR) $(BUNDLED_CMAKE_FLAG) $(BUNDLED_LUA_CMAKE_FLAG) $(DEPS_CMAKE_FLAGS)
	#$(CMAKE) -S $(MAKEFILE_DIR)/deps -B $(DEPS_BUILD_DIR) -G $(CMAKE_GENERATOR) $(BUNDLED_CMAKE_FLAG) $(BUNDLED_LUA_CMAKE_FLAG) $(DEPS_CMAKE_FLAGS)
endif
build/.ran-deps-cmake::
	$(MKDIR) build
	$(TOUCH) "$@"

collider: $(COLLIDER_BUILD_DIR)
	$(CMAKE) --build $(COLLIDER_BUILD_DIR) $(PARALLEL)

$(COLLIDER_BUILD_DIR):
ifeq ($(UNIX_LIKE),FALSE)
		$(CMAKE) -S $(MAKEFILE_DIR)/cmake.collider -B $(COLLIDER_BUILD_DIR) -G $(CMAKE_GENERATOR) $(BUNDLED_CMAKE_FLAG) $(BUNDLED_LUA_CMAKE_FLAG) $(COLLIDER_CMAKE_FLAGS); 
else
		@echo "Checking if build directory '$(COLLIDER_BUILD_DIR)' exists..."
		@if [ ! -d "$(COLLIDER_BUILD_DIR)" ]; then \
			echo "Build directory not found. Running CMake configure..."; \
			$(CMAKE) -S $(MAKEFILE_DIR)/cmake.collider -B $(COLLIDER_BUILD_DIR) -G $(CMAKE_GENERATOR) $(BUNDLED_CMAKE_FLAG) $(BUNDLED_LUA_CMAKE_FLAG) $(COLLIDER_CMAKE_FLAGS); \
		else \
			echo "Build directory '$(COLLIDER_BUILD_DIR)' already exists. Skipping CMake configure."; \
		fi
endif
#	# done!

# TODO: cmake 3.2+ add_custom_target() has a USES_TERMINAL flag.
oldtest: | collider
	$(SINGLE_MAKE) -C test/old/testdir clean
ifeq ($(strip $(TEST_FILE)),)
	$(SINGLE_MAKE) -C test/old/testdir COLLIDER_PRG=$(COLLIDER_PRG) $(MAKEOVERRIDES)
else
	@# Handle TEST_FILE=test_foo{,.res,.vim}.
	$(SINGLE_MAKE) -C test/old/testdir COLLIDER_PRG=$(COLLIDER_PRG) SCRIPTS= $(MAKEOVERRIDES) $(patsubst %.vim,%,$(patsubst %.res,%,$(TEST_FILE)))
endif
# Build oldtest by specifying the relative .vim filename.
.PHONY: phony_force
test/old/testdir/%.vim: phony_force collider
	$(SINGLE_MAKE) -C test/old/testdir COLLIDER_PRG=$(COLLIDER_PRG) SCRIPTS= $(MAKEOVERRIDES) $(patsubst test/old/testdir/%.vim,%,$@)

functionaltest-lua: | collider
	$(CMAKE) --build build --target functionaltest

FORMAT=formatc formatlua format
LINT=lintlua lintsh lintc clang-analyzer lintcommit lintdoc lint luals
TEST=functionaltest unittest
generated-sources benchmark $(FORMAT) $(LINT) $(TEST) doc: | build/.ran-cmake
	$(CMAKE) --build build --target $@

test: $(TEST)

# iwyu-fix-includes can be downloaded from
# https://github.com/include-what-you-use/include-what-you-use/blob/master/fix_includes.py.
# Create a iwyu-fix-includes shell script in your $PATH that invokes the python script.
iwyu: build/.ran-cmake
	$(CMAKE) --preset iwyu
	$(CMAKE) --build build > build/iwyu.log
	iwyu-fix-includes --only_re="src/collider" --ignore_re="(src/collider/eval/encode.c\
	|src/collider/auto/\
	|src/collider/os/lang.c\
	|src/collider/map.c\
	)" --nosafe_headers < build/iwyu.log
	$(CMAKE) -B build -U ENABLE_IWYU
	$(CMAKE) --build build

clean:
ifneq ($(wildcard build),)
	$(CMAKE) --build build --target clean
endif
	$(MAKE) -C test/old/testdir clean
	$(MAKE) -C runtime/indent clean

distclean:
	$(call rmdir, $(DEPS_BUILD_DIR))
	$(call rmdir, build)
	$(MAKE) clean

install: checkprefix collider
	$(CMAKE) --install build

appimage:
	bash scripts/genappimage.sh

# Build an appimage with embedded update information.
#   appimage-nightly: for nightly builds
#   appimage-latest: for a release
appimage-%:
	bash scripts/genappimage.sh $*

.PHONY: test clean distclean collider libcollider cmake deps install appimage checkprefix benchmark $(FORMAT) $(LINT) $(TEST)
