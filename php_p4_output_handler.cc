/*
 * Copyright (c) 2001-2011, Perforce Software, Inc.  All rights reserved.
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

#include "php_p4_output_handler.h"
#include <iostream>

zend_class_entry *p4_output_handler_interface_ce;
zend_class_entry *p4_output_handler_abstract_ce;

ZEND_BEGIN_ARG_INFO(__p4_output_args, 0)
    ZEND_ARG_INFO(0,arg)
ZEND_END_ARG_INFO()

/* P4_OutputHandlerInterface Methods */
static zend_function_entry perforce_p4_output_handler_interface_functions[] = {
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputStat,    __p4_output_args)
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputInfo,    __p4_output_args)
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputText,    __p4_output_args)
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputBinary,  __p4_output_args)
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputWarning, __p4_output_args)
    PHP_ABSTRACT_ME(P4_OutputHandlerInterface, outputError,   __p4_output_args)
    { NULL, NULL, NULL }
};

/* P4_OutputHandlerAbstract Methods */
static zend_function_entry perforce_p4_output_handler_abstract_functions[] = {
    PHP_ME(    P4_OutputHandlerAbstract, outputStat,                __p4_output_args, ZEND_ACC_PUBLIC)
    PHP_MALIAS(P4_OutputHandlerAbstract, outputInfo,    outputStat, __p4_output_args, ZEND_ACC_PUBLIC)
    PHP_MALIAS(P4_OutputHandlerAbstract, outputText,    outputStat, __p4_output_args, ZEND_ACC_PUBLIC)
    PHP_MALIAS(P4_OutputHandlerAbstract, outputBinary,  outputStat, __p4_output_args, ZEND_ACC_PUBLIC)
    PHP_MALIAS(P4_OutputHandlerAbstract, outputWarning, outputStat, __p4_output_args, ZEND_ACC_PUBLIC)
    PHP_MALIAS(P4_OutputHandlerAbstract, outputError,   outputStat, __p4_output_args, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_OutputHandlerInterface. */
zend_class_entry *get_p4_output_handler_interface_ce(void)
{
    return p4_output_handler_interface_ce;
}

/* Get the zend_class_entry for P4_OutputHandlerAbstract. */
zend_class_entry *get_p4_output_handler_abstract_ce(void)
{
    return p4_output_handler_abstract_ce;
}

/* Register the P4_OutputHandler Interface and Abstract. */
void register_p4_output_handler(INIT_FUNC_ARGS)
{
    zend_class_entry ceI, ceA;

    INIT_CLASS_ENTRY(ceI, "P4_OutputHandlerInterface", perforce_p4_output_handler_interface_functions);
    p4_output_handler_interface_ce = zend_register_internal_interface(&ceI);
    
    register_p4_output_handler_constants(p4_output_handler_interface_ce);
    
    REGISTER_PERFORCE_CLASS(ceA, "P4_OutputHandlerAbstract", NULL,
        perforce_p4_output_handler_abstract_functions, p4_output_handler_abstract_ce);
    zend_class_implements(p4_output_handler_abstract_ce, 1, p4_output_handler_interface_ce);

    p4_output_handler_abstract_ce->ce_flags |= ZEND_ACC_ABSTRACT;
}

void register_p4_output_handler_constants(zend_class_entry* ce)
{
    zend_declare_class_constant_long(ce, ZEND_STRL("HANDLER_REPORT"), HANDLER_REPORT);
    zend_declare_class_constant_long(ce, ZEND_STRL("HANDLER_HANDLED"), HANDLER_HANDLED);
    zend_declare_class_constant_long(ce, ZEND_STRL("HANDLER_CANCEL"), HANDLER_CANCEL);
}

/* {{{ proto string P4_OutputHandlerAbstract::outputStat()
    Simply return HANDLER_REPORT; used as an alias for all output methods. */
PHP_METHOD(P4_OutputHandlerAbstract, outputStat)
{
    RETURN_LONG(HANDLER_REPORT);
}
/* }}} */
