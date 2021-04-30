/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.utility;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;

/**
 * A TestConfiguration is used during testing and when running the Android demo app; it defines
 * the `BlocksetAccess` and the list of `AccountSpecification`
 */
public class TestConfiguration {

    // creators

    @JsonCreator
    public static TestConfiguration create(@JsonProperty("blockset") BlocksetAccess blocksetAccess,
                                           @JsonProperty("accounts") List<AccountSpecification> accountSpecifications) {
        return new TestConfiguration(
                blocksetAccess,
                accountSpecifications
        );
    }

    // fields

    private final BlocksetAccess blocksetAccess;
    private final List<AccountSpecification> accountSpecifications;

    public TestConfiguration(BlocksetAccess blocksetAccess,
                             List<AccountSpecification> accountSpecifications) {
        this.blocksetAccess = blocksetAccess;
        this.accountSpecifications = accountSpecifications;
    }

    // getters

    @JsonProperty("blockset")
    public BlocksetAccess getBlocksetAccess() {
        return blocksetAccess;
    }

    @JsonProperty("accounts")
    public List<AccountSpecification> getAccountSpecifications() {
        return accountSpecifications;
    }

    private static String readStreamAsString(final BufferedReader reader) throws IOException {
        final StringBuilder output = new StringBuilder();
        String nextLine = reader.readLine();
        while (nextLine != null) {
            output.append(nextLine);
            nextLine = reader.readLine();
        }
        return output.toString();
    }

    private static String readStreamAsString(String file) {
        try (final InputStream inputStream = new FileInputStream(new File(file));
             final BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {

            return readStreamAsString(reader);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private static TestConfiguration readTestConfiguationJSON (String json) {
        ObjectMapper mapper = new ObjectMapper();
        try {
            return mapper.readValue(json, TestConfiguration.class);
        } catch (JsonProcessingException e) {
            throw new RuntimeException(e);
        }
    }

    public static
    TestConfiguration loadFrom(final BufferedReader reader) {
        try {
            return readTestConfiguationJSON(readStreamAsString(reader));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public static
    TestConfiguration loadFrom (String file) {
        return readTestConfiguationJSON(readStreamAsString(file));
    }
}
