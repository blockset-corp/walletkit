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

# Make libcorecrypto.so on Linux with gcc
cmake_linux: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-linux; mkdir cmake-linux; cd cmake-linux; cmake ../; make clean; make)	

# Make libcorecrypto.so, libcorecryptotests.so & WalletKitCoreTests which is an application that runs
# equivalent of WalletKitCoreTests.swift and links with the 2 shared libs
cmake_linux_tests: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-linux-tests; mkdir cmake-linux-tests; cd cmake-linux-tests; cmake -DCMAKE_BUILD_TYPE=Debug ../; make clean; make)	


# Example: build just the bitcoin tests directly on Linux w/o building shared libs. Similar to 'test' target
#          but structured for linux build
test_linux:	clean
	cc -o $@ -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1    \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c  ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c \
		-pthread -ldl -lbsd -lm
clean:
	rm -f *.o */*.o */*/*.o test

FORCE:
