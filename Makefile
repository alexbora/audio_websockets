
ifeq ($(shell uname),Linux)
    OS = Linux
else ifeq ($(shell uname),Darwin)
    OS = macOS
else ifeq ($(shell uname),Windows_NT)
    OS = Windows
else
    OS = Unknown 
endif


.SILENT: 
# LIBS=`pkg-config --libs --cflags libmpg123` `pkg-config --libs --cflags ao` `pkg-config --cflags --libs libcurl`

ifeq ($(OS),Unknown) 
	echo "Unknown OS" 
	abort
endif

ifeq ($(OS),Darwin)
CC:=/opt/local/libexec/llvm-16/bin/clang
LIBS:+=-framework CoreAudio -framework AudioToolbox -framework CoreFoundation

ifeq($(OS),Linux) 
CC:=/usr/bin/gcc

ifeq($(OS),Windows_NT) 
CC=gcc 
endif 

LIBS=`pkg-config --libs --cflags libmpg123` `pkg-config --libs --cflags ao` `pkg-config --cflags --libs libcurl`
CFLAGS:=-w -Ofast -fomit-frame-pointer -ffast-math -fno-plt -funroll-loops -finline-functions -ftree-vectorize -funsafe-math-optimizations -funsafe-loop-optimizations -fno-stack-protector -fno-stack-check -funswitch-loops -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-strict-aliasing -fno-strict-overflow -fno-delete-null-pointer-checks -fno-merge-all-constants -foptimize-sibling-calls -fno-common -fno-ident -m64 -fno-rtti -fno-exceptions


#  -lm 
#-ldl -lc 

.PHONY: all clea
all: 
	@$(CC)	$(CFLAGS) $(LIBS) test3.c index.c -o test3  -DNOMAIN `pkg-config --libs --cflags libmpg123` `pkg-config --libs --cflags ao` `pkg-config --cflags --libs libcurl` 
	# mkdir -p bin 
	# cd bin 
	# mv test3 bin
	@echo "compiled"

clean: 
	# @rm -f [test1 test2 orgis orgis.exe test1.exe test2.exe]
	 # if [ -f test3 ]; then rm test2; fi
	for file in test3; do if [ -f $$file ]; then rm $$file; fi; done
	for file in index; do if [ -f $$file ]; then rm $$file; fi; done
	# rm -f [bin/test1 bin/test2 bin/orgis bin/orgis.exe bin/test1.exe bin/test2.exe]
	# if [ -f bin/test1.exe ]; then rm bin/test1.exe; fi
	@echo "cleaned"
	
	
