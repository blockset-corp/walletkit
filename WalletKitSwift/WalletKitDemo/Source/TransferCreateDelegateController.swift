//
//  TransferCreateDelegateController.swift
//  WalletKitDemo
//
//  Created by Ehsan Rezaie on 2020-10-09.
//  Copyright © 2020 breadwallet. All rights reserved.
//

import UIKit
import WalletKit

class TransferCreateDelegateController: TransferCreateController {

    override func viewDidLoad() {
        super.viewDidLoad()
        addressTextField.delegate = self
        submitButton.isEnabled = false
        
        balanceLabel.text = wallet.balance.description
    }
    
    private var target: Address? {
        guard let text = addressTextField.text,
              let target = Address.create(string: text, network: wallet.manager.network) else {
            return nil
        }
        return target
    }
    
    private var zeroAmount: Amount {
        return Amount.create(integer: 0, unit: wallet.unitForFee)
    }
    
    private var networkFee: NetworkFee {
        let fees = wallet.manager.network.fees.sorted { $0.timeIntervalInMilliseconds < $1.timeIntervalInMilliseconds }
        return fees.first!
    }
    
    private var transferAttributes: Set<TransferAttribute> {
        var attributes: Set<TransferAttribute> = Set()
        if let delegationOpAttribute = wallet.transferAttributes.first(where: { "DelegationOp" == $0.key }) {
            delegationOpAttribute.value = "1"
            attributes.insert(delegationOpAttribute)
        }
        return attributes
    }
    
    private func estimateFee() {
        guard let target = target else { return }
        self.feeLabel.text = "Fetching..."
        wallet.estimateFee(target: target,
                           amount: zeroAmount,
                           fee: networkFee,
                           attributes: transferAttributes) { result in
            switch result {
            case .failure(let error):
                DispatchQueue.main.async {
                    self.feeLabel.text = "Error: \(error.localizedDescription)"
                }
                
            case .success(let feeBasis):
                self.feeBasis = feeBasis
                DispatchQueue.main.async {
                    self.submitButton.isEnabled = true
                    self.feeLabel.text = "\(feeBasis.fee)"
                }
            }
        }
    }
    
    private func submitDelegationTransfer() {
        guard let target = target,
              let feeBasis = feeBasis else {
            return
        }
        
        guard let transfer = wallet.createTransfer(target: target,
                              amount: zeroAmount,
                              estimatedFeeBasis: feeBasis,
                              attributes: transferAttributes) else {
            return submitTransferFailed("Failed to create transfer")
        }
        
        wallet.manager.submit(transfer: transfer,
                              paperKey: UIApplication.paperKey)
        self.dismiss(animated: true)
    }
    
    private func submitTransferFailed (_ message: String) {
        let alert = UIAlertController (title: "Submit Transfer",
                                       message: "Failed to submit transfer - \(message)",
                                       preferredStyle: UIAlertController.Style.alert)
        alert.addAction(UIAlertAction (title: "Okay", style: UIAlertAction.Style.cancel) { (action) in
            self.dismiss(animated: true) {}
        })

        self.present (alert, animated: true) {}
    }
    
    @IBAction func submit(_ sender: Any) {
        print ("APP: TCC: Want to Delegate")
        
        let alert = UIAlertController (title: "\(enableDelegationSwitch.isOn ? "Enable" : "Disable") Delegation for \(wallet.name)",
            message: "Are you sure?",
            preferredStyle: UIAlertController.Style.actionSheet)

        alert.addAction(UIAlertAction (title: "Yes", style: UIAlertAction.Style.destructive) { (action) in
            self.submitDelegationTransfer()
        })
        
        alert.addAction(UIAlertAction (title: "No", style: UIAlertAction.Style.cancel) { (action) in
            print ("APP: TCC: Will Cancel" )
        })
        self.present(alert, animated: true) {}
    }
    
    @IBAction func cancel(_ sender: Any) {
        self.dismiss(animated: true) {}
    }
    
    @IBAction func delegationToggled(_ sender: Any) {
        addressTextField.isEnabled = enableDelegationSwitch.isOn
        if !enableDelegationSwitch.isOn {
            addressTextField.text = wallet.target.description
            estimateFee()
        } else {
            submitButton.isEnabled = false
            addressTextField.text = ""
            addressTextField.becomeFirstResponder()
        }
    }
    
    @IBOutlet var submitButton: UIBarButtonItem!
    @IBOutlet var addressTextField: UITextField!
    @IBOutlet var enableDelegationSwitch: UISwitch!
    @IBOutlet var balanceLabel: UILabel!
    @IBOutlet var feeLabel: UILabel!
    
}

extension TransferCreateDelegateController: UITextFieldDelegate {
    func textFieldShouldReturn(_ textField: UITextField) -> Bool {
        textField.endEditing(false)
    }
    
    func textFieldDidEndEditing(_ textField: UITextField) {
        estimateFee()
    }
}
