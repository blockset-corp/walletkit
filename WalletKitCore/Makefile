#
# Define OS_BRD and CPU_BRD
#
ifeq ($(OS),Windows_NT)
    OS_BRD := "Windows"
else
    OS_BRD  := $(shell uname -s)
    CPU_BRD := $(shell uname -p)
endif

#
# Set CFLAGS_{OS,CPU}_BRD
#
CFLAGS_OS_BRD  :=
CGLAGS_CPU_BRD :=
ifeq "$(OS_BRD)" "Darwin"
    CFLAGS_OS_BRD := -framework Security $(CFLAGS_OS_BRD)
endif

#
# Targets
#

# Bitcoin Tests
test:	clean
	cc -o $@ -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1    \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c  ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c

run:	test
	./test

# cmake (libcorecrypto.dylib, etc)
cmake:	FORCE
	(mkdir cmake; cd cmake; cmake ../; make clean; make)

cmake_gcc: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-gcc; mkdir cmake-gcc; cd cmake-gcc; cmake ../; make clean; make VERBOSE=1)	

clean:
	rm -f *.o */*.o */*/*.o test

FORCE:
