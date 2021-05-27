//
//  ExportablePaperWalletController.swift
//  WalletKitDemo
//
//  Created by Ed Gamble on 3/1/21.
//  Copyright © 2021 breadwallet. All rights reserved.
//

import UIKit
import WalletKit

class ExportablePaperWalletController: UIViewController {
    var walletManager  : WalletManager!
    var exportablePaperWalletResult : Result<ExportablePaperWallet, ExportablePaperWalletError>!

    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }

    override func viewWillAppear(_ animated: Bool) {
        exportablePaperWalletResult = walletManager.createExportablePaperWallet()
        super.viewWillAppear(animated);
        updateView ()
    }

    override func viewWillDisappear(_ animated: Bool) {
        exportablePaperWalletResult = nil
        super.viewWillDisappear(animated)
    }

    func updateView () {
        switch exportablePaperWalletResult! {
        case .success (let wallet):
            exportablePaperWalletResultLabel.text = "Success"
            exportablePaperWalletAddressButton.setTitle (wallet.address?.description ?? "<missing>", for: .normal)
            exportablePaperWalletPrivateKeyButton.setTitle(wallet.privateKey?.encodeAsPrivate ?? "<missing>", for: .normal)
            break
        case .failure(let error):
            exportablePaperWalletResultLabel.text = "Failed: \(error.localizedDescription)"
            exportablePaperWalletAddressButton.setTitle ("", for: .normal)
            exportablePaperWalletPrivateKeyButton.setTitle("", for: .normal)
            break
        }
    }

    @IBAction func cancel(_ sender: Any) {
        self.dismiss(animated: true) {}
    }

    @IBAction func toPasteBoard(_ sender: UIButton) {
        UIPasteboard.general.string = sender.titleLabel?.text
    }

    @IBOutlet var exportablePaperWalletAddressButton: UIButton!
    @IBOutlet var exportablePaperWalletPrivateKeyButton: UIButton!
    @IBOutlet var exportablePaperWalletResultLabel: UILabel!
}
