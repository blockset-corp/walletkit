/*
 * Created by Michael Carrara.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.type.TypeFactory;
import com.fasterxml.jackson.datatype.guava.GuavaModule;

import java.util.ArrayList;
import java.util.List;

public final class ObjectCoder {

    public static final class ObjectCoderException extends Exception {

        ObjectCoderException(Throwable e) {
            super(e);
        }
    }

    /* package */
    static ObjectCoder createObjectCoderWithFailOnUnknownProperties() {
        ObjectMapper mapper = new ObjectMapper()
                .registerModule(new GuavaModule())
                .configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);

        return new ObjectCoder(mapper);
    }

    private final ObjectMapper mapper;

    private ObjectCoder(ObjectMapper mapper) {
        this.mapper = mapper;
    }

    // serializers

    public String serializeObject(Object obj) throws ObjectCoderException {
        try {
            return mapper.writeValueAsString(obj);
        } catch (JsonProcessingException e) {
            throw new ObjectCoderException(e);
        }
    }

    // deserializers

    public <X> X deserializeJson(Class<? extends X> clazz, String json) throws ObjectCoderException {
        try {
            return mapper.readValue(json, clazz);
        } catch (JsonProcessingException e) {
            throw new ObjectCoderException(e);
        }
    }

    public <X> List<X> deserializeObjectList(Class<? extends X> clazz, Object object) throws ObjectCoderException {
        TypeFactory typeFactory = mapper.getTypeFactory();
        JavaType type = typeFactory.constructCollectionLikeType(ArrayList.class, clazz);
        try {
            return mapper.convertValue(object, type);
        } catch (IllegalArgumentException e) {
            throw new ObjectCoderException(e);
        }
    }

    public <X> List<X> deserializeJsonList(Class<? extends X> clazz, String json) throws ObjectCoderException {
        TypeFactory typeFactory = mapper.getTypeFactory();
        JavaType type = typeFactory.constructCollectionLikeType(ArrayList.class, clazz);
        try {
            return mapper.readValue(json, type);
        } catch (JsonProcessingException e) {
            throw new ObjectCoderException(e);
        }
    }

    public <X> X deserializeObject(Class<? extends X> clazz, Object object) throws ObjectCoderException {
        try {
            return mapper.convertValue(object, clazz);
        } catch (IllegalArgumentException e) {
            throw new ObjectCoderException(e);
        }
    }
}
