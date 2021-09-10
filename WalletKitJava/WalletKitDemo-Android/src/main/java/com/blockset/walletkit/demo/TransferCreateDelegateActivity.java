/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 07/05/21.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;

import com.blockset.walletkit.Address;
import com.blockset.walletkit.Amount;
import com.blockset.walletkit.NetworkFee;
import com.blockset.walletkit.Transfer;
import com.blockset.walletkit.TransferAttribute;
import com.blockset.walletkit.TransferFeeBasis;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.logging.Logger;

public class TransferCreateDelegateActivity extends AppCompatActivity {

    private static final Logger Log = Logger.getLogger(TransferCreateDelegateActivity.class.getName());
    private static final String EXTRA_WALLET_NAME = "com.blockset.walletkit.demo,TransferCreateDelegateActivity.EXTRA_WALLET_NAME";
    private static final String DELEGATION_OP = "DelegationOp";

    private Wallet              wallet;
    private TransferFeeBasis    feeBasis;

    private Switch              enable;
    private TextView            address;
    private TextView            balance;
    private TextView            fee;
    private Button              submit;

    public static void start(Activity callerActivity, Wallet wallet) {
        Intent intent = new Intent(callerActivity, TransferCreateDelegateActivity.class);
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

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_transfer_create_delegate);

        DemoApplication.initialize(this);

        Intent intent = getIntent();

        wallet = getWallet(intent);
        if (null == wallet) {
            finish();
            return;
        }

        enable = findViewById(R.id.delegation_enable_view);
        enable.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (!isChecked) {
                    address.setText(wallet.getTarget().toString());
                    estimateFee();
                } else {
                    submit.setEnabled(false);
                    address.setText("");
                }
            }
        });

        address = findViewById(R.id.delegate_target_view);
        address.addTextChangedListener(new AddressChangeHandler());

        balance = findViewById(R.id.delegate_balance_view);
        balance.setText(wallet.getBalance().toString());

        fee = findViewById(R.id.delegate_fee_view);

        submit = findViewById(R.id.delegate_submit_view);
        submit.setEnabled(false);
        submit.setOnClickListener(new SubmissionHandler(this));

        Toolbar toolbar = findViewById(R.id.delegate_toolbar_view);
        toolbar.setTitle("Create Delegate");
        setSupportActionBar(toolbar);
    }

    private Address getTarget() {
        Optional<Address> addr = Address.create(address.getText().toString(),
                                                wallet.getWalletManager().getNetwork());
        return addr.isPresent() ? addr.get() : null;
    }

    private Set<TransferAttribute> getTransferAttributes() {
        HashSet<TransferAttribute> delegateAttrs = new HashSet<TransferAttribute>();
        Set<TransferAttribute> attrs = (Set<TransferAttribute>)wallet.getTransferAttributes();
        Iterator<TransferAttribute> i = attrs.iterator();

        while (i.hasNext()) {
            TransferAttribute attr = i.next();
            if (attr.getKey().equals(DELEGATION_OP)) {
                attr.setValue("1");
                delegateAttrs.add(attr);
            }
        }
        return delegateAttrs;
    }

    private void estimateFee() {
        Address delegate = getTarget();

        if (delegate != null) {
            Amount zeroAmount = Amount.create(0, wallet.getUnitForFee());
            NetworkFee minimumFee = wallet.getWalletManager().getNetwork().getMinimumFee();
            Set<TransferAttribute> attributes = getTransferAttributes();
            fee.setText("Fetching...");
            wallet.estimateFee(delegate,
                    zeroAmount,
                    minimumFee,
                    attributes,
                    new FeeEstimationHandler());
        }
    }

    private class FeeEstimationHandler implements CompletionHandler<TransferFeeBasis, FeeEstimationError> {

        @Override
        public void handleData(TransferFeeBasis data) {
            feeBasis = data;
            runOnUiThread(new Runnable() {
                public void run() {
                    submit.setEnabled(true);
                    fee.setText(data.getFee().toString());
                }});
        }

        @Override
        public void handleError(FeeEstimationError error) {
            fee.setText("Error: " + error.getLocalizedMessage());
        }
    }

    private class AddressChangeHandler implements TextWatcher {

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }

        @Override
        public void afterTextChanged(Editable s) {
            estimateFee();
        }
    }

    private class SubmissionHandler implements View.OnClickListener {

        private final Context context;
        SubmissionHandler(Context c) {
            this.context = c;
        }

        void showError(String msg) {
            new AlertDialog.Builder(context)
                    .setTitle("Error")
                    .setMessage(msg)
                    .setCancelable(false)
                    .setNeutralButton("Ok", (dialog, which) -> { })
                    .show();
        }

        void submitDelegationTransfer() {
            Amount zeroAmount = Amount.create(0, wallet.getUnitForFee());
            NetworkFee minimumFee = wallet.getWalletManager().getNetwork().getMinimumFee();
            Set<TransferAttribute> attributes = getTransferAttributes();
            Address target = getTarget();
            if (target == null)
                showError("Unable to obtain valid target address");
            else {
                Optional<? extends Transfer> transfer = wallet.createTransfer(getTarget(),
                        zeroAmount,
                        feeBasis,
                        attributes);
                if (transfer.isPresent()) {
                    wallet.getWalletManager().submit(transfer.get(),
                            DemoApplication.getPaperKey());
                    finish();
                } else {
                    showError("Failed to create transfer");
                }
            }
        }

        @Override
        public void onClick(View v) {
            new AlertDialog.Builder(context)
                    .setTitle("Delegation for " + wallet.getName())
                    .setMessage("Are you sure?")
                    .setNegativeButton("Cancel", (d,w) -> {})
                    .setPositiveButton("Ok", (d,w) -> {
                        submitDelegationTransfer();
                    })
                    .show();
        }
    }
}




