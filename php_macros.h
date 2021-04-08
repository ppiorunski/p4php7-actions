/*
 * php_macros.h
 *
 *  Created on: 3 Sep 2009
 *      Author: sknop
 */

#ifndef PHP_MACROS_H_
#define PHP_MACROS_H_

#define PHP_PERFORCE_DEBUG_COMMANDS 0
#define PHP_PERFORCE_DEBUG_CALLS    0
#define PHP_PERFORCE_DEBUG_DATA     0
#define PHP_PERFORCE_DEBUG_GC       0
#define PHP_PERFORCE_DEBUG_RPC      0

#define PHP_PERFORCE_DEBUG (PHP_PERFORCE_DEBUG_COMMANDS \
|| PHP_PERFORCE_DEBUG_CALLS \
|| PHP_PERFORCE_DEBUG_DATA \
|| PHP_PERFORCE_DEBUG_GC \
|| PHP_PERFORCE_DEBUG_RPC)

#if PHP_VERSION_ID >= 80000
    #define TSRMLS_D void
    #define TSRMLS_DC
    #define TSRMLS_C
    #define TSRMLS_CC
    #define TSRMLS_FETCH()
#endif

#endif /* PHP_MACROS_H_ */
