NAME := master-control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)

ifdef CROSS_COMPILE
CXX := $(CROSS_COMPILE)g++
endif

BUILD_DIR := build-make
OBJ_DIR := $(BUILD_DIR)/objNAME
TARGET := $(BUILD_DIR)/$(NAME)
COMPILER_STAMP := $(BUILD_DIR)/.compiler

LIBS_ROOT := libs
LOCAL_LIBREMOTE_DIR := $(LIBS_ROOT)/LIB-remote
LOCAL_LIBFUNCMOD_DIR := $(LIBS_ROOT)/LIB-funcmod
LOCAL_NANOGUI_DIR := $(LIBS_ROOT)/nanogui

LOCAL_LIBREMOTE_STATIC := $(LOCAL_LIBREMOTE_DIR)/build/libremote.a
LOCAL_LIBFUNCMOD_STATIC := $(LOCAL_LIBFUNCMOD_DIR)/build/libfuncmod.a
LOCAL_NANOGUI_BUILD_DIR := $(LOCAL_NANOGUI_DIR)/build

THIRD_PARTY_INCLUDE_DIRS := \
	$(HOST_DIR)/include \
	$(HOST_DIR)/include/LIB-remote \
	$(HOST_DIR)/include/LIB-funcmod \
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
		../LIB-funcMod/src

THIRD_PARTY_LIB_DIRS += ../nanogui_mod/build \
		  ../LIB-remote/build \
		  ../LIB-funcMod/build

FOUND_REMOTE_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/LIB-remote/remoteBadgeChannel.h) $(wildcard $(d)/libremote/remoteBadgeChannel.h) $(wildcard $(d)/remoteBadgeChannel.h)))
FOUND_FUNCMOD_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/LIB-funcmod/common/Path.hpp) $(wildcard $(d)/libfuncmod/common/Path.hpp) $(wildcard $(d)/common/Path.hpp)))
FOUND_NANOGUI_HEADER := $(firstword $(foreach d,$(THIRD_PARTY_INCLUDE_DIRS),$(wildcard $(d)/nanogui/nanogui.h)))

FOUND_REMOTE_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libremote.a)))
FOUND_FUNCMOD_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libfuncmod.so*) $(wildcard $(d)/libfuncmod.a)))
FOUND_NANOGUI_LIB := $(firstword $(foreach d,$(THIRD_PARTY_LIB_DIRS),$(wildcard $(d)/libnanogui.so*) $(wildcard $(d)/libnanogui.a)))

NEED_LOCAL_LIBREMOTE := $(if $(and $(FOUND_REMOTE_HEADER),$(FOUND_REMOTE_LIB)),0,1)
NEED_LOCAL_LIBFUNCMOD := $(if $(and $(FOUND_FUNCMOD_HEADER),$(FOUND_FUNCMOD_LIB)),0,1)
NEED_LOCAL_NANOGUI := $(if $(and $(FOUND_NANOGUI_HEADER),$(FOUND_NANOGUI_LIB)),0,1)

SRCS	:= 	$(wildcard src/*.cpp) \
			$(wildcard src/Controller/*.cpp) \
			$(wildcard src/Controller/Readers/*.cpp) \
			$(wildcard src/Peripherals/*.cpp) \
			$(wildcard src/GUI/*.cpp) \
			$(wildcard src/GUI/Buttons/*.cpp) \
			$(wildcard src/GUI/Pages/*.cpp)

OBJS	:= $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS	:= $(OBJS:.o=.d)
		
CXXFLAGS += -DNANOGUI_SHARED -DNVG_SHARED -DNANOGUI_USE_OPENGL

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
		-I$(HOST_DIR)/include/LIB-remote/

INCS += -I../nanogui_mod/include \
		-I../nanogui_mod/ext/nanovg \
		-I../nanogui_mod/ext/nanovg/src \
		-I../LIB-remote/src \
		-I../LIB-funcMod/src

ifeq ($(NEED_LOCAL_LIBREMOTE),1)
INCS += -I$(LOCAL_LIBREMOTE_DIR)/src
endif

ifeq ($(NEED_LOCAL_LIBFUNCMOD),1)
INCS += -I$(LOCAL_LIBFUNCMOD_DIR)/src
endif

ifeq ($(NEED_LOCAL_NANOGUI),1)
INCS += -I$(LOCAL_NANOGUI_DIR)/include \
		-I$(LOCAL_NANOGUI_DIR)/ext/nanovg/src
endif
	 	
LIBS	:= -lnanogui \
			-lremote \
			-lfuncmod

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L../nanogui_mod/build \
		-L../LIB-remote/build \
		-L../LIB-funcMod/build \

ifeq ($(NEED_LOCAL_LIBREMOTE),1)
LIBDIR += -L$(LOCAL_LIBREMOTE_DIR)/build
endif

ifeq ($(NEED_LOCAL_LIBFUNCMOD),1)
LIBDIR += -L$(LOCAL_LIBFUNCMOD_DIR)/build
endif

ifeq ($(NEED_LOCAL_NANOGUI),1)
LIBDIR += -L$(LOCAL_NANOGUI_BUILD_DIR)
endif

LDFLAGS += -Wl,-rpath,$(HOST_DIR)

BUILD ?= release

ifeq ($(BUILD),debug)
CXXFLAGS += -DDEBUG -O0 -g
endif

ifeq ($(BUILD),release)
CXXFLAGS += -O2
endif


CXXFLAGS += -DDEBUG -O0 -g
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
	@if [ -f $@ ] && [ "`cat $@`" != "$(CXX)" ]; then rm -rf $(OBJ_DIR) $(TARGET); fi
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
		cmake -S "$(LOCAL_NANOGUI_DIR)" -B "$(LOCAL_NANOGUI_BUILD_DIR)" -DCMAKE_BUILD_TYPE=$(if $(filter $(BUILD),debug),Debug,Release); \
		cmake --build "$(LOCAL_NANOGUI_BUILD_DIR)"; \
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
		$(LOCAL_NANOGUI_BUILD_DIR)
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(TARGET)

-include $(DEPS)
