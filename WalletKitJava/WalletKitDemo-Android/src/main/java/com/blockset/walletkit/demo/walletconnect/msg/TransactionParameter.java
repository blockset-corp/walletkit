package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

import androidx.annotation.Nullable;

import java.util.HashMap;
import java.util.Map;

/** TransactionParameter
 *  For handling parameter 0 of eth_sendTransaction which is itself an Object.
 *  In the following the scope of parameters, naming etc is what is required
 *  from the specification and working samples.
 */
public class TransactionParameter {

    /** For the sake of unmarshalling received JSON transaction arguments,
     *  'to', 'gas' are the only absolutely mandatory arguements. 'gasPrice'
     *  may be met with default fees, the the other arguments are purely optional
     *  (and in some cases, 'nonce' for example, may be ignored)
     */
    @JsonCreator
    public static TransactionParameter create(@JsonProperty("from") @Nullable       String      from,
                                              @JsonProperty("to")                   String      to,
                                              @JsonProperty("gasPrice") @Nullable   String      gasPrice,
                                              @JsonProperty("gas")                  String      gas,
                                              @JsonProperty("value") @Nullable      String      value,
                                              @JsonProperty("nonce") @Nullable      String      nonce,
                                              @JsonProperty("data")  @Nullable      String      data ) {

        return new TransactionParameter(from,
                                        checkNotNull(to),
                                        gasPrice,
                                        checkNotNull(gas),
                                        value,
                                        nonce,
                                        data);
    }

    private final @Nullable String  from;
    private final String            to;
    private final @Nullable String  gasPrice;
    private final String            gas;
    private final @Nullable String  value;
    private final @Nullable String  nonce;
    private final @Nullable String  data;

    private TransactionParameter(@Nullable String   from,
                                 String             to,
                                 @Nullable String   gasPrice,
                                 String             gas,
                                 @Nullable String   value,
                                 @Nullable String   nonce,
                                 @Nullable String   data) {
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
    public Optional<String> getFrom() { return Optional.fromNullable(from); }

    @JsonProperty("to")
    public String getTo() { return to; }

    @JsonProperty("gasPrice")
    public Optional<String> getGasPrice() { return Optional.fromNullable(gasPrice); }

    @JsonProperty("gas")
    public String getGas() { return gas; }

    @JsonProperty("value")
    public Optional<String> getValue() { return Optional.fromNullable(value); }

    @JsonProperty("nonce")
    public Optional<String> getNonce() { return Optional.fromNullable(nonce); }

    @JsonProperty("data")
    public Optional<String> getData() { return Optional.fromNullable(data); }

    /** getAsWalletConnectorArguments
     *
     * @return A map of suitable name, value pairs necessary to, and
     *         named appropriately, for passing to WalletKit transaction
     *         creation
     */
    public Map<String, String> getAsWalletConnectorArguments() {
        Map<String, String> parms = new HashMap<String, String>();
        parms.put("to", getTo());
        if (getData().isPresent())
            parms.put("data", getData().get());
        parms.put("gas", getGas());
        if (getGasPrice().isPresent())
            parms.put("gasPrice", getGasPrice().get());
        if (getData().isPresent())
            parms.put("value", getValue().get());
        return parms;
    }

}
