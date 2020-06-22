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
#include "php_mergedata.h"
#include "php_p4_mergedata.h"

#include <iostream>

zend_class_entry *p4_mergedata_ce;
static zend_object_handlers p4_mergedata_object_handlers;

ZEND_BEGIN_ARG_INFO(__p4_mergedata_get_args, 0)
    ZEND_ARG_PASS_INFO(0)
ZEND_END_ARG_INFO()

/* Client API Properties */
static merge_property_t p4_properties[] = {
    { "your_name",   &PHPMergeData::GetYourName },
    { "their_name",  &PHPMergeData::GetTheirName },
    { "base_name",   &PHPMergeData::GetBaseName },
    { "your_path",   &PHPMergeData::GetYourPath },
    { "their_path",  &PHPMergeData::GetTheirPath },
    { "base_path",   &PHPMergeData::GetBasePath },
    { "result_path", &PHPMergeData::GetResultPath },
    { "merge_hint",  &PHPMergeData::GetMergeHint },
    /* Sentinal */
    { NULL, NULL }
};

/* P4_MergeData Class Methods */
static zend_function_entry perforce_p4_mergedata_functions[] = {
    PHP_ME(P4_MergeData, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(P4_MergeData, __get, __p4_mergedata_get_args, ZEND_ACC_PUBLIC)
    PHP_ME(P4_MergeData, run_merge,   NULL, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_MergeData. */
zend_class_entry *get_p4_mergedata_ce(void)
{
    return p4_mergedata_ce;
}

/* free storage allocated for P4_MergeData class. */
void p4_mergedata_object_free_storage(zend_object *object TSRMLS_DC)
{
    p4_mergedata_object *obj = (p4_mergedata_object *)object;
    delete obj->mergedata;
    zend_object_std_dtor(object TSRMLS_CC);
}

/* create_object handler for P4_MergeData class. */
zend_object * p4_mergedata_create_object(zend_class_entry *type TSRMLS_DC)
{
    p4_mergedata_object *obj = (p4_mergedata_object *) ecalloc(1,
	    sizeof(struct p4_mergedata_object) + zend_object_properties_size(type));

    zend_object_std_init(&obj->std, type TSRMLS_CC);

    obj->std.handlers = &p4_mergedata_object_handlers;

    return &obj->std;
}

/* Register the P4_MergeData Class. */
void register_p4_mergedata_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "P4_MergeData", perforce_p4_mergedata_functions);
    p4_mergedata_ce = zend_register_internal_class(&ce TSRMLS_CC);
    
    p4_mergedata_ce->create_object = p4_mergedata_create_object;
    memcpy(&p4_mergedata_object_handlers, zend_get_std_object_handlers(), 
        sizeof(zend_object_handlers));
    p4_mergedata_object_handlers.clone_obj = NULL;
    p4_mergedata_object_handlers.offset = XtOffsetOf(struct p4_mergedata_object, std);
    p4_mergedata_object_handlers.free_obj = p4_mergedata_object_free_storage;
}

/* Fetch the P4MergeData instance used by this instance from the pool. */
PHPMergeData *get_merge_data(zval *this_ptr) 
{
    p4_mergedata_object *obj;
    
    TSRMLS_FETCH();
    obj = Z_P4_MERGEDATA_OBJ_P(this_ptr);
    return obj->mergedata;
}

/* {{{ proto void P4_MergeData::__construct()
    Create a new P4_MergeData object instance. */
PHP_METHOD(P4_MergeData, __construct)
{
    p4_mergedata_object *obj;
    obj = Z_P4_MERGEDATA_OBJ_P(getThis());
    obj->mergedata = NULL; 
}
/* }}} */

/* {{{ proto string P4_MergeData::run_merge()
    If the environment variable P4MERGE is defined, run it and return a boolean
    based on the return value of that program. */
PHP_METHOD(P4_MergeData, run_merge)
{
    p4_mergedata_object *obj;
    obj = Z_P4_MERGEDATA_OBJ_P(getThis());
    if (obj->mergedata != NULL) {
        obj->mergedata->RunMergeTool(return_value);
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto mixed P4_MergeData::__get(string name)
    Magic method: __get. Reads properties from PHPMergeData property list. */
PHP_METHOD(P4_MergeData, __get)
{
    p4_mergedata_object *obj;
    char *name;
    size_t name_len;
    bool found = false;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &name, &name_len) == FAILURE) {
        if ( PHP_PERFORCE_DEBUG_DATA )
            std::cerr << "__get() returning null" << std::endl;
        RETURN_NULL();
    }
    
    obj = Z_P4_MERGEDATA_OBJ_P(getThis());
    if (obj->mergedata == NULL) {
        RETURN_NULL();
    }
    
    /* look for an element in the properties table. if an accessor exists for 
     * the element, call it. */
    for (merge_property_t *ptr = p4_properties; ptr->property != NULL; ptr++)  {
        if (strcmp(name, ptr->property) == 0) {
            found = true;
            method_t method = ptr->method;
            (obj->mergedata->*method)(return_value);
        }
    }
    
    /* if no element was found in the properties table, fall back to default 
     * php class behaviour, could be an undeclared field set at runtime. */
    if (!found) {
	zval rv;
        return_value = zend_read_property(p4_mergedata_ce, getThis(), name, sizeof(name) - 1, 0, &rv);
    }
}
/* }}} */
