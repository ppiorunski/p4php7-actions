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
#include "php_macros.h"
#include "php_perforce.h"
#include "php_p4_integration.h"

zend_class_entry *p4_integration_ce;

ZEND_BEGIN_ARG_INFO(__p4_no_args, 0)
ZEND_END_ARG_INFO()

/* P4_Integration Class Methods */
static zend_function_entry perforce_p4_integration_functions[] = {
    PHP_ME(P4_Integration, __construct,  __p4_no_args, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_Integration. */
zend_class_entry *get_p4_integration_ce(void)
{
    return p4_integration_ce;
}


/* Register the P4_Integration Class. */
void register_p4_integration_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    REGISTER_PERFORCE_CLASS(ce, "P4_Integration", NULL,
        perforce_p4_integration_functions, p4_integration_ce);

    /* Instance Attributes */

    zend_declare_property_string(p4_integration_ce, ZEND_STRS("how") - 1,
            "", ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_string(p4_integration_ce, ZEND_STRS("file") - 1,
            "", ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(p4_integration_ce, ZEND_STRS("erev") - 1,
            0, ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_long(p4_integration_ce, ZEND_STRS("srev") - 1,
            0, ZEND_ACC_PUBLIC TSRMLS_CC);
}

/* {{{ proto void P4_Revision::__construct()
    Create a new P4_Revision object instance. */
PHP_METHOD(P4_Integration, __construct)
{

}
/* }}} */
