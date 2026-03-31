NAME := control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)

NANOGUI_ROOT := /home/ayrley/priv/nanogui

BUILD_DIR := build-make
OBJ_DIR := $(BUILD_DIR)/obj
TARGET := $(BUILD_DIR)/$(NAME)
 
SRCS	:= 	$(wildcard src/*.cpp) \
			$(wildcard src/Controller/*.cpp) \
			$(wildcard src/Peripherals/*.cpp) \
			$(wildcard src/GUI/*.cpp) \
			$(wildcard src/GUI/Buttons/*.cpp) \
			$(wildcard src/GUI/Pages/*.cpp)

OBJS	:= $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS	:= $(OBJS:.o=.d)
		
CXXFLAGS += -DNANOGUI_USE_OPENGL -DNANOGUI_SHARED -DNVG_SHARED

INCS 	:= 	-I. \
		-Isrc \
		-Isrc/Controller \
		-Isrc/Peripherals \
		-Isrc/GUI \
		-Isrc/GUI/Buttons \
		-Isrc/GUI/Pages \
		-I$(NANOGUI_ROOT)/include \
		-I$(NANOGUI_ROOT)/include/nanovg \
		-I$(NANOGUI_ROOT)/ext/nanovg/src
	 	
LIBS	:= -lnanogui

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L$(NANOGUI_ROOT)

LDFLAGS += -Wl,-rpath,$(NANOGUI_ROOT)

.PHONY: all clean $(NAME)-linter

all: $(TARGET)

$(NAME): $(TARGET)
	@echo "Built $(TARGET)"

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBDIR) $(LIBS)
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