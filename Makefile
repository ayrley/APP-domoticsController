NAME := control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)

NANOGUI_ROOT := /home/ayrley/priv/nanogui
 
SRCS	:= $(wildcard src/*.cpp) $(wildcard src/GUI/*.cpp)
		
CXXFLAGS += -DNANOGUI_USE_OPENGL -DNANOGUI_SHARED -DNVG_SHARED

INCS 	:= 	-I. \
		-Isrc \
		-Isrc/GUI \
		-I$(NANOGUI_ROOT)/include \
		-I$(NANOGUI_ROOT)/include/nanovg \
		-I$(NANOGUI_ROOT)/ext/nanovg/src
	 	
LIBS	:= -lnanogui

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib \
		-L$(NANOGUI_ROOT)

LDFLAGS += -Wl,-rpath,$(NANOGUI_ROOT)
$(NAME):
	$(CXX) $(CXXFLAGS) -c -fPIC $(INCS) $(SRCS) 
	$(CXX) -o $(NAME) *.o $(LDFLAGS) $(LIBDIR) $(LIBS)
	rm -f *.o
	
clean:
	rm -f $(NAME) *.o
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(NAME) 