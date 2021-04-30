package com.blockset.walletkit;

import java.util.Objects;

public final class SystemState {

    // create constant values, where possible
    private static final SystemState CREATED_STATE = new SystemState(Type.CREATED);
    private static final SystemState DELETED_STATE = new SystemState(Type.DELETED);

    public static SystemState CREATED() {
        return CREATED_STATE;
    }

    public static SystemState DELETED() {
        return DELETED_STATE;
    }

    public enum Type {CREATED, DELETED}

    private final Type type;

    private SystemState(Type type) {
        this.type = type;
    }

    public Type getType() {
        return type;
    }

    @Override
    public String toString() {
        switch (type) {
            case CREATED:
                return "Created";
            case DELETED:
                return "Deleted";
            default:
                throw new IllegalStateException("Invalid type");
        }
    }

    public boolean equalsSystemState(SystemState that) {
        return type == that.type;
    }

    @Override
    public boolean equals(Object that) {
        return (this == that ||
                ((that instanceof SystemState) &&
                        this.equalsSystemState((SystemState) that)));
    }

    @Override
    public int hashCode() {
        return Objects.hash(type);
    }
}
