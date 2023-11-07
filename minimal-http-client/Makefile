
SILENT = 1
.PHONY: all clean 

	ifeq(uname, Linux)
	LDFLAGS += -lrt 
	CFLAGS += -D_GNU_SOURCE 
	CC = gcc 
	endif 

	ifeq(uname, Darwin)
	LDFLAGS += -framework CoreFoundation -framework Security -framework CoreServices
	CC = clang-16 
	endif

	ifeq(uname, _WIN32) 
	LDFLAGS += -lws2_32 -ladvapi32 -lcrypt32 -luser32 -lkernel32 -lntdll -ladvapi32 -luser32 -lkernel32 -lntdll -lole32 
	endif


CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g -O2 
LDFLAGS = -L/usr/local/lib -lcrypto -lssl -lz -lm -lpthread -ldl `pkg-config --cflags --libs libwebsockets`

	
all: 
	@gcc $(CFLAGS) $(LDFLAGS) minimal-http-client.c -o lws 
	@echo "Build complete."

clean: 
	@rm -rf lws *.o *.exe *.dSYM 
	@echo "Clean complete."
