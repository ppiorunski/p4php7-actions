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

#include "php_p4_depotfile.h"


zend_class_entry *p4_depotfile_ce;

ZEND_BEGIN_ARG_INFO(__p4_no_args, 0)
ZEND_END_ARG_INFO()

/* P4_DepotFile Class Methods */
static zend_function_entry perforce_p4_depotfile_functions[] = {
    PHP_ME(P4_DepotFile, __construct,  __p4_no_args, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_DepotFile. */
zend_class_entry *get_p4_depotfile_ce(void)
{
    return p4_depotfile_ce;
}


/* Register the P4_DepotFile Class. */
void register_p4_depotfile_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    REGISTER_PERFORCE_CLASS(ce, "P4_DepotFile", NULL,
        perforce_p4_depotfile_functions, p4_depotfile_ce);

    zend_declare_property_string(p4_depotfile_ce, ZEND_STRS("depotFile") - 1,
            "", ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(p4_depotfile_ce, ZEND_STRS("revisions") - 1,
            ZEND_ACC_PUBLIC TSRMLS_CC);
}

PHP_METHOD(P4_DepotFile, __construct)
{
    zval revisions;
    array_init(&revisions);
    #if (PHP_VERSION_ID < 80000)
      zend_update_property(p4_depotfile_ce, getThis(), ZEND_STRS("revisions") - 1, &revisions TSRMLS_CC);
    #else
      zend_update_property(p4_depotfile_ce, Z_OBJ_P(getThis()), ZEND_STRS("revisions") - 1, &revisions TSRMLS_CC);
    #endif
    zval_ptr_dtor(&revisions);
}
