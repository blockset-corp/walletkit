package com.blockset.walletkit.demo.walletconnect;

import com.blockset.walletkit.demo.DemoApplication;
import com.google.common.base.Splitter;

import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLDecoder;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import okhttp3.WebSocketListener;

// Expect URI of the form: // wc:6778992f-dbec-4fa2-a29a-f7da0bcd8268@1?bridge=https%3A%2F%2Fv.bridge.walletconnect.org&key=3fdc05843fc19940696295914cb39f0638ad94d23c0ac489d1e08c968d888978
//
// Example of URI parsing .../walletconnect-test-wallet/node_modules/@walletconnect/utils/dist/esm/session.js
public final class WalletConnectSessionDescription extends WebSocketListener {

    private static final Logger Log = Logger.getLogger(DemoApplication.class.getName());

    private static final String WALLET_CONNECT_SCHEME = "wc";
    private static final int WALLET_CONNECT_VERSION_1_0 = 1;

    private final String    topic;
    private final URL       bridge;
    private final String    dAppPubKey;

    private WalletConnectSessionDescription(

            String topic,
            String bridgeURL,
            String dAppPubKey) throws MalformedURLException {
        this.topic = topic;
        this.dAppPubKey = dAppPubKey;
        this.bridge = new URL(bridgeURL);
    }

    public String getBridge() {
        return this.bridge.toString();
    }
    public String getTopic() { return this.topic; }
    public String getDAppPubKey() { return this.dAppPubKey; }

    public static WalletConnectSessionDescription createFromWalletConnectUri(String walletConnectUri) {

        String topic;
        String bridgeUrl;
        String pubKey;

        if (walletConnectUri.length() == 0)
            return null;

        try {
            List<String> protocolAndContent = Splitter.on(':').trimResults().splitToList(walletConnectUri);
            if (!(protocolAndContent.size() == 2 && protocolAndContent.get(0).equals(WALLET_CONNECT_SCHEME))) {
                Log.log(Level.WARNING, "WC: Invalid WalletConnect URI scheme: " + walletConnectUri);
                return null;
            }
            List<String> pathAndParams = Splitter.on('?').trimResults().splitToList(protocolAndContent.get(1));
            if (pathAndParams.size() != 2) {
                Log.log(Level.WARNING, "WC: Invalid WalletConnect URI format: " + walletConnectUri);
                return null;
            }
            List<String> topicAndVersion = Splitter.on('@').trimResults().splitToList(pathAndParams.get(0));
            if (topicAndVersion.size() != 2) {
                Log.log(Level.WARNING, "WC: Invalid WalletConnect URI topic: " + walletConnectUri);
                return null;
            }

            // Version number for WalletConnect 1.0 is verified
            topic = topicAndVersion.get(0);
            int version = Integer.parseInt(topicAndVersion.get(1));
            if (version != WALLET_CONNECT_VERSION_1_0) {
                Log.log(Level.WARNING, "WC: Invalid WalletConnect URI version value:" + version);
                return null;
            }

            Map<String, String> wcQueryParms =
                    Splitter.on('&').trimResults().withKeyValueSeparator('=').split(pathAndParams.get(1));

            if ((bridgeUrl = wcQueryParms.get("bridge")) != null &&
                    bridgeUrl.length() > 0 &&
                    (pubKey = wcQueryParms.get("key")) != null &&
                    pubKey.length() > 0) {

                bridgeUrl = URLDecoder.decode(bridgeUrl, "UTF-8");
                return new WalletConnectSessionDescription(topic, bridgeUrl, pubKey);
            }

        } catch (NumberFormatException badVersion) {
            Log.log(Level.WARNING, "WC: Invalid WalletConnect URI version: " + walletConnectUri);
        } catch (MalformedURLException badBridge) {
            Log.log(Level.WARNING, "WC: Invalid WalletConnect bridge: " + walletConnectUri);
        } catch (UnsupportedEncodingException badBridgeDecode) {
            Log.log(Level.WARNING, "WC: Invalid WalletConnect bridge URL: " + walletConnectUri);
        }

        return null;
    }

    public String toString() {
        return "Wallet Connect topic:" + topic + " on " + bridge;
    }
}
