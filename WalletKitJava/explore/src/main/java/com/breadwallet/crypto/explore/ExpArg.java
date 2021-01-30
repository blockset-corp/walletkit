package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;
import com.google.common.util.concurrent.Uninterruptibles;

import java.util.concurrent.TimeUnit;

public class ExpArg {

    private String[]    rawArgs;
    private int         argNum = 0;

    String              walletConfig;
    boolean             isMainNet = false;
    int                 systemInstances = 1;

    ExpArg(String[] args) {
        rawArgs = args;
        GetUserArgs();
    }

    public void Usage(String appName)
    {
        System.out.println("Usage: " + appName + " [Options] <path_to_wallet_kit_config_json>");
        System.out.println("  where [Options]:");
        System.out.println("    -m: use mainnet (default: false)");
        System.out.println("    -s: number of systems to instantiate (default: 1)");
    }

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

    private void GetUserArgs() {

        int argsc;
        String arg;
        String val = "";

        if (rawArgs.length == 0)
            return;

        while ((arg = argn(false)) != null) {
            if (arg.equals("m")) {
                isMainNet = true;
            } else if (arg.equals("s")) {
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
            }
        }
        walletConfig = argn(true);
    }

    boolean Ok() {
        return argNum == rawArgs.length
               && (walletConfig != null && walletConfig.length() > 0)
               && systemInstances > 0;
    }
}
