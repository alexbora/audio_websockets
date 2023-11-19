OS := $(shell uname)

# ifeq ($(OS),Windows_NT)
# 		CFLAGS += -D WIN32
# 		LIBS += -lws2_32
# else ifeq ($(OS),Linux)
# 		UNAME_S := $(shell uname -s)
# 		ifeq ($(UNAME_S),Linux)
# 				CFLAGS += -D LINUX
# 				LIBS += -lssl -lcrypto
# endif

ifeq ($(UNAME_S),Darwin)
				CFLAGS += -D OSX
				LIBS += -lssl -lcrypto
endif



ifeq ($(OS),Windows_NT)
		TARGET = ssl_read.exe
else ifeq ($(OS),Linux)
		TARGET = ssl_read
endif

ifeq ($(OS),Windows_NT)
		SRC = ssl_read.c
else ifeq ($(OS),Linux)
		SRC = ssl_read.c
endif

ifeq ($(OS),Windows_NT)
		CC = gcc
else
		CC = gcc-13
endif 


.PHONY: all
all: 
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRC) -o $(TARGET) 

clean:
	$(RM) -rf $(TARGET) *.o *~ *.dSYM


