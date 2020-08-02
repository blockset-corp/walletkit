//
//  WalletManagerViewController.swift
//  WalletKitDemo
//
//  Created by Ed Gamble on 8/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import UIKit
import WalletKitKotlin

class WalletManagerViewController: UIViewController, WalletManagerListener {
    var manager: WalletManager!
    let modes = [WalletManagerMode.apiOnly,
                 WalletManagerMode.apiWithP2pSubmit,
                 WalletManagerMode.p2pWithApiSync,
                 WalletManagerMode.p2pOnly]

    let addressSchemes = [AddressScheme.btclegacy,
                          AddressScheme.btcsegwit,
                          AddressScheme.ethdefault,
                          AddressScheme.gendefault]

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    override func viewWillDisappear(_ animated: Bool) {
        if let listener = UIApplication.sharedSystem.listener as? CoreDemoListener {
            listener.remove (managerListener: self)
        }

        super.viewWillDisappear(animated)
    }

    func modeSegmentIsSelected (_ index: Int) -> Bool {
        return manager.mode == modes[index]
    }

    func modeSegmentIsEnabled (_ index: Int) -> Bool {
        return manager.network.supportsWalletManagerMode(mode: modes[index])
    }

    func addressSchemeIsSelected (_ index: Int) -> Bool {
        return manager.addressScheme == addressSchemes[index]
    }

    func addressSchemeIsEnabled (_ index: Int) -> Bool {
        return manager.network.supportsAddressScheme(addressScheme: addressSchemes[index])
   }

    func connectStateIsSelected (_ index: Int, _ state: WalletManagerState? = nil) -> Bool {
        switch state ?? manager.state {
        case is WalletManagerState.DISCONNECTED: return index == 0
        case is WalletManagerState.CONNECTED:    return index == 1
        case is WalletManagerState.SYNCING:      return index == 2
        default: return false
        }
    }

    func connectStateIsEnabled (_ index: Int, _ state: WalletManagerState? = nil) -> Bool {
        switch state ?? manager.state {
        case is WalletManagerState.CREATED:      return 1 == index
        case is WalletManagerState.DISCONNECTED: return 1 == index
        case is WalletManagerState.CONNECTED:    return 0 == index || 2 == index
        case is WalletManagerState.SYNCING:      return 1 == index
        default: return false
        }
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.navigationItem.title = "\(manager.name.uppercased()) Wallet Manager"
        if let listener = UIApplication.sharedSystem.listener as? CoreDemoListener {
            listener.add (managerListener: self)
        }
        
        for index in 0..<modes.count {
            modeSegmentedControl.setEnabled (modeSegmentIsEnabled(index), forSegmentAt: index)
            if modeSegmentIsSelected(index) {
                modeSegmentedControl.selectedSegmentIndex = index
            }
        }

        for index in 0..<addressSchemes.count {
            addressSchemeSegmentedControl.setEnabled(addressSchemeIsEnabled(index), forSegmentAt: index)
            if addressSchemeIsSelected(index) {
                addressSchemeSegmentedControl.selectedSegmentIndex = index
            }
        }

        for index in 0..<3 {
            connectStateSegmentedControl.setEnabled(connectStateIsEnabled(index), forSegmentAt: index)
            if connectStateIsSelected(index) {
                connectStateSegmentedControl.selectedSegmentIndex = index
            }
        }
        self.connectStateLabel.text = manager.state.description
        self.blockHeightLabel.text  = manager.network.height.description
        self.syncProgressLabel.text = ""
        updateView ()
    }

    func updateView () {
    }

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */
    @IBOutlet var modeSegmentedControl: UISegmentedControl!
    @IBAction func selectMode(_ sender: UISegmentedControl) {
        print ("WMVC: Mode: \(modes[sender.selectedSegmentIndex].description)")
        manager.mode = modes [sender.selectedSegmentIndex]
    }
    
    @IBOutlet var addressSchemeSegmentedControl: UISegmentedControl!
    @IBAction func selectAddressScheme(_ sender: UISegmentedControl) {
        print ("WMVC: AddressScheme: \(addressSchemes[sender.selectedSegmentIndex].description)")
        manager.addressScheme = addressSchemes[sender.selectedSegmentIndex]
    }
    @IBOutlet var connectStateSegmentedControl: UISegmentedControl!
    @IBAction func selectConnectState(_ sender: UISegmentedControl) {
        switch sender.selectedSegmentIndex {
        case 0: manager.disconnect()
        case 1: manager.connect(peer: nil)
        case 2: manager.sync()
        default: break
        }
    }
    @IBOutlet var connectStateLabel: UILabel!
    @IBOutlet var blockHeightLabel: UILabel!
    @IBOutlet var syncProgressLabel: UILabel!

    func handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) {
        guard manager == self.manager else { return }
        DispatchQueue.main.async {
            switch event {
            case is WalletManagerEvent.Changed:
                let newState = (event as! WalletManagerEvent.Changed).oldState
                self.connectStateLabel.text = newState.description
                for index in 0..<3 {
                    self.connectStateSegmentedControl.setEnabled(self.connectStateIsEnabled(index, newState), forSegmentAt: index)
                    if self.connectStateIsSelected(index, newState) {
                        self.connectStateSegmentedControl.selectedSegmentIndex = index
                    }
                }
            case is WalletManagerEvent.SyncProgress:
                let percentComplete = (event as! WalletManagerEvent.SyncProgress).percentComplete
                self.syncProgressLabel.text = percentComplete.description
            case is WalletManagerEvent.SyncStopped:
                let reason = (event as! WalletManagerEvent.SyncStopped).reason
                switch reason {
                case is SyncStoppedReason.COMPLETE: self.syncProgressLabel.text = "COMPLETE"
                case is SyncStoppedReason.REQUESTED: self.syncProgressLabel.text = ""
                case is SyncStoppedReason.UNKNOWN: self.syncProgressLabel.text = "<UNKNOWN>"
                case is SyncStoppedReason.POSIX:
                    let errnum = (reason as! SyncStoppedReason.POSIX).errNum
                    let message = (reason as! SyncStoppedReason.POSIX).errMessage
                    self.syncProgressLabel.text = "\(errnum): \(message ?? "")"
                default: assert(false)
                }
            case is WalletManagerEvent.BlockUpdated:
                let height = (event as! WalletManagerEvent.BlockUpdated).height
                self.blockHeightLabel.text = height.description
            default:
                break
            }
        }
    }
}

/*
extension WalletManagerMode: CustomStringConvertible {
    public override var description: String {
        switch self {
        case .apiOnly:
            return "api_only"
        case .apiWithP2pSubmit:
            return "api_with_p2p_submit"
        case .p2pWithApiSync:
            return "p2p_with_api_sync"
        case .p2pOnly:
            return "p2p_only"
        default: assert(false)
        }
    }
}*/

/*
extension WalletManagerState: CustomStringConvertible {
    public var description: String {
        switch self {
        case .created:      return "Created"
        case .disconnected: return "Disconnected"
        case .connected:    return "Connected"
        case .syncing:      return "Syncing"
        case .deleted:      return "Deleted"
        }
    }
}
*/
