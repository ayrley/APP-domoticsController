NAME := control
VERSION := 0.1
DISTNAME := $(NAME)-$(VERSION)
 
SRCS 	:=	src/control.cpp \
			src/network.cpp \
			src/settings.cpp \
			src/accessController.cpp \
			src/reader.cpp \
			src/badge.cpp \
			src/io.cpp
		
INCS 	:= 	-I. \
		-Isrc
	 	
LIBS	:= 	

LIBDIR	:= 	-L$(HOST_DIR)/usr/lib \
		-L$(TARGET_DIR)/usr/lib
$(NAME):
	$(CXX) $(CXXFLAGS) -c -fPIC $(INCS) $(SRCS) 
	$(CXX) -o $(NAME) *.o $(LIBDIR) $(LIBS)
	
clean:
	rm -f $(NAME) *.o
	
$(NAME)-linter:
	clang-tidy $(SRCS) -- $(INCS) > clang-tidy-output.txt 2>&1

dist: Makefile $(NAME) 

