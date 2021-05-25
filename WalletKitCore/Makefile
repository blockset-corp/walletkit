
test:	clean
	cc -I./include -I./src -I./src/support -I./vendor -I./vendor/secp256k1 -o $@ \
		./src/bitcoin/*.c ./src/bcash/*.c ./src/bsv/*.c ./src/litecoin/*.c ./src/dogecoin/*.c ./src/support/event/*.c ./src/support/*.c ./vendor/sqlite3/sqlite3.c \
		-framework Security \
		-IWalletKitCoreTests/test/include WalletKitCoreTests/test/bitcoin/test.c

clean:
	rm -f *.o */*.o */*/*.o test

run:	test
	./test
