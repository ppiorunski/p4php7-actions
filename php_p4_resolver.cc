/*
 * Copyright (c) 2001-2008, Perforce Software, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PERFORCE SOFTWARE, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "clientapi.h"

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}

#include "php_perforce.h"
#include "php_mergedata.h"
#include "php_p4_mergedata.h"

#include "php_p4_resolver.h"

zend_class_entry *p4_resolver_ce;

/* P4_Resolver Class Methods */
static zend_function_entry perforce_p4_resolver_functions[] = {
    PHP_ME(P4_Resolver, __construct,  NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(P4_Resolver, resolve,      NULL, ZEND_ACC_PUBLIC | ZEND_ACC_ABSTRACT)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_Resolver. */
zend_class_entry *get_p4_resolver_ce(void)
{
    return p4_resolver_ce;
}


/* Register the P4_Resolver Class. */
void register_p4_resolver_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    REGISTER_PERFORCE_CLASS(ce, "P4_Resolver", NULL,
        perforce_p4_resolver_functions, p4_resolver_ce);
    p4_resolver_ce->ce_flags |= ZEND_ACC_IMPLICIT_ABSTRACT_CLASS;
}

/* {{{ proto void P4_Resolver::__construct()
    Create a new P4_Resolver object instance. */
PHP_METHOD(P4_Resolver, __construct)
{

}
/* }}} */

/* {{{ proto string P4_Resolver::resolve()
    Perform a resolve and return the resolve decision as a string. */
PHP_METHOD(P4_Resolver, resolve)
{
    zval *merge_info;
    zval *merge_hint;
    zend_class_entry *ce;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"z", 
            &merge_info) == FAILURE) {
        RETURN_NULL();
    }

    ce = get_p4_mergedata_ce();
    zval rv;
    merge_hint = zend_read_property(ce, merge_info, ZEND_STRS("merge_hint") - 1, 0, &rv);
    
    if (Z_TYPE_P(merge_hint) != IS_STRING) {
        RETURN_NULL();
        return;
    }
    if (strncmp(Z_STRVAL_P(merge_hint), "e", 1) == 0) {
        StrBuf m;
        m << "Standard resolver encountered merge conflict, skipping resolve";
        php_error(E_WARNING, m.Text(), 1);
        ZVAL_STRING(return_value, (char *)"s");
    } else {
        ZVAL_STRING(return_value, (char *)Z_STRVAL_P(merge_hint));
    }
}
/* }}} */
