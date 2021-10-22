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

LIBS_OS_BRD := -lpthread -lm
ifeq "$(OS_BRD)" "Linux"
    LIBS_OS_BRD := $(LIBS_OS_BRD) -lbsd
endif

LFLAGS_OS_BRD  := -L./cmake
ifeq "$(OS_BRD)" "Darwin"
    LFLAGS_OS_BRD := $(LFLAGS_OS_BRD) -rpath @executable_path/cmake
else
    LFLAGS_OS_BRD := $(LFLAGS_OS_BRD) -Wl,-rpath,`pwd`/cmake
endif

#
# Targets
#

# Bitcoin Tests
test:	clean
	cc -o $@ -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1    \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c ./src/litecoin/*.c ./src/dogecoin/*.c ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c $(LIBS_OS_BRD)

run:	test
	./test

# cmake (libcorecrypto.dylib, etc)
cmake:	FORCE
	(mkdir cmake; cd cmake; cmake ../; make clean; make)

cmake_test: cmake
	cc -o test -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1      \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) $(LFLAGS_OS_BRD) \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c \
		-lcorecrypto $(LIBS_OS_BRD)

#######################################################################################
#  Targets for GNU/Linux
#
# Traditionally WalletKit has been built using Darwin/Clang
#
# cmake_gnu_linux:           build walletkit and test.c
#                               Output: cmake-linux/test_bitcoin
#
# cmake_gnu_linux_verbose:   same as cmake_linux_gnu but with verbose makefile
#
# cmake_gnu_linux_tests:     cmake_linux_gnu + WalletKitCoreTests with test application
#                               Output: cmake-linux-tests/test_bitcoin
#                                       cmake-linux-tests/WalletKitCoreTests
#
# test_gnu_linux:            builds test.c based application w/o building Walletkit libs (similar to 'test'
#                            but for GNU/Linux
########################################################################################
cmake_gnu_linux_verbose: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-gcc; mkdir cmake-gcc; cd cmake-gcc; cmake ../; make clean; make VERBOSE=1)	

# Make libcorecrypto.so on Linux with gcc
cmake_gnu_linux: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-linux; mkdir cmake-linux; cd cmake-linux; cmake ../; make clean; make)	

# Make libcorecrypto.so, libcorecryptotests.so & WalletKitCoreTests which is an application that runs
# equivalent of WalletKitCoreTests.swift and links with the 2 shared libs
cmake_gnu_linux_tests: FORCE
	(export CC=gcc; export CXX=g++; rm -rf cmake-linux-tests; mkdir cmake-linux-tests; cd cmake-linux-tests; cmake -DCMAKE_BUILD_TYPE=Debug ../; make clean; make)	


# Example: build just the bitcoin tests directly on Linux w/o building shared libs. Similar to 'test' target
#          but structured for linux build
test_gnu_linux:	clean
	cc -o $@ -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1    \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c  ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c \
		-pthread -ldl -lbsd -lm

#################################### E/O Linux/GNU #####################################		
clean:
	rm -f *.o */*.o */*/*.o test

FORCE:
