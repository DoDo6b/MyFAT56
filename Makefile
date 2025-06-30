# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
BUILD_DIR = build
TARGET = FileManager

# Исходные файлы
SRCS = main.c commands.c
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Цель по умолчанию
all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Сборка .o файлов
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
