
//
//  BROSCompat.c
//
//  Created by Ed Gamble on 2/14/20.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#include "BROSCompat.h"
#include "time.h"
#include "sys/time.h"

#if defined (__APPLE__)
#include <Security/Security.h>
#endif

extern int
pthread_setname_brd(pthread_t thread,  const char *name) {
#if defined (__ANDROID__) || defined (__linux__)
    return pthread_setname_np (thread, name);
#elif defined (__APPLE__)
    return pthread_setname_np (name);
#else
#  error Undefined pthread_setname_brd()
#endif
}

extern void
pthread_yield_brd (void) {
#if defined (__ANDROID__) || defined (__linux__)
    nanosleep (&(struct timespec) {0, 1}, NULL); // pthread_yield() isn't POSIX standard :(
#elif defined (__APPLE__)
    pthread_yield_np ();
#else
#  error Undefined pthread_yield_brd()
#endif
}

/// MARK: - Helpers

extern int
pthread_cond_timedwait_relative_brd (pthread_cond_t *cond,
                                     pthread_mutex_t *mutex,
                                     const struct timespec *reltime) {
#if defined (__ANDROID__) || defined (__linux__)
        struct timeval t;
        gettimeofday(&t, NULL);

        struct timespec timeout;
        timeout.tv_sec  = reltime->tv_sec + t.tv_sec;
        timeout.tv_nsec = reltime->tv_nsec +t.tv_usec*1000;

        return pthread_cond_timedwait (cond, mutex, &timeout);
#elif defined (__APPLE__)
        return pthread_cond_timedwait_relative_np (cond, mutex, reltime);
#else
#  error Undefined pthread_cond_timedwait_relative_brd()
#endif
}

extern void
arc4random_buf_brd (void *bytes, size_t bytesCount) {
#if defined (__ANDROID__) || defined (__linux__)
    arc4random_buf (bytes, bytesCount);

#elif defined (__APPLE__) // IOS, MacOS
    if (0 != SecRandomCopyBytes(kSecRandomDefault, bytesCount, bytes))
        arc4random_buf (bytes, bytesCount); // fallback
#else
#  error Undefined random_bytes_brd()
#endif
}

extern uint32_t
arc4random_uniform_brd(uint32_t upperBbound) {
    return arc4random_uniform(upperBbound);
}
