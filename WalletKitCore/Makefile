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


all: run

#######################################################################################
#  Targets for cmake 
#
# Traditionally WalletKit has been built using Darwin/Clang
#
# libs:                 Build WalletKitCore lib
#
# tests:                Build required library and link with tests 
#                         (test_bitcoin, WalletKitCoreTests)
#
########################################################################################

# Make libcorecrypto.so on Linux with gcc
libs: FORCE
	(rm -rf libs; mkdir libs; cd libs; cmake ../; make clean; make)	

# Make libcorecrypto.so, libcorecryptotests.so, WalletKitCoreTests & test_bitcoin
# which is an application that runs equivalent of WalletKitCoreTests.swift and links 
# with the 2 shared libs
tests: FORCE
	(rm -rf tests; mkdir tests; cd tests; cmake -DCMAKE_BUILD_TYPE=Debug ../; make clean; make)	


#################################### E/O Linux/GNU #####################################		
clean:
	rm -f *.o */*.o */*/*.o test

FORCE:
