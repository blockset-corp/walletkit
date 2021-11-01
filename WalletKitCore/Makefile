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
    LIBS_OS_BRD := $(LIBS_OS_BRD) -lbsd -ldl
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

help:
	@echo
	@echo "WalletKit Build Targets"
	@echo
	@echo "    btc-test:                 Builds Bitcoin 'test.c' directly with all required WalletKit sources"
	@echo    
	@echo "    btc-test-run:             Runs Bitcoin tests"
	@echo    
	@echo "    wallet-kit-libs:          Builds the WalletKit shared library 'WalletKitCore'"
	@echo
	@echo "    wallet-kit-libs-verbose:  Builds as wallet-kit-libs but with a verbose makefile"  
	@echo    
	@echo "    wallet-kit-test:          Builds the WalletKit test applications, linked with 'WalletKitCore' shared library"
	@echo    
	@echo "    wallet-kit-test-run:      Builds and runs WalletKit test applications"
	@echo    
	@echo "    clean:                    Removes the build folder and all build files"
	@echo    
	@echo "    For 'wallet-kit-*' targets, cmake is a prerequisite. Deliverables go to 'build' folder."
	@echo "    The first invokation of a 'wallet-kit-*' target will both execute cmake and do a build."
	@echo "    Thereafter, incremental builds will be done until the next 'clean'."
	@echo    
	@echo "    This Makefile has been tested using make version 4.2.1 and cmake version 3.16.3"
	@echo "    Both Darwin/Clang and GNU/Linux based builds are supported."
	@echo "    The default toolchain selection by cmake can be overcome by setting environment 'CC'"
	@echo "    variable to the custom selection e.g."
	@echo
	@echo "	     $$ export CC=clang;make wallet-kit-libs-verbose"
	@echo
	@echo "    will force the native toolchain to 'clang'"

# Bitcoin Tests
btc-test:	clean
	cc -o $@ -I./include -I./src               \
		-I./vendor -I./vendor/secp256k1    \
		$(CFLAGS_OS_BRD) $(CFLAGS_CPU_BRD) \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c ./src/litecoin/*.c ./src/dogecoin/*.c ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c $(LIBS_OS_BRD)

btc-test-run:	btc-test
		./btc-test



#######################################################################################
#  Targets for cmake 
#
# WalletKit can be built under Darwin/Clang or alternatively with GNU/Linux.
#
# libs:                 Build WalletKitCore lib
#
# tests:                Build required library and link with tests 
#                         (test_bitcoin, WalletKitCoreTests)
#
########################################################################################

# Test for DEBUG nature of the build requires inspecting
# the prior `cmake` CMAKE_BUILD_TYPE, presuming cmake has
# been run. Only `Debug` results in the production of
# tests and required test libraries, so in some cases 
# force remake with appropriate flags.
cmake-walletkit-debug:
	@if [ ! -d "build" ]; then \
         mkdir build; \
         cd build; \
         cmake -DCMAKE_BUILD_TYPE=Debug ..; \
     else \
         cur_dbg_setting=($$(cat build/CMakeCache.txt | grep CMAKE_BUILD_TYPE | tr "=" " ")); \
         if [ -z "$${cur_dbg_setting[1]}" ]; then \
             echo "Re-make for debug (tests)"; \
             rm -rf build; \
             mkdir build; \
             cd build; \
             cmake -DCMAKE_BUILD_TYPE=Debug ..; \
         fi; \
     fi;
 

cmake-walletkit:
	@if [ ! -d "build" ]; then mkdir build; cd build; cmake ..; fi

cmake-walletkit-verbose:
	@if [ ! -d "build" ]; then mkdir build; cd build; cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON ..; fi

cmake-installed:
	@[ -f `which cmake` ] || { echo "cmake is required for WalletKit make"; exit 1; }

wallet-kit-libs: cmake-installed cmake-walletkit FORCE
	@make -C build 

wallet-kit-libs-verbose: cmake-installed cmake-walletkit-verbose FORCE
	@make -C build 

wallet-kit-test: cmake-installed cmake-walletkit-debug FORCE
	@make -C build

wallet-kit-test-run: wallet-kit-test FORCE
	build/WalletKitCoreTests
	build/bitcoin_test

#################################### E/O Linux/GNU #####################################		
clean:
	@rm -f *.o */*.o */*/*.o test
	@rm -rf build

FORCE:
