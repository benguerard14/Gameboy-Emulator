CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lSDL2
SRC = $(shell find . -name "*.c")
OBJ = $(SRC:.c=.o)
BUILD_DIR = build
OBJ_BUILD = $(addprefix $(BUILD_DIR)/, $(SRC:.c=.o))
OUT = GBemulator

all: $(OUT)

$(OUT): $(OBJ_BUILD)
	$(CC) $(OBJ_BUILD) -o $(OUT) $(LIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(OUT)

clean:
	rm -f $(OUT)
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
