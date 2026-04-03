NAME := control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)

ifdef CROSS_COMPILE
CXX := $(CROSS_COMPILE)g++
endif

BUILD_DIR := build-make
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/$(NAME)
COMPILER_STAMP := $(BUILD_DIR)/.compiler
 
SRCS	:= 	$(wildcard src/*.cpp) \
			$(wildcard src/Controller/*.cpp) \
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
		-I$(HOST_DIR)/include/nanogui/ext/nanovg/src
	 	
LIBS	:= -lnanogui

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L$(HOST_DIR)

LDFLAGS += -Wl,-rpath,$(HOST_DIR)

BUILD ?= release

ifeq ($(BUILD),debug)
CXXFLAGS += -DDEBUG -O0 -g
endif

ifeq ($(BUILD),release)
CXXFLAGS += -O2
endif

.PHONY: all clean $(NAME)-linter debug release prepare-build

all: $(TARGET)

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

$(NAME): $(TARGET)
	@echo "Built $(TARGET)"


prepare-build: $(COMPILER_STAMP)

$(COMPILER_STAMP): | $(BUILD_DIR)
	@if [ -f $@ ] && [ "`cat $@`" != "$(CXX)" ]; then rm -rf $(OBJ_DIR) $(TARGET); fi
	@printf '%s\n' '$(CXX)' > $@

$(TARGET): prepare-build $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBDIR) $(LIBS)
	ln -sf $(TARGET) $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC $(INCS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)
	
clean:
	rm -rf $(BUILD_DIR)
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(TARGET)

-include $(DEPS)
