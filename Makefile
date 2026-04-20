NAME := master-control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)

ifdef CROSS_COMPILE
CXX := $(CROSS_COMPILE)g++
endif

# Infer toolchain prefix from CXX when CROSS_COMPILE is not explicitly provided.
ifeq ($(strip $(CROSS_COMPILE)),)
ifneq ($(findstring g++,$(notdir $(CXX))),)
CROSS_COMPILE := $(patsubst %g++,%,$(CXX))
endif
endif

BUILD_DIR := build-make
OBJ_DIR := $(BUILD_DIR)/objNAME
TARGET := $(BUILD_DIR)/$(NAME)
COMPILER_STAMP := $(BUILD_DIR)/.compiler

LIBS_ROOT := libs
LOCAL_LIBREMOTE_DIR := $(LIBS_ROOT)/LIB-remote
LOCAL_LIBFUNCMOD_DIR := $(LIBS_ROOT)/LIB-funcmod
LOCAL_NANOGUI_DIR := $(LIBS_ROOT)/nanogui
LOCAL_LIBOSDP_DIR := $(LIBS_ROOT)/libosdp

LOCAL_LIBREMOTE_STATIC := $(LOCAL_LIBREMOTE_DIR)/build/libremote.a
LOCAL_LIBFUNCMOD_STATIC := $(LOCAL_LIBFUNCMOD_DIR)/build/libfuncmod.a
LOCAL_NANOGUI_BUILD_DIR := $(LOCAL_NANOGUI_DIR)/build
LOCAL_LIBOSDP_BUILD_DIR := $(LOCAL_LIBOSDP_DIR)/build

CMAKE_CROSS_ARGS := $(if $(strip $(CROSS_COMPILE)),-DCMAKE_C_COMPILER=$(CROSS_COMPILE)gcc -DCMAKE_CXX_COMPILER=$(CROSS_COMPILE)g++,)

THIRD_PARTY_INCLUDE_DIRS := \
	$(HOST_DIR)/include \
	$(HOST_DIR)/include/LIB-remote \
	$(HOST_DIR)/include/LIB-funcmod \
	$(HOST_DIR)/include/libosdp \
	$(TARGET_DIR)/usr/include \
	/usr/include \
	/usr/local/include

THIRD_PARTY_LIB_DIRS := \
	$(HOST_DIR)/usr/lib \
	$(TARGET_DIR)/usr/lib \
	/usr/lib \
	/usr/local/lib


ifeq ($(LOCAL),local)
THIRD_PARTY_INCLUDE_DIRS += ../nanogui_mod/include \
		../nanogui_mod/ext/nanovg \
		../nanogui_mod/ext/nanovg/src \
		../LIB-remote/src \
		../LIB-funcMod/src \
		../libosdp/include

THIRD_PARTY_LIB_DIRS += ../nanogui_mod/build \
		  ../LIB-remote/build \
		  ../LIB-funcMod/build \
		  ../libosdp/build/lib

CXXFLAGS += -DDEBUG -O0 -g
endif

FOUND_REMOTE_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/LIB-remote/remoteBadgeChannel.h) $(wildcard $(d)/libremote/remoteBadgeChannel.h) $(wildcard $(d)/remoteBadgeChannel.h)))
FOUND_FUNCMOD_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/LIB-funcmod/common/Path.hpp) $(wildcard $(d)/libfuncmod/common/Path.hpp) $(wildcard $(d)/common/Path.hpp)))
FOUND_NANOGUI_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/nanogui/nanogui.h)))
FOUND_OSDP_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/libosdp/osdp.hpp) $(wildcard $(d)/osdp.hpp)))

FOUND_REMOTE_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libremote.a)))
FOUND_FUNCMOD_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libfuncmod.so*) $(wildcard $(d)/libfuncmod.a)))
FOUND_NANOGUI_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libnanogui.so*) $(wildcard $(d)/libnanogui.a)))
FOUND_OSDP_STATIC_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libosdpstatic.a) $(wildcard $(d)/libosdp.a)))

NEED_LOCAL_LIBREMOTE := $(if $(and $(FOUND_REMOTE_HEADER),$(FOUND_REMOTE_LIB)),0,1)
NEED_LOCAL_LIBFUNCMOD := $(if $(and $(FOUND_FUNCMOD_HEADER),$(FOUND_FUNCMOD_LIB)),0,1)
NEED_LOCAL_NANOGUI := $(if $(and $(FOUND_NANOGUI_HEADER),$(FOUND_NANOGUI_LIB)),0,1)
NEED_LOCAL_LIBOSDP := $(if $(and $(FOUND_OSDP_HEADER),$(FOUND_OSDP_STATIC_LIB)),0,1)

# The Buildroot-provided nanogui may not include project-required symbols.
# For cross builds, prefer the bundled nanogui to keep headers/libs consistent.
ifneq ($(strip $(CROSS_COMPILE)),)
NEED_LOCAL_NANOGUI := 1
endif

OSDP_LINK_FILE := $(FOUND_OSDP_STATIC_LIB)
ifeq ($(NEED_LOCAL_LIBOSDP),1)
OSDP_LINK_FILE := $(LOCAL_LIBOSDP_BUILD_DIR)/lib/libosdpstatic.a
endif

SRCS	:= 	$(wildcard src/*.cpp) \
			$(wildcard src/Controller/*.cpp) \
			$(wildcard src/Controller/Readers/*.cpp) \
			$(wildcard src/Peripherals/*.cpp) \
			$(wildcard src/GUI/*.cpp) \
			$(wildcard src/GUI/Buttons/*.cpp) \
			$(wildcard src/GUI/Pages/*.cpp)

OBJS	:= $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS	:= $(OBJS:.o=.d)
		
CXXFLAGS += -DNANOGUI_USE_OPENGL

INCS 	:= 	-I. \
		-Isrc \
		-Isrc/Controller \
		-Isrc/Peripherals \
		-Isrc/GUI \
		-Isrc/GUI/Buttons \
		-Isrc/GUI/Pages \
		-I$(HOST_DIR)/include \
		-I$(HOST_DIR)/include/nanovg \
		-I$(HOST_DIR)/include/nanogui/ext/nanovg/src \
		-I$(HOST_DIR)/include/LIB-funcmod/ \
		-I$(HOST_DIR)/include/LIB-remote/ \
		-I$(HOST_DIR)/include/libosdp


INCS += -I../nanogui_mod/include \
		-I../nanogui_mod/ext/nanovg \
		-I../nanogui_mod/ext/nanovg/src \
		-I../LIB-remote/src \
		-I../LIB-funcMod/src \
		-I../libosdp/include

ifeq ($(NEED_LOCAL_LIBREMOTE),1)
INCS += -I$(LOCAL_LIBREMOTE_DIR)/src
endif

ifeq ($(NEED_LOCAL_LIBFUNCMOD),1)
INCS += -I$(LOCAL_LIBFUNCMOD_DIR)/src
endif

ifeq ($(NEED_LOCAL_NANOGUI),1)
INCS := -I$(LOCAL_NANOGUI_DIR)/include \
		-I$(LOCAL_NANOGUI_DIR)/ext/nanovg/src \
		$(INCS)
endif

ifeq ($(NEED_LOCAL_LIBOSDP),1)
INCS += -I$(LOCAL_LIBOSDP_DIR)/include
endif
	 	
NANOGUI_LINK_FILE := $(FOUND_NANOGUI_LIB)
ifeq ($(NEED_LOCAL_NANOGUI),1)
NANOGUI_LINK_FILE := $(LOCAL_NANOGUI_BUILD_DIR)/libnanogui.a
endif

LIBS	:= $(NANOGUI_LINK_FILE) \
			-lremote \
			-lfuncmod \
			$(OSDP_LINK_FILE) \
			-lcrypto

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L../nanogui_mod/build \
		-L../LIB-remote/build \
		-L../LIB-funcMod/build

ifeq ($(NEED_LOCAL_LIBREMOTE),1)
LIBDIR += -L$(LOCAL_LIBREMOTE_DIR)/build
endif

ifeq ($(NEED_LOCAL_LIBFUNCMOD),1)
LIBDIR += -L$(LOCAL_LIBFUNCMOD_DIR)/build
endif

ifeq ($(NEED_LOCAL_NANOGUI),1)
LIBDIR := -L$(LOCAL_NANOGUI_BUILD_DIR) $(LIBDIR)
endif

ifeq ($(NEED_LOCAL_LIBOSDP),1)
LIBDIR += -L$(LOCAL_LIBOSDP_BUILD_DIR)/lib
endif

LDFLAGS += -Wl,-rpath,$(HOST_DIR)

BUILD ?= release

ifeq ($(BUILD),debug)
CXXFLAGS += -DDEBUG -O0 -g
endif

ifeq ($(BUILD),release)
CXXFLAGS += -O2
endif

.PHONY: all clean $(NAME)-linter debug release prepare-build prepare-thirdparty

all: $(TARGET)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

local:
	$(MAKE) LOCAL=local all

$(NAME): $(TARGET)
	@echo "Built $(TARGET)"

prepare-build: $(COMPILER_STAMP)

$(COMPILER_STAMP): | $(BUILD_DIR)
	@if [ -f $@ ] && [ "`cat $@`" != "$(CXX)" ]; then rm -rf $(OBJ_DIR) $(TARGET) $(LOCAL_LIBREMOTE_DIR)/build $(LOCAL_LIBFUNCMOD_DIR)/build $(LOCAL_NANOGUI_BUILD_DIR) $(LOCAL_LIBOSDP_BUILD_DIR); fi
	@printf '%s\n' '$(CXX)' > $@

prepare-thirdparty:
	@if [ "$(NEED_LOCAL_LIBREMOTE)" = "1" ] && [ ! -f "$(LOCAL_LIBREMOTE_STATIC)" ]; then \
		echo "[deps] Building local libremote"; \
		$(MAKE) -C "$(LOCAL_LIBREMOTE_DIR)" BUILD=$(BUILD) CROSS_COMPILE=$(CROSS_COMPILE) static; \
	fi
	@if [ "$(NEED_LOCAL_LIBFUNCMOD)" = "1" ] && [ ! -f "$(LOCAL_LIBFUNCMOD_STATIC)" ]; then \
		echo "[deps] Building local libfuncmod"; \
		$(MAKE) -C "$(LOCAL_LIBFUNCMOD_DIR)" BUILD=$(BUILD) CROSS_COMPILE=$(CROSS_COMPILE) static; \
	fi
	@if [ "$(NEED_LOCAL_NANOGUI)" = "1" ] && ! ls "$(LOCAL_NANOGUI_BUILD_DIR)"/libnanogui* >/dev/null 2>&1; then \
		echo "[deps] Building local nanogui"; \
		cmake -S "$(LOCAL_NANOGUI_DIR)" -B "$(LOCAL_NANOGUI_BUILD_DIR)" $(CMAKE_CROSS_ARGS) -DCMAKE_BUILD_TYPE=$(if $(filter $(BUILD),debug),Debug,Release); \
		cmake --build "$(LOCAL_NANOGUI_BUILD_DIR)"; \
	fi
	@if [ "$(NEED_LOCAL_LIBOSDP)" = "1" ] && [ -n "$(CROSS_COMPILE)" ] && [ -f "$(LOCAL_LIBOSDP_BUILD_DIR)/CMakeCache.txt" ] && [ -f "$(LOCAL_LIBOSDP_BUILD_DIR)/lib/libosdpstatic.a" ] && ! grep -q "CMAKE_CXX_COMPILER:FILEPATH=$(CROSS_COMPILE)g++" "$(LOCAL_LIBOSDP_BUILD_DIR)/CMakeCache.txt"; then \
		echo "[deps] Purging local libosdp built with a different compiler"; \
		rm -rf "$(LOCAL_LIBOSDP_BUILD_DIR)"; \
	fi
	@if [ "$(NEED_LOCAL_LIBOSDP)" = "1" ] && ! ls "$(LOCAL_LIBOSDP_BUILD_DIR)"/lib/libosdp* >/dev/null 2>&1; then \
		echo "[deps] Building local libosdp"; \
		cmake -S "$(LOCAL_LIBOSDP_DIR)" -B "$(LOCAL_LIBOSDP_BUILD_DIR)" $(CMAKE_CROSS_ARGS) -DOPT_OSDP_LIB_ONLY=ON -DOPT_BUILD_STATIC=ON -DOPT_BUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=$(if $(filter $(BUILD),debug),Debug,Release); \
		cmake --build "$(LOCAL_LIBOSDP_BUILD_DIR)"; \
	fi

$(TARGET): prepare-build prepare-thirdparty $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBDIR) $(LIBS)
	ln -sf $(TARGET) $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC $(INCS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	
clean:
	rm -rf $(BUILD_DIR) $(NAME) \
		$(LOCAL_LIBREMOTE_DIR)/build \
		$(LOCAL_LIBFUNCMOD_DIR)/build \
		$(LOCAL_NANOGUI_BUILD_DIR) \
		$(LOCAL_LIBOSDP_BUILD_DIR)
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(TARGET)

-include $(DEPS)
