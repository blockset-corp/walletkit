package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

import java.util.HashMap;
import java.util.Map;

/** TransactionParameter
 *  For handling parameter 0 of eth_sendTransaction which is itself an Object.
 *  In the following the scope of parameters, naming etc is what is required
 *  from the specification and working samples.
 */
public class TransactionParameter {

    @JsonCreator
    public static TransactionParameter create(@JsonProperty("from")     String      from,
                                              @JsonProperty("to")       String      to,
                                              @JsonProperty("gasPrice") String      gasPrice,
                                              @JsonProperty("gas")      String      gas,
                                              @JsonProperty("value")    String      value,
                                              @JsonProperty("nonce")    String      nonce,
                                              @JsonProperty("data")     String      data ) {

        return new TransactionParameter(checkNotNull(from),
                                        checkNotNull(to),
                                        checkNotNull(gasPrice),
                                        checkNotNull(gas),
                                        checkNotNull(value),
                                        checkNotNull(nonce),
                                        checkNotNull(data));
    }

    private final String    from;
    private final String    to;
    private final String    gasPrice;
    private final String    gas;
    private final String    value;
    private final String    nonce;
    private final String    data;

    private TransactionParameter(String     from,
                                 String     to,
                                 String     gasPrice,
                                 String     gas,
                                 String     value,
                                 String     nonce,
                                 String     data) {
        this.from = from;
        this.to = to;
        this.gasPrice = gasPrice;
        this.gas = gas;
        this.value = value;
        this.nonce = nonce;
        this.data = data;
    }

    // getters

    @JsonProperty("from")
    public String getFrom() { return from; }

    @JsonProperty("to")
    public String getTo() { return to; }

    @JsonProperty("gasPrice")
    public String getGasPrice() { return gasPrice; }

    @JsonProperty("gas")
    public String getGas() { return gas; }

    @JsonProperty("value")
    public String getValue() { return value; }

    @JsonProperty("nonce")
    public String getNonce() { return nonce; }

    @JsonProperty("data")
    public String getData() { return data; }

    /** getAsWalletConnectorArguments
     *
     * @return A map of suitable name, value pairs necessary to, and
     *         named appropriately, for passing to WalletKit transaction
     *         creation
     */
    public Map<String, String> getAsWalletConnectorArguments() {
        Map<String, String> parms = new HashMap<String, String>();
        parms.put("to", getTo());
        parms.put("data", getData());
        parms.put("gas", getGas());
        parms.put("gasPrice", getGasPrice());
        parms.put("value", getValue());
        return parms;
    }

}
