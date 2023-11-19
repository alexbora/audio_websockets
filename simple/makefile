.SILENT:1

MACHINE_NAME := $(shell hostname)
UNAME_S := $(shell uname -s)

CFLAGS:=-O2 -g -Wall -std=c11
LIBS:=$(shell pkg-config --libs OpenSSL) $(shell pkg-config --libs libcurl) -lpthread
INCLUDES:= $(shell pkg-config --cflags OpenSSL) $(shell pkg-config --cflags libcurl) -lpthread
CC:=

SRC := ssl_read.c  # Replace with your actual source files
TARGET :=   # Replace with your actual target executable
SUFFIX:= 

ifeq ($(UNAME_S),Windows_NT)
	SUFFIX+=.exe 
endif

ifeq ($(UNAME_S),Linux)
    # Linux-specific settings
    CC=gcc-12x
		LIBS=-lssl -lcrypto -lcurl -lpthread
    TARGET=ssl_read
else ifeq ($(UNAME_S),Darwin)
    # macOS-specific settings
    CC=gcc-13
    TARGET=ssl_read
else ifeq ($(UNAME_S),Windows_NT)
    # Windows-specific settings
    LIBS += -lws2_32
    CC=gcc
    TARGET=ssl_read
else
    $(error Unsupported operating system: $(UNAME_S))
endif

.PHONY: all release debug config clean

all: config
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRC) -o $(TARGET)$(SUFFIX) 

release: CFLAGS += -O3
release: 
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRC) -o $(TARGET)_release$(SUFFIX) 

debug: CFLAGS += -O0 -Wall -Wextra -g
debug: 
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRC) -o $(TARGET)_debug$(SUFFIX)

config:
	@echo "#ifndef CONFIG_H" > config.h
	@echo "#define CONFIG_H" >> config.h
	@if [ "$(shell echo $(MACHINE_NAME) | grep -q '^RO' && echo yes)" = "yes" ]; then \
        echo "#define USE_PROXY (1)" >> config.h; \
    else \
    	echo "#define USE_PROXY (0)" >> config.h; \
    fi
	@echo "#endif" >> config.h

clean:
	$(RM) -rf $(TARGET)$(SUFFIX) $(TARGET)_release$(SUFFIX) $(TARGET)_debug$(SUFFIX) *.o *~ *.dSYM


