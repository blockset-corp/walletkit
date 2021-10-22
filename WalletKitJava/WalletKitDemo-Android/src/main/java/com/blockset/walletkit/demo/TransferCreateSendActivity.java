/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.content.Intent;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import androidx.appcompat.widget.Toolbar;
import android.text.Editable;
import android.text.Html;
import android.text.Spanned;
import android.text.TextWatcher;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;

import com.blockset.walletkit.Address;
import com.blockset.walletkit.Amount;
import com.blockset.walletkit.Network;
import com.blockset.walletkit.NetworkFee;
import com.blockset.walletkit.Transfer;
import com.blockset.walletkit.TransferAttribute;
import com.blockset.walletkit.TransferFeeBasis;
import com.blockset.walletkit.Unit;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletManager;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.LimitEstimationError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static com.google.common.base.Preconditions.checkState;

public class TransferCreateSendActivity extends AppCompatActivity {

    private static final double MIN_VALUE = 0.0;

    private static final String EXTRA_WALLET_NAME = "com.blockset.walletkit.demo,TransferCreateSendActivity.EXTRA_WALLET_NAME";

    public static void start(Activity callerActivity, Wallet wallet) {
        Intent intent = new Intent(callerActivity, TransferCreateSendActivity.class);
        intent.putExtra(EXTRA_WALLET_NAME, wallet.getName());
        callerActivity.startActivity(intent);
    }

    @Nullable
    private static Wallet getWallet(Intent intent) {
        String walletName = intent.getStringExtra(EXTRA_WALLET_NAME);
        for(Wallet wallet: DemoApplication.getSystem().getWallets()) {
            if (wallet.getName().equals(walletName)) {
                return wallet;
            }
        }
        return null;
    }

    private Network network;
    private WalletManager walletManager;
    private Wallet wallet;
    private Unit baseUnit;
    private NetworkFee selectedFee;
    private List<NetworkFee> allFees;

    @Nullable
    private TransferFeeBasis feeBasis;

    private Amount minValue;
    private Amount maxValue;

    private EditText receiverView;
    private SeekBar amountView;
    private TextView amountValueView;
    private TextView amountMinView;
    private TextView amountMaxView;
    private TextView feeView;
    private Spinner availableFeesView;
    private Button submitView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transfer_create);

        DemoApplication.initialize(this);

        wallet = getWallet(getIntent());
        if (null == wallet) {
            finish();
            return;
        }
        walletManager = wallet.getWalletManager();
        network = walletManager.getNetwork();

        // Initial default fee selection is the minimum fee,
        // assuming the network fees are returned sorted in
        // ascending order (as they are)
        allFees = (List<NetworkFee>)walletManager.getNetwork().getFees();
        selectedFee = allFees.get(0);

        baseUnit = wallet.getUnit();
        minValue = Amount.create(0, baseUnit);
        maxValue = wallet.getBalance();

        receiverView = findViewById(R.id.receiver_view);
        amountView = findViewById(R.id.amount_view);
        amountValueView = findViewById(R.id.amount_value_view);
        feeView = findViewById(R.id.fee_view);
        availableFeesView = findViewById(R.id.available_fees_view);
        submitView = findViewById(R.id.submit_view);
        amountMinView = findViewById(R.id.amount_min_view);
        amountMaxView = findViewById(R.id.amount_max_view);

        int amountViewProgress = 50;
        amountView.setProgress(amountViewProgress);
        amountView.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                updateFee();
                updateViewOnChange(progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        ArrayAdapter<NetworkFee> feeAdapter = new ArrayAdapter(this,
                                                                android.R.layout.simple_spinner_item,
                                                                allFees.toArray());
        feeAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        availableFeesView.setAdapter(feeAdapter);
        availableFeesView.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                selectedFee = (NetworkFee)parent.getItemAtPosition(position);
                updateFee();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        });

        CharSequence receiverViewText = network.isMainnet() ? "" : wallet.getTarget().toString();
        receiverView.setText(receiverViewText);
        receiverView.addTextChangedListener(new TextWatcher() {
            @Override
            public void afterTextChanged(Editable s) {
                setTransferEditDisposition();

                updateLimit();
                updateFee();
                updateViewOnChange(s);
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }
        });

        submitView.setOnClickListener(v -> {
            String addressStr = receiverView.getText().toString();
            Optional<? extends Address> target = Address.create(addressStr, network);
            if (!target.isPresent()) {
                showError("Invalid target address");
                return;
            }

            TransferFeeBasis feeBasis = this.feeBasis;
            if (feeBasis == null) {
                showError("Invalid fee");
                return;
            }

            Amount amount = calculateValue(amountView.getProgress());
            showConfirmTransfer(target.get(), amount, feeBasis);
        });

        Toolbar toolbar = findViewById(R.id.toolbar_view);
        setSupportActionBar(toolbar);

        setTransferEditDisposition();
        updateLimit();
        updateFee();
        updateView(amountViewProgress, receiverViewText);;
    }

    /** Enables various transfer editing fields only once a valid RA is
     *  entered
     */
    private void setTransferEditDisposition() {
        String addressStr = receiverView.getText().toString();
        Optional<? extends Address> target = network.addressFor(addressStr);
        boolean validRAAddress = target.isPresent();

        availableFeesView.setEnabled(validRAAddress);
        feeView.setEnabled(validRAAddress);
        amountView.setEnabled(validRAAddress);
        amountValueView.setEnabled(validRAAddress);
        amountMinView.setEnabled(validRAAddress);
        amountMaxView.setEnabled(validRAAddress);
    }

    private void updateLimit() {
        // Contingent on valid receiver address
        String addressStr = receiverView.getText().toString();
        Optional<? extends Address> target = network.addressFor(addressStr);
        if (!target.isPresent()) {
            return;
        }

        wallet.estimateLimitMinimum(target.get(), selectedFee, new CompletionHandler<Amount, LimitEstimationError>() {
            @Override
            public void handleData(Amount amount) {
                minValue = amount;
                updateViewOnLimitEstimate();
            }

            @Override
            public void handleError(LimitEstimationError error) {
                // do nothing
            }
        });

        wallet.estimateLimitMaximum(target.get(), selectedFee, new CompletionHandler<Amount, LimitEstimationError>() {
            @Override
            public void handleData(Amount amount) {
                maxValue = amount;
                updateViewOnLimitEstimate();
            }

            @Override
            public void handleError(LimitEstimationError error) {
                // do nothing
            }
        });
    }

    private void updateFee() {
        String addressStr = receiverView.getText().toString();
        Optional<? extends Address> target = Address.create(addressStr, network);
        if (!target.isPresent()) {
            return;
        }

        Amount amount = calculateValue(amountView.getProgress());
        wallet.estimateFee(target.get(), amount, selectedFee, null, new CompletionHandler<TransferFeeBasis,
                FeeEstimationError>() {
            @Override
            public void handleData(TransferFeeBasis data) {
                runOnUiThread(() -> {
                    feeBasis = data;
                    updateViewOnFeeEstimate();
                });
            }

            @Override
            public void handleError(FeeEstimationError error) {
                runOnUiThread(() -> {
                    feeBasis = null;
                    updateViewOnFeeEstimate();
                });
            }
        });
    }

    private void updateViewOnLimitEstimate() {
        updateView(amountView.getProgress(), receiverView.getText());
    }

    private void updateViewOnFeeEstimate() {
        updateView(amountView.getProgress(), receiverView.getText());
    }

    private void updateViewOnChange(Editable receiver) {
        updateView(amountView.getProgress(), receiver);
    }

    private void updateViewOnChange(int progress) {
        updateView(progress, receiverView.getText());
    }

    private void updateView(int progress, CharSequence receiver) {
        Amount amount = calculateValue(progress);
        Optional<? extends Address> target = Address.create(receiver.toString(), network);

        amountMinView.setText(minValue.toStringAsUnit(baseUnit).or(""));
        amountMaxView.setText(maxValue.toStringAsUnit(baseUnit).or(""));

        if (amount.isZero()) {
            // we have a valid amount but it is zero...
            submitView.setEnabled(false);
            amountValueView.setText(amount.toString());
            feeView.setText("");
        } else if (!target.isPresent()){
            // we don't have a valid target...
            submitView.setEnabled(false);
            amountValueView.setText(amount.toString());
            feeView.setText("");
        } else if (feeBasis == null) {
            // we have a valid target but it don't have a fee estimate...
            submitView.setEnabled(false);
            amountValueView.setText(amount.toString());
            feeView.setText("");
        } else {
            // we have it all!
            submitView.setEnabled(true);
            amountValueView.setText(amount.toString());
            feeView.setText(feeBasis.getFee().toString());
        }
    }

    private Amount calculateValue(int percentage) {
        Optional<Double> minDoubleAsBase = minValue.doubleAmount(baseUnit);
        checkState(minDoubleAsBase.isPresent());

        Optional<Double> maxDoubleAsBase = maxValue.doubleAmount(baseUnit);
        checkState(maxDoubleAsBase.isPresent());

        double deltaDouble = maxDoubleAsBase.get() - minDoubleAsBase.get();
        double valueDouble = (deltaDouble * percentage / 100) + minDoubleAsBase.get();

        return Amount.create(valueDouble, baseUnit);
    }

    private void showConfirmTransfer(Address target, Amount amount, TransferFeeBasis feeBasis) {
        String targetAsString = target.toString();

        String escapedTarget = Html.escapeHtml(targetAsString);
        String escapedAmount = Html.escapeHtml(amount.toString());
        Spanned message = Html.fromHtml(String.format("Send <b>%s</b> to <b>%s</b>?", escapedAmount, escapedTarget));

        // Assign transfer attributes, if required
        Set<TransferAttribute> attributes = new HashSet<>();
        Set<? extends TransferAttribute> walletAttributes = wallet.getTransferAttributesFor(target);
        for (TransferAttribute attribute : walletAttributes) {
            // For the Demo, only consider required attributes.
            if (attribute.isRequired()) {
                // If it is a 'DestinationTag' attribute and Coinbase, give it a destination
                if (attribute.getKey().equals("DestinationTag") &&
                        targetAsString.equals("rw2ciyaNshpHe7bCHo4bRWq6pqqynnWKQg")) // Coinbase
                    attribute.setValue("739376465");                    // My Address
                attributes.add(attribute);
            }
        }

        new AlertDialog.Builder(this)
                .setTitle("Confirmation")
                .setMessage(message)
                .setNegativeButton("Cancel",   (dialog, which) -> {})
                .setPositiveButton("Continue", (dialog, which) -> {
                    boolean haveRequiredAttributes = true;
                    for (TransferAttribute attribute : attributes) {
                        haveRequiredAttributes &=
                                attribute.isRequired() && attribute.getValue().isPresent();
                    }

                    if (!haveRequiredAttributes) {
                        showError("Missed Required Attribute");
                        return;
                    }

                    Optional<? extends Transfer> transfer = wallet.createTransfer(target, amount, feeBasis, attributes);

                    if (!transfer.isPresent()) {
                        showError("Balance too low?");
                    } else {
                        walletManager.submit(transfer.get(), DemoApplication.getPaperKey());
                        finish();
                    }
                })
                .show();
    }

    private void showError(String message) {
        new AlertDialog.Builder(this)
                .setTitle("Error")
                .setMessage(message)
                .setCancelable(false)
                .setNeutralButton("Ok", (dialog, which) -> { })
                .show();
    }
}
