package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;
import com.google.common.util.concurrent.Uninterruptibles;

import java.util.concurrent.TimeUnit;

/// @brief Collects user arguments for driving number of test instances,
///        total run time and config
public class ExpArg {

    /// @brief Raw arguments switches and values
    private String[]    rawArgs;

    /// @brief Current argument number
    private int         argNum = 0;

    /// @brief Configuration file (e.g. '.walletKitTestsConfig.json')
    String              walletConfig;

    /// @brief Indication to use mainnet or testnet testing scenario
    boolean             isMainNet = false;

    /// @brief Number of concurrent system instances (accounts) to run
    int                 systemInstances = 1;

    /// @brief Total run time before exit
    int                 runSeconds = 60;

    ExpArg(String[] args) {
        rawArgs = args;
        GetUserArgs();
    }

    public void Usage(String appName)
    {
        System.out.println("Usage: " + appName + " [Options] <path_to_wallet_kit_config_json>");
        System.out.println("  where [Options]:");
        System.out.println("    -m: use mainnet (default: false)");
        System.out.println("    -n: number of systems to instantiate (default: 1)");
        System.out.println("    -s: number of seconds to run (default: 60)");
    }

    /// @brief Get a single arguments from passed arguments
    /// @param valueArg Indicates whether a switch or switches value (valueArg == true)
    /// @return The bare argument switch or value, or null if there are insufficient
    ///         remaining arguments or the argument looks like a switch rather than value
    private String argn(boolean valueArg) {
        String arg = null;
        if (argNum < rawArgs.length - (valueArg ? 0 : 1)) {
            arg = rawArgs[argNum];
            if (arg.startsWith("-") ^ valueArg) {
                arg = arg.substring(valueArg ? 0 : 1, arg.length());
            } else {
                arg = null;
            }
            argNum++;
        }
        return arg;
    }

    /// @brief Populate application defaults from user input
    private void GetUserArgs() {

        int argsc;
        String arg;
        String val = "";

        if (rawArgs.length == 0)
            return;

        while ((arg = argn(false)) != null) {
            if (arg.equals("m")) {
                isMainNet = true;
            } else if (arg.equals("n")) {
                if ((val = argn(true)) != null) {
                    try {
                        systemInstances = Integer.parseInt(val);
                    } catch (NumberFormatException nfe) {
                        System.err.println("Err: System instances not a number");
                        systemInstances = 0;
                    }
                } else {
                    break;
                }
            } else if (arg.equals("s")) {
                if ((val = argn(true)) != null) {
                    try {
                        runSeconds = Integer.parseInt(val);
                    } catch (NumberFormatException nfe) {
                        System.err.println("Err: Run seconds not a number");
                        runSeconds = 0;
                    }
                } else {
                    break;
                }
            }
        }
        walletConfig = argn(true);
    }

    /// @brief Verify that all required input is provided and options are valid
    /// @return true on check OK
    boolean Ok() {
        return argNum == rawArgs.length
               && (walletConfig != null && walletConfig.length() > 0)
               && systemInstances > 0
               && runSeconds != 0;
    }
}
