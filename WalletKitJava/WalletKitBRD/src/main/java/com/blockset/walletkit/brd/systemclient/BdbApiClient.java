/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import androidx.annotation.Nullable;

import com.blockset.walletkit.brd.systemclient.ObjectCoder.ObjectCoderException;
import com.blockset.walletkit.errors.SystemClientError;
import com.blockset.walletkit.errors.SystemClientSubmitError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.collect.Multimap;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.HttpUrl;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import okhttp3.ResponseBody;

public class BdbApiClient {

    private static final Logger Log = Logger.getLogger(BdbApiClient.class.getName());

    private static final MediaType MEDIA_TYPE_JSON = MediaType.parse("application/json; charset=utf-8");

    private final OkHttpClient client;
    private final String baseUrl;
    private final DataTask dataTask;
    private final ObjectCoder coder;

    public BdbApiClient(OkHttpClient client, String baseUrl, DataTask dataTask, ObjectCoder coder) {
        this.client = client;
        this.baseUrl = baseUrl;
        this.dataTask = dataTask;
        this.coder = coder;
    }

    // Create (Crud)

    void sendPost(String resource,
                  Multimap<String, String> params,
                  Object body,
                  CompletionHandler<Void, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                body,
                "POST",
                new EmptyResponseParser(),
                handler);
    }

    <T> void sendPost(String resource,
                      Multimap<String, String> params,
                      Object body,
                      Class<? extends T> clazz,
                      CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                body,
                "POST",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    <T> void sendPost(List<String> resourcePath,
                      Multimap<String, String> params,
                      Object body,
                      Class<? extends T> clazz,
                      CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                resourcePath,
                params,
                body,
                "POST",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    // Read (cRud)

    /* package */
    <T> void sendGet(String resource,
                     Multimap<String, String> params,
                     Class<? extends T> clazz,
                     CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                null,
                "GET",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetForArray(String resource,
                             Multimap<String, String> params,
                             Class<? extends T> clazz,
                             CompletionHandler<List<T>, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                null,
                "GET",
                new EmbeddedArrayResponseParser<>(resource, coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetForArray(List<String> resourcePath,
                             String embeddedPath,
                             Multimap<String, String> params,
                             Class<? extends T> clazz,
                             CompletionHandler<List<T>, SystemClientError> handler) {
        makeAndSendRequest(
                resourcePath,
                params,
                null,
                "GET",
                new EmbeddedArrayResponseParser<>(embeddedPath, coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetForArrayWithPaging(String resource,
                                       Multimap<String, String> params,
                                       Class<? extends T> clazz,
                                       CompletionHandler<PagedData<T>, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                null,
                "GET",
                new EmbeddedPagedArrayResponseHandler<>(resource, coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetForArrayWithPaging(String resource,
                                       String url,
                                       Class<? extends T> clazz,
                                       CompletionHandler<PagedData<T>, SystemClientError> handler) {
        makeAndSendRequest(
                url,
                "GET",
                new EmbeddedPagedArrayResponseHandler<>(resource, coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetWithId(String resource,
                           String id,
                           Multimap<String, String> params,
                           Class<? extends T> clazz,
                           CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                Arrays.asList(resource, id),
                params,
                null,
                "GET",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    /* package */
    <T> void sendGetWithId(List<String> resourcePath,
                           String id,
                           Multimap<String, String> params,
                           Class<? extends T> clazz,
                           CompletionHandler<T, SystemClientError> handler) {
        List<String> fullResourcePath = new ArrayList<>(resourcePath);
        fullResourcePath.add(id);

        makeAndSendRequest(
                fullResourcePath,
                params,
                null,
                "GET",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    // Update (crUd)

    <T> void sendPut(String resource,
                     Multimap<String, String> params,
                     Object body,
                     Class<? extends T> clazz,
                     CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                Collections.singletonList(resource),
                params,
                body,
                "PUT",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    <T> void sendPutWithId(String resource,
                           String id,
                           Multimap<String, String> params,
                           Object json,
                           Class<? extends T> clazz,
                           CompletionHandler<T, SystemClientError> handler) {
        makeAndSendRequest(
                Arrays.asList(resource, id),
                params,
                json,
                "PUT",
                new RootObjectResponseParser<>(coder, clazz),
                handler);
    }

    // Delete (crdD)

    /* package */
    void sendDeleteWithId(String resource,
                          String id,
                          Multimap<String, String> params,
                          CompletionHandler<Void, SystemClientError> handler) {
        makeAndSendRequest(
                Arrays.asList(resource, id),
                params,
                null,
                "DELETE",
                new EmptyResponseParser(),
                handler);
    }

    private <T> void makeAndSendRequest(String fullUrl,
                                        String httpMethod,
                                        ResponseParser<? extends T> parser,
                                        CompletionHandler<T, SystemClientError> handler) {
        HttpUrl url = HttpUrl.parse(fullUrl);
        if (null == url) {
            handler.handleError(new SystemClientError.BadRequest("Invalid base URL " + fullUrl));
            return;
        }

        HttpUrl.Builder urlBuilder = url.newBuilder();
        HttpUrl httpUrl = urlBuilder.build();
        Log.log(Level.FINE, String.format("Request: %s: Method: %s", httpUrl, httpMethod));

        Request.Builder requestBuilder = new Request.Builder();
        requestBuilder.url(httpUrl);
        requestBuilder.header("Accept", capabilities.getVersionDescription());
        requestBuilder.method(httpMethod, null);

        sendRequest(requestBuilder.build(), dataTask, parser, handler);
    }

    private <T> void makeAndSendRequest(List<String> pathSegments,
                                        Multimap<String, String> params,
                                        @Nullable Object json,
                                        String httpMethod,
                                        ResponseParser<? extends T> parser,
                                        CompletionHandler<T, SystemClientError> handler) {
        RequestBody httpBody;
        if (json == null) {
            httpBody = null;

        } else try {
            httpBody = RequestBody.create(coder.serializeObject(json), MEDIA_TYPE_JSON);

        } catch (ObjectCoderException e) {
            handler.handleError(new SystemClientError.BadRequest(e.getMessage()));
            return;
        }

        HttpUrl url = HttpUrl.parse(baseUrl);
        if (null == url) {
            handler.handleError(new SystemClientError.BadRequest("Invalid base URL " + baseUrl));
            return;
        }

        HttpUrl.Builder urlBuilder = url.newBuilder();
        for (String segment : pathSegments) {
            urlBuilder.addPathSegment(segment);
        }

        for (Map.Entry<String, String> entry : params.entries()) {
            String key = entry.getKey();
            String value = entry.getValue();
            urlBuilder.addQueryParameter(key, value);
        }

        HttpUrl httpUrl = urlBuilder.build();
        Log.log(Level.FINE, String.format("Request: %s: Method: %s: Data: %s", httpUrl, httpMethod, json));

        Request.Builder requestBuilder = new Request.Builder();
        requestBuilder.url(httpUrl);
        requestBuilder.header("Accept", capabilities.getVersionDescription());
        requestBuilder.method(httpMethod, httpBody);

        sendRequest(requestBuilder.build(), dataTask, parser, handler);
    }

    private <T> void sendRequest(Request request,
                                 DataTask dataTask,
                                 ResponseParser<? extends T> parser,
                                 CompletionHandler<T, SystemClientError> handler) {
        dataTask.execute(client, request, new Callback() {
            @Override
            public void onResponse(Call call, Response response) throws IOException {
                T data = null;
                SystemClientError error = null;

                try (ResponseBody responseBody = response.body()) {
                    int responseCode = response.code();
                    if (HttpStatusCodes.responseSuccess(request.method()).contains(responseCode)) {
                        if (responseBody == null) {
                            error = new SystemClientError.BadResponse("No Data");
                        } else {
                            data = parser.parseResponse(responseBody.string());
                        }
                    } else {
                        switch (responseCode) {
                            case 400:
                            case 404:
                                error = new SystemClientError.BadRequest("Resource Not Found: Request: " + request.toString());
                                break;
                            case 403:
                                error = new SystemClientError.Permission();
                                break;
                            case 429:
                                error = new SystemClientError.Resource();
                                break;
                            case 500:
                            case 504:
                                error = new SystemClientError.Unavailable();
                                break;
                            case 422: {
                                Map<String, Object> json = null;
                                boolean jsonError = false;

                                // Parse any responseData as JSON
                                if (responseBody == null)
                                    error = new SystemClientError.BadResponse("Submission Status Error: No 'data' Provided: Request: " + request.toString());
                                else {
                                    try {
                                        json = coder.deserializeJson(Map.class, responseBody.string());
                                    } catch (ObjectCoderException e) {
                                        error = new SystemClientError.BadResponse("Submission Status Error: Can't Parse 'data' Provided: Data: " + responseBody.string());
                                    }
                                }

                                String status = "success";

                                if (null == error) {
                                    Object statusObject = json.get("submit_status");
                                    if (statusObject instanceof String)
                                        status = (String) statusObject;
                                    else
                                        error = new SystemClientError.BadResponse("Submission Status Error: No 'submit_status' Provided: Data: " + responseBody.string());
                                }

                                if (null == error) {
                                    SystemClientSubmitError submitError;
                                    String details = json.get("network_message").toString();
                                    switch (status) {
                                        case "success":
                                        case "unknown_error":
                                            submitError = new SystemClientSubmitError.Unknown(details);
                                            break;
                                        case "fee_too_low":
                                            submitError = new SystemClientSubmitError.InsufficientFee(details);
                                            break;
                                        case "gas_too_low":
                                        case "gas_limit_too_low":
                                            submitError = new SystemClientSubmitError.InsufficientNetworkCostUnit(details);
                                            break;
                                        case "invalid_signature":
                                            submitError = new SystemClientSubmitError.Signature(details);
                                            break;
                                        case "invalid_transaction":
                                            submitError = new SystemClientSubmitError.Transaction(details);
                                            break;
                                        case "transaction_expired":
                                            submitError = new SystemClientSubmitError.TransactionExpired(details);
                                            break;
                                        case "insufficient_payer_balance":
                                            submitError = new SystemClientSubmitError.InsufficientBalance(details);
                                            break;
                                        case "nonce_already_used":
                                            submitError = new SystemClientSubmitError.NonceTooLow(details);
                                            break;
                                        case "nonce_gap":
                                        case "nonce_error":
                                            submitError = new SystemClientSubmitError.NonceInvalid(details);
                                            break;
                                        case "duplicate":
                                            submitError = new SystemClientSubmitError.TransactionDuplicate(details);
                                            break;
                                        case "rejected":
                                            submitError = new SystemClientSubmitError.Transaction(details);
                                            break;
                                        case "invalid_address_or_key":
                                        case "unknown_account":
                                            submitError = new SystemClientSubmitError.Account(details);
                                            break;

                                        // Client had an access/internal issue communicating the submission
                                        case "unauthorized":
                                        case "bad_request":
                                        case "parse_error":
                                        case "coin_node_error":
                                        case "invalid_parameters":
                                        case "method_not_found":
                                            submitError = new SystemClientSubmitError.Access(details);
                                            break;
                                        default:
                                            submitError = new SystemClientSubmitError.Unknown(details);
                                            break;
                                    }

                                    error = new SystemClientError.Submission(submitError);
                                }
                            }
                        }
                    }
                } catch (SystemClientError e) {
                    error = e;
                } catch (RuntimeException e) {
                    error = new SystemClientError.BadResponse(e.getMessage());
                }

                // if anything goes wrong, make sure we report as an error
                if (error != null) {
                    Log.log(Level.SEVERE, "response failed with error: ", error);
                    handler.handleError(error);
                } else {
                    handler.handleData(data);
                }
            }

            @Override
            public void onFailure(Call call, IOException e) {
                Log.log(Level.SEVERE, "send request failed", e);
                handler.handleError(new SystemClientError.BadResponse(e.getMessage()));
            }
        });
    }

    private interface ResponseParser<T> {
        @Nullable
        T parseResponse(String responseData) throws SystemClientError;
    }

    private static class EmptyResponseParser implements ResponseParser<Void> {

        @Override
        public Void parseResponse(String responseData) {
            return null;
        }
    }

    private static class RootObjectResponseParser<T> implements ResponseParser<T> {

        private final ObjectCoder coder;
        private final Class<? extends T> clazz;

        RootObjectResponseParser(ObjectCoder coder,
                                 Class<? extends T> clazz) {
            this.coder = coder;
            this.clazz = clazz;
        }

        @Override
        public T parseResponse(String responseData) throws SystemClientError {
            try {
                T resp = coder.deserializeJson(clazz, responseData);
                if (resp == null) {
                    throw new SystemClientError.BadResponse("Transform error");
                }

                return resp;
            } catch (ObjectCoderException e) {
                throw new SystemClientError.BadResponse("Transform Parse Error: " + e.getMessage());
            }
        }
    }

    private static class EmbeddedArrayResponseParser<T> implements ResponseParser<List<T>> {

        private final String path;
        private final ObjectCoder coder;
        private final Class<?extends T> clazz;

        EmbeddedArrayResponseParser(String path,
                                    ObjectCoder coder,
                                    Class<? extends T> clazz) {
            this.path = path;
            this.coder = coder;
            this.clazz = clazz;
        }

        @Override
        public List<T> parseResponse(String responseData) throws SystemClientError {
            try {
                BdbEmbeddedResponse resp = coder.deserializeJson(BdbEmbeddedResponse.class, responseData);
                List<T> data = (resp == null || !resp.containsEmbedded(path)) ?
                        Collections.emptyList() :
                        coder.deserializeObjectList(clazz, resp.getEmbedded(path).get());
                if (data == null) {
                    throw new SystemClientError.BadResponse("Transform EmbeddedArray error");
                }

                return data;
            } catch (ObjectCoderException e) {
                throw new SystemClientError.BadResponse("Transform EmbeddedArray Parse Error: " + e.getMessage());
            }
        }
    }

    private static class EmbeddedPagedArrayResponseHandler<T> implements ResponseParser<PagedData<T>> {

        private final String path;
        private final ObjectCoder coder;
        private final Class<? extends T> clazz;

        EmbeddedPagedArrayResponseHandler(String path,
                                          ObjectCoder coder,
                                          Class<? extends T> clazz) {
            this.path = path;
            this.coder = coder;
            this.clazz = clazz;
        }

        @Override
        public PagedData<T> parseResponse(String responseData) throws SystemClientError {
            try {
                BdbEmbeddedResponse resp = coder.deserializeJson(BdbEmbeddedResponse.class, responseData);
                List<T> data = (resp == null || !resp.containsEmbedded(path)) ?
                        Collections.emptyList() :
                        coder.deserializeObjectList(clazz, resp.getEmbedded(path).get());
                if (data == null) {
                    throw new SystemClientError.BadResponse("Transform PagedArray error");
                }

                String prevUrl = resp == null ? null : resp.getPreviousUrl().orNull();
                String nextUrl = resp == null ? null : resp.getNextUrl().orNull();
                return new PagedData<>(data, prevUrl, nextUrl);
            } catch (ObjectCoderException e) {
                throw new SystemClientError.BadResponse("Transform PagedArray Parse Error: " + e.getMessage());
            }
        }
    }

    // JSON methods

    // Capabilities

    /** The BdbApiClient capabilities */
    public Capabilities capabilities = Capabilities.current;

    /** An 'OptionSet' of capabilities. */
    public static class Capabilities {
        int rawValue;

        Capabilities(int index) {
            this.rawValue = 1 << index;
        }

        Capabilities (Capabilities... capabilitiesArray) {
            rawValue = 0;
            for (Capabilities capabilities : capabilitiesArray)
                rawValue |= capabilities.rawValue;
        }

        /**
         * Check if `capabilities` is a subset of `this`
         *
         * @param capabilities the capabilities
         * @return `true` is a subset; otherwise `false`.
         */
        public boolean hasCapabilities (Capabilities capabilities) {
            return capabilities.rawValue == (capabilities.rawValue & rawValue);
        }

        /** The `Transfer` JSON includes a status of "revert" */
        public static Capabilities transferStatusRevert = new Capabilities (0);
        /** The `Transfer` JSON includes a status of "reject" */
        public static Capabilities transferStatusReject = new Capabilities (1);
        /** The 'tdb' capability */
        public static Capabilities tbdCap               = new Capabilities (2);

        /**
         * Get a 'version string', suitable for the HTTP "Accept" Header
         *
         * @return The "Accept" Header string for `this`
         */
        String getVersionDescription () {
            if (rawValue == v2020_03_21.rawValue) return "application/vnd.blockset.V_2020-03-21+json";
            else return "application/json";
        }

        /** The 2020-03-21 capabilities are 'revert' and 'reject' */
        public static Capabilities v2020_03_21 = new Capabilities(
                transferStatusRevert,
                transferStatusReject
        );

        /** The 2020-04-xx capabilities include the 2020-03-21 capabilities with 'tbd' */
        public static Capabilities v2020_04_xx = new Capabilities(
                v2020_03_21,
                tbdCap
        );

        /** The current capabilities */
        public static Capabilities current = v2020_03_21;
    }
}
