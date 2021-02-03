package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.Account;
import com.breadwallet.crypto.AddressScheme;
import com.breadwallet.crypto.Currency;
import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.NetworkFee;
import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.WalletManager;
import com.breadwallet.crypto.WalletManagerMode;
import com.breadwallet.crypto.events.system.DefaultSystemEventVisitor;
import com.breadwallet.crypto.events.system.SystemChangedEvent;
import com.breadwallet.crypto.events.system.SystemCreatedEvent;
import com.breadwallet.crypto.events.system.SystemDeletedEvent;
import com.breadwallet.crypto.events.system.SystemDiscoveredNetworksEvent;
import com.breadwallet.crypto.events.system.SystemManagerAddedEvent;
import com.breadwallet.crypto.events.system.SystemNetworkAddedEvent;
import com.breadwallet.crypto.System;

import java.util.Collections;
import java.util.List;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.annotation.Nullable;


public class ExploreSystemEventHandler<Void> extends DefaultSystemEventVisitor<Void> {

    private final System            cryptoSystem;
    private final boolean           isMainnet;
    private final Logger            logger;


    public ExploreSystemEventHandler(
            System      system,
            boolean     isMainnet,
            Logger      logger) {

        super();
        this.cryptoSystem = system;
        this.isMainnet = isMainnet;
        this.logger = logger;
    }

    @Nullable
    @Override
    public Void visit(SystemCreatedEvent event)
    {
        logger.log(Level.FINER, "[todo-ev] System created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(SystemDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] System deleted");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(SystemChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] System changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(SystemManagerAddedEvent event) {
        // Here we connect the wallet
        connectWallet(event.getWalletManager());
        return null;
    }

    @Nullable
    @Override
    public Void visit(SystemNetworkAddedEvent event) {
        // Here we create the wallet
        logger.log(Level.INFO, String.format("Network added %s",
                                           event.getNetwork().getName()));
        if (createWallet(event.getNetwork())) {
            logger.log(Level.INFO, "--> Wallet created");
        } else {
            logger.log(Level.WARNING, "Failed wallet creation");
        }
        return null;
    }

    @Nullable
    @Override
    public Void visit(SystemDiscoveredNetworksEvent event) {
        logAvailableNetworks(event.getNetworks());
        return null;
    }

    private boolean createWallet(Network net)
    {
        if (net.getCurrencies().size() == 0) {
            logger.log(Level.INFO,"~No currencies");
            return false;
        }
        String whatNet = isMainnet ? "mainnet" : "testnet";
        if (net.isMainnet() ^ isMainnet) {
            logger.log(Level.INFO, String.format("~Desired network type mismatched (on %s)",
                                                  whatNet));
            return false;
        }

        // Use network defaults for now...
        AddressScheme       addrScheme = net.getDefaultAddressScheme();
        WalletManagerMode   defMode = net.getDefaultWalletManagerMode();
        logger.log(Level.INFO, String.format("Creating %s %s wallet (addrScheme:%s, mode:%s)",
                                             whatNet,
                                             net.getName(),
                                             addrScheme,
                                             defMode));
        boolean created = cryptoSystem.createWalletManager(net,
                                                           defMode,
                                                           addrScheme,
                                                           Collections.emptySet());
        if (!created) {
            logger.log(Level.WARNING, "~recover failed wallet creation");

            // Recover: wipe network initialize accounts if required
            // CoreSystemListener attempts recovery by wiping network
            // and re-initializing the account. Its not clear if this is
            // normal course correction and need in this instance

        }
        return created;
    }

    private void connectWallet(WalletManager mgr) {
        StringBuffer desc = new StringBuffer();
        desc.append(String.format("Connecting wallet manager: %s", mgr.getName()));
        for (Wallet wallet: mgr.getWallets()) {
            desc.append(String.format(" balance %s (%s)",
                    wallet.getBalance().toStringAsUnit(wallet.getUnit()).or("None"),
                    wallet.getUnit().getSymbol()));
        }
        logger.log(Level.INFO, desc.toString());

        // Presumably the manager uses the network on which it had been created
        mgr.connect(null);
    }

    private void logAvailableNetworks(List<Network> nets) {

        int     netNumber = 1;

        logger.log(Level.INFO, String.format("%d Networks discovered",
                                             nets.size()));
        for (Network net: nets) {

            StringBuffer desc = new StringBuffer();

            // Identity
            desc.append(String.format("\n  %d) %s %s (%s)\n",
                                      netNumber,
                                      net.getName(),
                                      net.getType(),
                                      (net.isMainnet() ? "mainnet" : "testnet")));

            // Height
            desc.append(String.format("    Height: %s\n",
                                      net.getHeight()));

            // Currencies
            desc.append(String.format("    Currencies: %s\n",
                                       (net.getCurrencies()).size() > 0 ? "" : "None"));
            for (Currency currency : net.getCurrencies()) {
                desc.append(String.format("      %s %s %s %s\n",
                                          currency.getName(),
                                          currency.getType(),
                                          currency.getCode(),
                                          (currency.getIssuer().isPresent() ? currency.getIssuer().get() : "")));
            }

            // Fees
            desc.append(String.format("    Fees: %s",
                                      net.getFees().size() > 0 ? "" : "None"));
            for (NetworkFee fee: net.getFees()) {
                desc.append(String.format("%sMsec ",
                                          fee.getConfirmationTimeInMilliseconds()));
            }

            logger.log(Level.INFO, desc.toString());

            netNumber++;
        }
    }
}
