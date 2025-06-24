CC := clang++
CFLAGS := -std=c++20 -Wall -Wextra -Werror
INCFLAGS := -I/opt/homebrew/opt/raylib/include
LFLAGS := -L/opt/homebrew/opt/raylib/lib -lraylib \
		  -framework OpenGL -framework IOKit -framework Cocoa

ifeq ($(MODE),debug)
	CFLAGS += -O0 -g
	BUILD_DIR := build/debug
else
	CFLAGS += -O2 -flto
	BUILD_DIR := build/release
endif

NAME := main
BINARY := $(BUILD_DIR)/$(NAME)
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

$(BINARY): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCFLAGS) $(OBJECTS) -o $@ $(LFLAGS)
	cp $@ .

$(BUILD_DIR)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(INCFLAGS) -MMD -MP -o $@ $<

-include $(OBJECTS:.o=.d)

run: $(BINARY)
	./$(NAME)

clean:
	rm -rf build

.PHONY: run clean
