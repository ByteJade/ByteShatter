BUILD_DIR = build
LIB_BUILD_DIR = build/lib
SRC_DIR = src
LIB_DIR = custom

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CXX = g++
CC = gcc
CFLAGS = -Wall -O2
AS = as

TARGET = shatter

all: $(BUILD_DIR) $(LIB_BUILD_DIR) \
    $(LIB_BUILD_DIR)/my_libc.so.6 \
    $(LIB_BUILD_DIR)/my_libm.so.6 \
    $(LIB_BUILD_DIR)/my_libX11.so.6 \
    $(LIB_BUILD_DIR)/my_libGL.so.1 \
    $(LIB_BUILD_DIR)/my_libSDL-2.0.so.0 \
    $(LIB_BUILD_DIR)/my_libSDL_ttf-2.0.so.0 \
    $(LIB_BUILD_DIR)/my_libpng16.so.16 \
    $(LIB_BUILD_DIR)/my_libjpeg.so.8 \
    $(LIB_BUILD_DIR)/my_libvorbisfile.so.3 \
    $(LIB_BUILD_DIR)/my_libstdc++.so.6 \
	$(TARGET)

$(BUILD_DIR):
	mkdir -p $@

$(LIB_BUILD_DIR):
	mkdir -p $@

$(TARGET): $(OBJS)
	$(CC) -o $(BUILD_DIR)/$@ $^

$(LIB_BUILD_DIR)/my_libc.o: $(LIB_DIR)/my_libc.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libc.so.6: $(LIB_BUILD_DIR)/my_libc.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@

$(LIB_BUILD_DIR)/my_libm.o: $(LIB_DIR)/my_libm.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libm.so.6: $(LIB_BUILD_DIR)/my_libm.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lm

$(LIB_BUILD_DIR)/my_libX11.o: $(LIB_DIR)/my_libX11.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libX11.so.6: $(LIB_BUILD_DIR)/my_libX11.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lX11

$(LIB_BUILD_DIR)/my_libGL.o: $(LIB_DIR)/my_libGL.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libGL.so.1: $(LIB_BUILD_DIR)/my_libGL.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lGL

$(LIB_BUILD_DIR)/my_libSDL-2.0.o: $(LIB_DIR)/my_libSDL2.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libSDL-2.0.so.0: $(LIB_BUILD_DIR)/my_libSDL-2.0.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lSDL2

$(LIB_BUILD_DIR)/my_libSDL_ttf-2.0.o: $(LIB_DIR)/my_libSDL2_ttf.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libSDL_ttf-2.0.so.0: $(LIB_BUILD_DIR)/my_libSDL_ttf-2.0.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lSDL2_ttf

$(LIB_BUILD_DIR)/my_libpng16.o: $(LIB_DIR)/my_libpng16.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libpng16.so.16: $(LIB_BUILD_DIR)/my_libpng16.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lpng

$(LIB_BUILD_DIR)/my_libjpeg.o: $(LIB_DIR)/my_libjpeg.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libjpeg.so.8: $(LIB_BUILD_DIR)/my_libjpeg.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -ljpeg

$(LIB_BUILD_DIR)/my_libvorbisfile.o: $(LIB_DIR)/my_libvorbisfile.s | $(LIB_BUILD_DIR)
	$(AS) -I$(LIB_DIR) $< -o $@
$(LIB_BUILD_DIR)/my_libvorbisfile.so.3: $(LIB_BUILD_DIR)/my_libvorbisfile.o | $(LIB_BUILD_DIR)
	$(CC) -fPIC -shared $< -o $@ -lvorbisfile

$(LIB_BUILD_DIR)/my_libstdc++.so.6: $(LIB_DIR)/my_libstdc++.cpp | $(LIB_BUILD_DIR)
	$(CXX) -fPIC -shared $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

.PHONY: all clean run