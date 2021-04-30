/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.google.common.base.Optional;

import javax.annotation.Nullable;

public interface TransferAttribute {
    String getKey();

    Optional<String> getValue();

    void setValue(@Nullable String value);

    boolean isRequired();

    enum Error {
        REQUIRED_BUT_NOT_PROVIDED,
        MISMATCHED_TYPE,
        RELATIONSHIP_INCONSISTENCY
    }
}
