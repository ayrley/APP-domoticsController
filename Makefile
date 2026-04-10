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
		-I$(LOCAL_LIBREMOTE_DIR)/src \
		-I$(LOCAL_LIBFUNCMOD_DIR)/src \
		-I$(LOCAL_NANOGUI_DIR)/include \
		-I$(LOCAL_NANOGUI_DIR)/ext/nanovg/src \
		-I$(HOST_DIR)/include \
		-I$(HOST_DIR)/include/nanovg \
		-I$(HOST_DIR)/include/nanogui/ext/nanovg/src \
		-I$(HOST_DIR)/include/LIB-funcmod/ \
		-I$(HOST_DIR)/include/LIB-remote/
	 	
LIBS	:= -lnanogui \
			-lremote \
			-lfuncmod

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L$(LOCAL_LIBREMOTE_DIR)/build \
		-L$(LOCAL_LIBFUNCMOD_DIR)/build \
		-L$(LOCAL_NANOGUI_BUILD_DIR) \

LDFLAGS += -Wl,-rpath,$(HOST_DIR)

BUILD ?= release

ifeq ($(BUILD),debug)
CXXFLAGS += -DDEBUG -O0 -g
endif

ifeq ($(BUILD),release)
CXXFLAGS += -O2
endif

ifeq ($(LOCAL),local)
INCS += -I../nanogui_mod/include \
		-I../nanogui_mod/ext/nanovg \
		-I../nanogui_mod/ext/nanovg/src \
		-I../LIB-remote/src \
		-I../LIB-funcMod/src

LIBDIR += -L../nanogui_mod/build \
		  -L../LIB-remote/build \
		  -L../LIB-funcMod/build

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
	@if [ ! -f "$(LOCAL_LIBREMOTE_STATIC)" ]; then \
		echo "[deps] Building local libremote"; \
		$(MAKE) -C "$(LOCAL_LIBREMOTE_DIR)" BUILD=$(BUILD) CROSS_COMPILE=$(CROSS_COMPILE) static; \
	fi
	@if [ ! -f "$(LOCAL_LIBFUNCMOD_STATIC)" ]; then \
		echo "[deps] Building local libfuncmod"; \
		$(MAKE) -C "$(LOCAL_LIBFUNCMOD_DIR)" BUILD=$(BUILD) CROSS_COMPILE=$(CROSS_COMPILE) static; \
	fi
	@if ! ls "$(LOCAL_NANOGUI_BUILD_DIR)"/libnanogui* >/dev/null 2>&1; then \
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
	rm -rf $(BUILD_DIR) $(NAME)
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(TARGET)

-include $(DEPS)
