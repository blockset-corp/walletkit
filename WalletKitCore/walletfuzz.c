#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "BRCryptoCoder.h"

#define SIZE 50

// see https://github.com/ethereum/solidity/tree/develop/test/tools
int main(int argc, const char *argv[])
{
    char input[SIZE] = {0};

	ssize_t length;
	length = read(STDIN_FILENO, input, SIZE);

    BRCryptoCoder coder = cryptoCoderCreate(CRYPTO_CODER_BASE58);
    cryptoCoderDecodeLength(coder, input);
    cryptoCoderGive(coder);

    return 0;
}
