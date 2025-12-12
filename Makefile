# Modular Makefile with ImGui Support

CXX := g++
STD := -std=c++17
CXXFLAGS := -Wall -Wextra -O3 $(STD) `sdl2-config --cflags`

# Important: We assume ImGui is located in ./vendor/imgui
# You must download Dear ImGui and put it there, or update this path.
IMGUI_DIR := vendor/imgui
IMGUI_BACKENDS := $(IMGUI_DIR)/backends

# Include directories
INC_DIRS := $(shell find . -type d -name include 2>/dev/null | sed 's|^./||')
INC_DIRS += $(IMGUI_DIR) $(IMGUI_BACKENDS)

CPPFLAGS := $(patsubst %,-I%,$(INC_DIRS))

# Libraries
LDLIBS := `sdl2-config --libs` -lGL -ldl

# Sources
# 1. Standard project sources
SRCS := $(shell find . -name '*.cpp' ! -path './build/*' ! -path './vendor/*' -print | sed 's|^./||')

# 2. ImGui sources (Core + Backends)
# Only add these if the directory exists to prevent find errors if you haven't added them yet
ifneq (,$(wildcard $(IMGUI_DIR)/imgui.cpp))
    IMGUI_SRCS := $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp \
                  $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp \
                  $(IMGUI_BACKENDS)/imgui_impl_sdl2.cpp $(IMGUI_BACKENDS)/imgui_impl_sdlrenderer2.cpp
    SRCS += $(IMGUI_SRCS)
endif

# Objects
OBJS := $(patsubst %.cpp,build/%.o,$(SRCS))

TARGET := sim

.PHONY: all clean show

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo Linking $@
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo Compiling $<
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

show:
	@echo "Includes: $(INC_DIRS)"
	@echo "Sources: $(SRCS)"

clean:
	@rm -rf build/ $(TARGET)