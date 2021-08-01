# CPP compiler
CPP=clang++

# CPP version
CPP_VERSION=-std=c++14

# Compiler flags \
-g -> debug symbols. Should not be used in release
CPP_FLAGS=-g

# Optimization levels \
-O0 -> None \
-O1 -> Moderate \
-O2 -> Full \
-O3 -> Maximum
OPTIMIZATION_LEVEL=-O0

# Warnings that the compiler should throw
WARNING_FLAGS=-Wall -Wextra -Wpedantic

CPP_FLAGS += $(OPTIMIZATION_LEVEL) 
CPP_FLAGS += $(WARNING_FLAGS)

# Static libraries that must be linked at compile time
LINKER=

# Optional flags, can use to enable print debugs instructions \
Use: make <target> "OPTIONAL_FLAGS= <flag-1> <flag-n>" \
Example to print debug  \
make "OPTIONAL_FLAGS=-DDEBUG"
OPTIONAL_FLAGS=

# Where the declarations are
HEADERS_DIR=./include

# Where the source code are
SOURCES_DIR=./src

# Where compiled objects should go
BUILD_DIR=./build

# Gets all the source codes
SRCS=$(wildcard $(SOURCES_DIR)/*.cpp)

APP_NAME=termsequel.out

OBJS=$(patsubst $(SOURCES_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

all: $(APP_NAME)

# Create the executable.
$(APP_NAME): termsequel.cpp $(OBJS)
	@$(CPP) $(CPP_FLAGS) $(CPP_VERSION) $(OPTIONAL_FLAGS) -o $@ $^

# Compile all the sources
$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(HEADERS_DIR)/%.hpp
	@$(CPP) $(CPP_FLAGS) $(CPP_VERSION) $(OPTIONAL_FLAGS) -I $(HEADERS_DIR) $(LINKER) -c -o $@ $<
