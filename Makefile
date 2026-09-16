CC := gcc
CFLAGS := -O2 -Wall -Wextra -Wpedantic
CPPFLAGS := -Isrc
LDLIBS_SERVER := -pthread
SRC_DIR := src
BUILD_DIR := build
SERVER := server
CLIENT := client
SERVER_SRCS := $(SRC_DIR)/server.c $(SRC_DIR)/monitor.c $(SRC_DIR)/protocol.c $(SRC_DIR)/util.c
CLIENT_SRCS := $(SRC_DIR)/client.c $(SRC_DIR)/protocol.c $(SRC_DIR)/util.c
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CLIENT_SRCS))
.PHONY: all clean run-server run-client debug asan tsan
all: $(SERVER) $(CLIENT)
$(SERVER): $(SERVER_OBJS)
	$(CC) -o $@ $^ $(LDLIBS_SERVER)
$(CLIENT): $(CLIENT_OBJS)
	$(CC) -o $@ $^
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
run-server: $(SERVER)
	./$(SERVER) 127.0.0.1 9000
run-client: $(CLIENT)
	./$(CLIENT) 127.0.0.1 9000
debug: CFLAGS := -O0 -g3 -Wall -Wextra -Wpedantic
debug: clean all
asan: CFLAGS := -O1 -g3 -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer
asan: LDFLAGS := -fsanitize=address,undefined
asan: clean all
tsan: CFLAGS := -O1 -g3 -Wall -Wextra -Wpedantic -fsanitize=thread -fno-omit-frame-pointer
tsan: LDFLAGS := -fsanitize=thread
tsan: clean all
clean:
	rm -rf $(BUILD_DIR) $(SERVER) $(CLIENT)
