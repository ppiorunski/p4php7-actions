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

#ifndef PHP_P4_H
#define PHP_P4_H

extern Ident ident;

// Someone is changing GetMessage to GetMessageA for Windows

#ifdef GetMessage
# undef GetMessage
#endif

//
// Publically visible methods
//

PHP_METHOD(P4, __construct);
PHP_METHOD(P4, __set);
PHP_METHOD(P4, __get);
PHP_METHOD(P4, __isset);
PHP_METHOD(P4, __unset);
PHP_METHOD(P4, __call);
PHP_METHOD(P4, connect);
PHP_METHOD(P4, connected);
PHP_METHOD(P4, disconnect);
PHP_METHOD(P4, env);
PHP_METHOD(P4, format_spec);
PHP_METHOD(P4, identify);
PHP_METHOD(P4, parse_spec);
PHP_METHOD(P4, run);
PHP_METHOD(P4, run_filelog);
PHP_METHOD(P4, run_login);
PHP_METHOD(P4, run_password);
PHP_METHOD(P4, run_resolve);
PHP_METHOD(P4, run_submit);
PHP_METHOD(P4, set_protocol);
PHP_METHOD(P4, set_var);
PHP_METHOD(P4, set_evar);
PHP_METHOD(P4, get_evar);
PHP_METHOD(P4, set_trace);

//
// Register the P4 class with PHP
//

void register_p4_class(INIT_FUNC_ARGS);

//
// Debugging Functions
//

void p4php_print_r(zval *);

//
// Properties table
//

typedef void (PHPClientAPI::*mutator_t)  (zval *);
typedef void (PHPClientAPI::*accessor_t) (zval *);

struct property_t {
    const char *property;
    mutator_t mutator;
    accessor_t accessor;
    bool isset;
};

//
// Internal representation of a P4 object.
//

struct p4_object {
    PHPClientAPI *client;
    zend_object std;
};

static inline struct p4_object * php_p4_object_fetch_object(zend_object *obj) {
      return (struct p4_object *)((char *)obj - XtOffsetOf(struct p4_object, std));
}

#define Z_P4_OBJ_P(zv) php_p4_object_fetch_object(Z_OBJ_P(zv));

#endif /* PHP_P4_H */
