# Define the compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra $(shell sdl2-config --cflags)

BUILD_DIR = build

# Define the source and object file variables

HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

# Define the output executable name
EXEC = $(BUILD_DIR)/vulkan-engine

.phony: all clean format

# Build rules
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $^ -o $@ -lvulkan $(shell sdl2-config --libs)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

format:
	clang-format -i --style=file $(SOURCES) $(HEADERS)
