#! /bin/sh

BASE_DIR=$1

if [ "" == "${BASE_DIR}" ]; then
    BASE_DIR="`pwd`"
fi

# Source the template env file to get WALLET_KIT_TESTS_CONFIG_FILE
source "${BASE_DIR}/templates/WalletKitTestsConfig.env"

# Source the User's env file, assuming they created one with their WALLET_KIT_TESTS_CONFIG_FILE.
if [ -f "${BASE_DIR}/WalletKitTestsConfig.env" ]; then
    source "${BASE_DIR}/WalletKitTestsConfig.env"
fi

# Check for the existence of a WALLET_KIT_TESTS_CONFIG_FILE environment variable
if [ "" == "${WALLET_KIT_TESTS_CONFIG_FILE}" ]; then
    echo "Configuration error - no ENV_VAR named WALLET_KIT_TESTS_CONFIG_FILE"
    exit 0
fi

SRC_FILE="${WALLET_KIT_TESTS_CONFIG_FILE}"

# If there is no SRC_FILE, work a bit to get one
if [ ! -f "${SRC_FILE}" ]; then
    SRC_FILE_DEFAULT="${BASE_DIR}/WalletKitTestsConfig.json"
    if [ ! -f "${SRC_FILE_DEFAULT}" ]; then
      cp "${BASE_DIR}/templates/WalletKitTestsConfig_Example.json" "${SRC_FILE_DEFAULT}"
    fi
    SRC_FILE="${SRC_FILE_DEFAULT}"
fi

# Check for the existence of a WALLET_KIT_TESTS_CONFIG_FILE file.
if [ ! -f "${SRC_FILE}" ]; then
    echo "Configuration error - no WalletKitTestsConfig.json file"
    exit 0
fi

function copyConfigFile () {
    TGT_FILE=$1

    cp "${SRC_FILE}" "${TGT_FILE}"
    chmod 644 "${TGT_FILE}"
    if [ ! -f "${TGT_FILE}" ]; then
	echo "Copy to  \"${TGT_FILE}\" failed."
    else
	echo "Copied \"${SRC_FILE}\" to \"${TGT_FILE}\""
    fi
}

# Swift
copyConfigFile "${BASE_DIR}/WalletKitSwift/WalletKitDemo/Source/WalletKitTestsConfig.json"
copyConfigFile "${BASE_DIR}/WalletKitSwift/WalletKitTests/Resources/WalletKitTestsConfig.json"

# Java
copyConfigFile "${BASE_DIR}/WalletKitJava/WalletKitDemo-Android/src/main/assets/WalletKitTestsConfig.json"

