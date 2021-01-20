
test:	clean
	cc -I./src -I./src/support -I./vendor -I./vendor/secp256k1 -I./include -I./WalletKitCoreTests/test/include -framework Security -o $@ src/bitcoin/*.c src/bcash/*.c src/bsv/*.c src/support/event/*.c src/support/*.c vendor/sqlite3/sqlite3.c WalletKitCoreTests/test/bitcoin/test.c

clean:
	rm -f *.o */*.o test

run:	test
	./test
