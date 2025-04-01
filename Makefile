CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
LDFLAGS = -lwebsockets

SRC_DIR_SERVER = src
SRC_DIR_CLIENT = src-client
BUILD_DIR = build
INCLUDE_DIR = include

SRCS_SERVER = $(wildcard $(SRC_DIR_SERVER)/*.cpp)
SRCS_CLIENT = $(wildcard $(SRC_DIR_CLIENT)/*.cpp)

OBJS_SERVER = $(patsubst $(SRC_DIR_SERVER)/%.cpp,$(BUILD_DIR)/server_%.o,$(SRCS_SERVER))
OBJS_CLIENT = $(patsubst $(SRC_DIR_CLIENT)/%.cpp,$(BUILD_DIR)/client_%.o,$(SRCS_CLIENT))

TARGET_SERVER = chat_server
TARGET_CLIENT = chat_client

.PHONY: all clean run_server run_client

all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Compilar objetos para chat_server
$(BUILD_DIR)/server_%.o: $(SRC_DIR_SERVER)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilar objetos para chat_client
$(BUILD_DIR)/client_%.o: $(SRC_DIR_CLIENT)/%.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_SERVER): $(OBJS_SERVER)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_CLIENT): $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

run_server: $(TARGET_SERVER)
	./$(TARGET_SERVER)

run_client: $(TARGET_CLIENT)
	./$(TARGET_CLIENT)

clean:
	rm -rf $(BUILD_DIR) $(TARGET_SERVER) $(TARGET_CLIENT)

