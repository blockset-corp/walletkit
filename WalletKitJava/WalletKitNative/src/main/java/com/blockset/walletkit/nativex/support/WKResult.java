package com.blockset.walletkit.nativex.support;

import javax.annotation.Nullable;

/** A result class that can indicate success or failure
 *  within the nativex object group.
 *
 *  Use of this return type simplifies native method calls
 *  that need to indicate either a return object on success
 *  or an error on failure. This approach reinforces the ability
 *  to return the primary object of interest (in particular for items
 *  that are not mutable as arguments), or an error in cases of failure;
 *  and makes it simple to determine which has occurred through the WKResult
 *  interface.
 *
 *  Use static creation methods {@link WKResult#success} or {@link WKResult#failure}
 *  to create WKResult(s).
 *
 * @param <Success>
 * @param <Failure>
 */
public class WKResult<Success, Failure> {
    private Success success;
    private Failure failure;

    private WKResult( @Nullable Success success,
                      @Nullable Failure failure)  {
        this.success = success;
        this.failure = failure;
    }

    public @Nullable Success    getSuccess()   { return success; }
    public @Nullable Failure    getFailure()   { return failure; }
    public boolean              isFailure()    { return failure != null; }
    public boolean              isSuccess()    { return !isFailure(); }

    /** Create a successful result with the object for celebration
     *
     * @param success A successful result object
     * @return A successfull result object
     */
    public static <Success> WKResult success(Success success) {
        return new WKResult(success, null);
    }

    /** Create a failed result with the object giving pause
     *
     * @param failure A successful result object
     * @return A failed result object
     */
    public static <Failure> WKResult failure(Failure failure) {
        return new WKResult(null, failure);
    }
}
