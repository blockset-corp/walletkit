//
//  TransferTableViewCell.swift
//  CoreDemo
//
//  Created by Ed Gamble on 8/9/18.
//  Copyright © 2018-2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import UIKit
import WalletKitKotlin

class TransferTableViewCell: UITableViewCell {

    var transfer : Transfer? {
        didSet {
            updateView()
        }
    }

    var dateFormatter : DateFormatter!

    override func awakeFromNib() {
        super.awakeFromNib()
        // Initialization code
        if nil == dateFormatter {
            dateFormatter = DateFormatter()
            dateFormatter.dateFormat = "yyyy-MM-dd HH:mm:ss"
        }
    }

    override func setSelected(_ selected: Bool, animated: Bool) {
        super.setSelected(selected, animated: animated)

        // Configure the view for the selected state
    }

    func colorForState() -> UIColor {
        guard let state = transfer?.state else { return UIColor.black }
        switch state {
        case is TransferState.CREATED: return UIColor.gray
        case is TransferState.SUBMITTED: return UIColor.yellow
        case is TransferState.INCLUDED:
            return UIColor.green// TODO: transfer!.confirmation!.success ? UIColor.green : UIColor.red
        case is TransferState.DELETED: return UIColor.black

        case is TransferState.SIGNED: return UIColor.yellow
        case is TransferState.PENDING: return UIColor.yellow
        case is TransferState.FAILED/* (let reason) */: return UIColor.red
        default:
            print("todo: fix witch statements")
            return UIColor.red
        }
    }

    func updateView () {
        if let transfer = transfer {
            let date: Date? = transfer.confirmation.map {
                Date (timeIntervalSince1970: TimeInterval ($0.timestamp))
            }
            let hash: TransferHash? = transfer.hash

            dateLabel.text = date.map { dateFormatter.string(from: $0) } ?? "<pending>"
            addrLabel.text = hash.map { $0.description } ?? "<pending>"
            amountLabel.text = transfer.amountDirected.description
            feeLabel.text = "Fee: \(transfer.fee)"
            dotView.mainColor = colorForState()
            dotView.setNeedsDisplay()
        }
    }
    @IBOutlet var dateLabel: UILabel!
    @IBOutlet var amountLabel: UILabel!
    @IBOutlet var addrLabel: UILabel!
    @IBOutlet var feeLabel: UILabel!
    @IBOutlet var dotView: Dot!
}

