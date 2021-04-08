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
#include "error.h"
#include "enviro.h"
#include "hostenv.h"
#include "i18napi.h"
#include "ident.h"
#include "strtable.h"

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}
#include "Zend/zend_exceptions.h"
#include "php_macros.h"

#include "php_p4_exception.h"
#include "php_p4_depotfile.h"
#include "php_p4_revision.h"
#include "php_p4_integration.h"

#include "specmgr.h"
#include "php_p4result.h"
#include "php_clientuser.h"
#include "php_clientapi.h"
#include "php_p4.h"

#include <iostream>

zend_class_entry *p4_ce;
static zend_object_handlers p4_object_handlers;

// convenience functions.
static zval p4php_create_revision_objects(zval *hash);
static zval p4php_create_integration_objects(zval *hash, int i);
static zval p4php_run_filelog(char *filespec, zval *this_ptr);
static zval p4php_create_p4_object(zend_class_entry *ce);
static bool p4php_client_is_tagged(zval *this_ptr);
static void p4php_fetch_spec(char *name, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS);
static void p4php_delete_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS);
static void p4php_parse_or_format_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS);
static void p4php_run_cmd(char *cmd, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS);
static void p4php_save_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS);

// reusable for functions with no parameters
ZEND_BEGIN_ARG_INFO(__p4_no_args, 0)
ZEND_END_ARG_INFO()
// argument infos: magic functions:
ZEND_BEGIN_ARG_INFO(__p4_set_args, 0)
    ZEND_ARG_INFO(0,property)
    ZEND_ARG_INFO(0,value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_get_args, 0)
        ZEND_ARG_INFO(0,property)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_isset_args, 0)
    ZEND_ARG_INFO(0,isset_args)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_call_args, 0)
    ZEND_ARG_INFO(0,call_arg_1)
    ZEND_ARG_INFO(0,call_arg_2)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_unset_args, 0)
    ZEND_ARG_INFO(0,unset_arg_1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_env_args, 0)
    ZEND_ARG_INFO(0,var)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_format_spec_args, 0)
    ZEND_ARG_INFO(0,spectype)
    ZEND_ARG_ARRAY_INFO(0,dict,0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_parse_spec_args, 0)
    ZEND_ARG_INFO(0,spectype)
    ZEND_ARG_INFO(0,spec)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_run_args, 0)
    ZEND_ARG_INFO(0,command)
    ZEND_ARG_VARIADIC_INFO(0,mixed)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_run_filelog_args, 0)
    ZEND_ARG_INFO(0,fileSpec)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_run_login_args, 0)
    ZEND_ARG_INFO(0,password)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_run_password_args, 0)
    ZEND_ARG_INFO(0,oldpass)
    ZEND_ARG_INFO(0,newpass)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_run_resolve_args, 0)
    ZEND_ARG_INFO(0,resolver)
    ZEND_ARG_ARRAY_INFO(0,args,0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_set_protocol_args, 0)
    ZEND_ARG_INFO(0,key)
    ZEND_ARG_INFO(0,value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_set_var_args, 0)
    ZEND_ARG_INFO(0,key)
    ZEND_ARG_INFO(0,value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO(__p4_set_evar_args, 0)
    ZEND_ARG_INFO(0,key)
    ZEND_ARG_INFO(0,value)
ZEND_END_ARG_INFO()


// member function argument describtions:


/* Client API Properties */
static property_t p4_properties[] = {

    /* Read / Write Properties */
    { "api_level",   &PHPClientAPI::SetApiLevel,    &PHPClientAPI::GetApiLevel,    true  },
    { "charset",     &PHPClientAPI::SetCharset,     &PHPClientAPI::GetCharset,     true  },
    { "client",      &PHPClientAPI::SetClient,      &PHPClientAPI::GetClient,      true  },
    { "cwd",         &PHPClientAPI::SetCwd,         &PHPClientAPI::GetCwd,         true  },
    { "exception_level",
            &PHPClientAPI::SetExceptionLevel, &PHPClientAPI::GetExceptionLevel,    true  },
    { "handler",     &PHPClientAPI::SetHandler,     &PHPClientAPI::GetHandler,     true  },
    { "host",        &PHPClientAPI::SetHost,        &PHPClientAPI::GetHost,        true  },
    { "input",       &PHPClientAPI::SetInput,       &PHPClientAPI::GetInput,       true  },
    { "maxlocktime", &PHPClientAPI::SetMaxLockTime, &PHPClientAPI::GetMaxLockTime, true  },
    { "maxresults",  &PHPClientAPI::SetMaxResults,  &PHPClientAPI::GetMaxResults,  true  },
    { "maxscanrows", &PHPClientAPI::SetMaxScanRows, &PHPClientAPI::GetMaxScanRows, true  },
    { "password",    &PHPClientAPI::SetPassword,    &PHPClientAPI::GetPassword,    true  },
    { "port",        &PHPClientAPI::SetPort,        &PHPClientAPI::GetPort,        true  },
    { "prog",        &PHPClientAPI::SetProg,        &PHPClientAPI::GetProg,        true  },
    { "resolver",    &PHPClientAPI::SetResolver,    &PHPClientAPI::GetResolver,    true  },
    { "streams",     &PHPClientAPI::SetStreams,     &PHPClientAPI::GetStreams,     true  },
    { "tagged",      &PHPClientAPI::SetTagged,      &PHPClientAPI::GetTagged,      true  },
    { "ticket_file", &PHPClientAPI::SetTicketFile,  &PHPClientAPI::GetTicketFile,  true  },
    { "user",        &PHPClientAPI::SetUser,        &PHPClientAPI::GetUser,        true  },
    { "version",     &PHPClientAPI::SetVersion,     &PHPClientAPI::GetVersion,     true  },
    { "expand_sequences",
            &PHPClientAPI::SetExpandSequences, &PHPClientAPI::GetExpandSequences,  true  },
    { "loginsso",    &PHPClientAPI::SetEnableSSO,   &PHPClientAPI::GetEnableSSO,   true  },
    { "ssovars",     NULL,                          &PHPClientAPI::GetSSOVars,     true  },
    { "ssopassresult",
               &PHPClientAPI::SetSSOPassResult, &PHPClientAPI::GetSSOPassResult,   true  },
    { "ssofailresult",
               &PHPClientAPI::SetSSOFailResult, &PHPClientAPI::GetSSOFailResult,   true  },

    /* Read-Only Properties */
    { "errors",        NULL, &PHPClientAPI::GetErrors,      true },
    { "p4config_file", NULL, &PHPClientAPI::GetConfig,      true },
    { "server_level",  NULL, &PHPClientAPI::GetServerLevel, true },
    { "warnings",      NULL, &PHPClientAPI::GetWarnings,    true },

    /* Sentinal */
    { NULL, NULL, NULL, false }
};

/* P4 Class Methods */
static zend_function_entry perforce_p4_functions[] = {
    PHP_ME(P4, __construct,  __p4_no_args,              ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(P4, __set,        __p4_set_args,             ZEND_ACC_PUBLIC)
    PHP_ME(P4, __get,        __p4_get_args,             ZEND_ACC_PUBLIC)
    PHP_ME(P4, __isset,      __p4_isset_args,           ZEND_ACC_PUBLIC)
    PHP_ME(P4, __call,       __p4_call_args,            ZEND_ACC_PUBLIC)
    PHP_ME(P4, __unset,      __p4_unset_args,           ZEND_ACC_PUBLIC)
    PHP_ME(P4, connect,      __p4_no_args,              ZEND_ACC_PUBLIC)
    PHP_ME(P4, connected,    __p4_no_args,              ZEND_ACC_PUBLIC)
    PHP_ME(P4, disconnect,   __p4_no_args,              ZEND_ACC_PUBLIC)
    PHP_ME(P4, env,          __p4_env_args,             ZEND_ACC_PUBLIC)
    PHP_ME(P4, format_spec,  __p4_format_spec_args,     ZEND_ACC_PUBLIC)
    PHP_ME(P4, identify,     __p4_no_args,              ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(P4, parse_spec,   __p4_parse_spec_args,      ZEND_ACC_PUBLIC)
    PHP_ME(P4, run,          __p4_run_args,             ZEND_ACC_PUBLIC)
    PHP_ME(P4, run_filelog,  __p4_run_filelog_args,     ZEND_ACC_PUBLIC)
    PHP_ME(P4, run_login,    __p4_run_login_args,       ZEND_ACC_PUBLIC)
    PHP_ME(P4, run_password, __p4_run_password_args,    ZEND_ACC_PUBLIC)
    PHP_ME(P4, run_resolve,  __p4_run_resolve_args,     ZEND_ACC_PUBLIC)
    PHP_ME(P4, run_submit,   __p4_no_args,              ZEND_ACC_PUBLIC)
    PHP_ME(P4, set_protocol, __p4_set_protocol_args,    ZEND_ACC_PUBLIC)
    PHP_ME(P4, set_var,      __p4_set_var_args,         ZEND_ACC_PUBLIC)
    PHP_ME(P4, set_evar,     __p4_set_evar_args,        ZEND_ACC_PUBLIC)
    PHP_ME(P4, get_evar,     __p4_env_args,             ZEND_ACC_PUBLIC)
    PHP_ME(P4, set_trace,    __p4_no_args,              ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

/* free storage allocated for P4 class. */
void p4_object_free_storage(zend_object *object TSRMLS_DC)
{
    p4_object *obj = php_p4_object_fetch_object(object);
    delete obj->client;
    zend_object_std_dtor(object TSRMLS_CC);
}

static void p4_object_destroy(zend_object *object TSRMLS_DC)
{
    p4_object *obj = php_p4_object_fetch_object(object);
    zend_objects_destroy_object(object);
}

/* create_object handler for P4 class. */
zend_object * p4_create_object(zend_class_entry *type TSRMLS_DC)
{
    p4_object *obj = (p4_object *) ecalloc(1, sizeof(struct p4_object) + zend_object_properties_size(type));

    zend_object_std_init(&obj->std, type TSRMLS_CC);

    obj->std.handlers = &p4_object_handlers;

    return &obj->std;
}

/* Register the P4 Class. */
void register_p4_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "P4", perforce_p4_functions);
    p4_ce = zend_register_internal_class(&ce TSRMLS_CC);

    p4_ce->create_object = p4_create_object;
    memcpy(&p4_object_handlers, zend_get_std_object_handlers(),
        sizeof(zend_object_handlers));

    p4_object_handlers.clone_obj = NULL;
    p4_object_handlers.offset = XtOffsetOf(struct p4_object, std);
    p4_object_handlers.free_obj = p4_object_free_storage;
    p4_object_handlers.dtor_obj = p4_object_destroy;
}

/* Fetch the PHPClientAPI instance used by this instance from the pool. */
PHPClientAPI *get_client_api(zval *this_ptr)
{
    p4_object *obj = Z_P4_OBJ_P(this_ptr);

    // protect against null client
    if (obj->client == NULL) {
        zend_error(E_ERROR, "Cannot get perforce client api instance");
    }

    return obj->client;
}

/* {{{ proto void P4::__construct()
    Constructor: Initialize certain fields. */
PHP_METHOD(P4, __construct)
{
    p4_object *obj = Z_P4_OBJ_P(getThis());
    obj->client = new PHPClientAPI;
}
/* }}} */

/* {{{ proto mixed P4::__get(string name)
    Magic method: __get. Reads properties from client API property list. */
PHP_METHOD(P4, __get)
{
    char *name;
    size_t name_len;
    PHPClientAPI *client;
    bool found = false;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &name, &name_len) == FAILURE) {
        if ( PHP_PERFORCE_DEBUG_DATA )
            std::cerr << "__get() returning null" << std::endl;
        RETURN_NULL();
    }

    client = get_client_api(getThis());

    /* look for an element in the properties table. if an accessor exists for
     * the element, call it. */
    for (property_t *ptr = p4_properties; ptr->property != NULL; ptr++)  {
        if (strcmp(name, ptr->property) == 0) {
            found = true;
            accessor_t accessor = ptr->accessor;
            mutator_t mutator = ptr->mutator;
            if (accessor == NULL) {
                continue;
            }
            // read-only attributes are set by the server or environment
            if (mutator != NULL && ptr->isset == false) {
                RETURN_NULL();
            }
            (client->*accessor)(return_value);
        }
    }

    /* if no element was found in the properties table, fall back to default
     * php class behaviour, could be an undeclared field set at runtime. */
    if (!found) {
	zval rv;
	#if ( PHP_VERSION_ID < 80000) 
    	zval *rval = zend_read_property(p4_ce, getThis(), name, sizeof(name) - 1, 0, &rv);
	#else 
		zval *rval = zend_read_property(p4_ce, Z_OBJ_P(getThis()), name, sizeof(name) - 1, 0, &rv);
    #endif
	ZVAL_DUP(return_value, rval);

	return;
    }

}
/* }}} */

/* {{{ proto void P4::__set(string name)
    Magic method: __set. Reads properties from client API property list. */
PHP_METHOD(P4, __set)
{
    char *name;
    size_t  name_len;
    zval *value;
    PHPClientAPI *client;
    bool read_only = false;
    bool found = false;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"sz",
            &name, &name_len, &value) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());

    /* look for an element in the properties table. if a mutator method is
     * found, call it. */
    for (property_t *ptr = p4_properties; ptr->property != NULL; ptr++) {
        if (strcmp(name, ptr->property) == 0) {
            found = true;
            mutator_t mutator = ptr->mutator;
            if (mutator == NULL) {
                read_only = true;
                continue;
            }
            ptr->isset = true;
            (client->*mutator)(value);
        }
    }

    /* throw an exception if caller is trying to set a read-only attribute. */
    if (read_only) {
        StrBuf m;
        m << "Attempted to set read-only attribute: ";
        m.Append(name);
        zend_throw_exception_ex(get_p4_exception_ce(),
                0 TSRMLS_CC, m.Text());
    }

    /* fallback to default php behaviour if attribute is not found. */
    if (!found) {
      	#if ( PHP_VERSION_ID < 80000)
          zend_update_property(p4_ce, getThis(), name, sizeof(name) - 1, value TSRMLS_CC);
        #else
            zend_update_property(p4_ce, Z_OBJ_P(getThis()), name, sizeof(name) - 1, value TSRMLS_CC);
        #endif
    }
}
/* }}} */

/* {{{ proto bool P4::__isset(string name)
    Magic method: __isset. */
PHP_METHOD(P4, __isset)
{
    char *name;
    size_t name_len;
    bool isset;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &name, &name_len) == FAILURE) {
        RETURN_NULL();
    }

    // default to false
    isset = false;

    for (property_t *ptr = p4_properties; ptr->property != NULL; ptr++) {
        if (strcmp(name, ptr->property) == 0) {
            isset = ptr->isset;
        }
    }

    RETVAL_BOOL(isset);
}
/* }}} */

/* {{{ proto mixed P4::__call(string name, array arguments)
    Magic method: __call. triggered when invoking inaccessible methods. */
PHP_METHOD(P4, __call)
{
    char *name;
    size_t name_len;
    zval *args;
    zval func;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"sz",
            &name, &name_len, &args) == FAILURE) {
        RETURN_NULL();
    }

    // run method name
    ZVAL_STRING(&func, (char *)"run");

    if (strncmp(name, "fetch_", sizeof("fetch_") - 1) == 0) {
        name += strlen("fetch_");
        p4php_fetch_spec(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else if (strncmp(name, "delete_", sizeof("delete_") - 1) == 0) {
        name += strlen("delete_");
        p4php_delete_spec(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else if (strncmp(name, "format_", sizeof("format_") - 1) == 0) {
        name += strlen("format_");
        // set function name
        zval_dtor(&func);
        ZVAL_STRING(&func, (char *)"format_spec");
        p4php_parse_or_format_spec(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else if (strncmp(name, "parse_", sizeof("parse_") - 1) == 0) {
        name += strlen("parse_");
        // set function name
        zval_dtor(&func);
        ZVAL_STRING(&func, (char *)"parse_spec");
        p4php_parse_or_format_spec(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else if (strncmp(name, "run_", sizeof("run_") - 1) == 0) {
        name += strlen("run_");
        p4php_run_cmd(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else if (strncmp(name, "save_", sizeof("save_") - 1) == 0) {
        name += strlen("save_");
        p4php_save_spec(name, func, args, INTERNAL_FUNCTION_PARAM_PASSTHRU);
        zval_dtor(&func);
        return;
    } else {
        // Method not found.
        StrBuf msg;
        msg << "Call to undefined method P4::";
        msg.Append(name);
        msg.Append("()");
        php_error(E_ERROR, msg.Text(), 1);
        RETURN_NULL();
    }
}
/* }}} */

/* {{{ proto void P4::__unset(string name)
    Invoked when unset is used on an inaccessible member. */
PHP_METHOD(P4, __unset)
{
    char *name;
    size_t name_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &name, &name_len) == FAILURE) {
        RETURN_NULL();
    }

    for (property_t *ptr = p4_properties; ptr->property != NULL; ptr++) {
        if (strcmp(name, ptr->property) == 0) {
            zval val;
            ptr->isset = false;
            mutator_t mutator = ptr->mutator;
            if (mutator == NULL) continue;
            ZVAL_NULL(&val);
            client = get_client_api(getThis());
            (client->*mutator)(&val);
        }
    }
}
/* }}} */

/* {{{ proto bool P4::connect()
    Connect to the perforce server. */
PHP_METHOD(P4, connect)
{
    zval connected;
    PHPClientAPI *client;

    client = get_client_api(getThis());
    connected = client->Connect();

    if (Z_TYPE(connected) != IS_TRUE) {
        RETURN_FALSE;
    }

    RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool P4::connected()
    Determine whether client is connected or not. */
PHP_METHOD(P4, connected)
{
    PHPClientAPI *client = get_client_api(getThis());
    RETURN_BOOL(client->Connected());
}
/* }}} */

/* {{{ proto void P4::disconnect()
    Disconnect from Perforce server. */
PHP_METHOD(P4, disconnect)
{
    PHPClientAPI *client = get_client_api(getThis());
    client->Disconnect();
}
/* }}} */

/* {{{ proto string P4::env(string var)
    Get a value from the environment following the Perforce conventions,
    including honouring P4CONFIG files, etc. */
PHP_METHOD(P4, env)
{
    char *var;
    size_t var_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &var, &var_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    ZVAL_STRING(return_value, (char *)client->GetEnv(var));
}
/* }}} */

/* {{{ proto string P4::format_spec(string spectype, array dict)
 *  Converts fields in dict to a Perforce form (spec). */
PHP_METHOD(P4, format_spec)
{
    const char *type;
    size_t type_len;
    zval *hash;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"sz",
            &type, &type_len, &hash) == FAILURE) {
        RETURN_NULL();
    }
    client = get_client_api(getThis());

    // FormatSpec wraps estrdup, so don't duplicate from ZVAL_STRING
    RETURN_STR(client->FormatSpec(type, hash));
}
/* }}} */

/* {{{ proto string P4::identify()
    Print build information including P4-PHP version and P4API version. */
PHP_METHOD(P4, identify)
{
    StrBuf s;
    ident.GetMessage(&s);
    RETVAL_STRING(s.Text());
}
/* }}} */

/* {{{ proto array P4::parse_spec(string spectype, string spec)
    Parses a Perforce form (spec) into a array. */
PHP_METHOD(P4, parse_spec)
{
    const char *type, *form;
    size_t type_len, form_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"ss",
            &type, &type_len, &form, &form_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    client->ParseSpec(type, form, return_value);
}
/* }}} */

/* {{{ proto mixed P4::run(string command, [, mixed ... ])
    Run a Perforce command and return its results. */
PHP_METHOD(P4, run)
{
    int argc = ZEND_NUM_ARGS();
    zval *args;
    StrBuf cmdString;
    PHPClientAPI *client;

    client = get_client_api(getThis());

    args = (zval *) safe_emalloc(argc, sizeof(zval), 0);
    if (argc == 0 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
        efree(args);
        WRONG_PARAM_COUNT;
    }

    zend_string ** strArgs = (zend_string **) safe_emalloc(argc, sizeof(zend_string *), 0);
    for (int i = 0 ; i < argc; i++) {
        strArgs[i] = zval_get_string(&args[i]);
    }

    if (PHP_PERFORCE_DEBUG) {
        cmdString << "\"p4";
        for (int i = 0; i < argc; i++) {
            switch (Z_TYPE(args[i])) {
                case IS_STRING:
                    cmdString << " " << Z_STRVAL(args[i]);
                    break;
                case IS_LONG:
                    cmdString << " " << Z_LVAL(args[i]);
                    break;
                case IS_DOUBLE:
                    cmdString << " " << Z_DVAL(args[i]);
                    break;
                case IS_TRUE:
                    cmdString << " " << "TRUE";
                    break;
                case IS_FALSE:
                    cmdString << " " << "FALSE";
                    break;
                case IS_REFERENCE:
                    cmdString << " REFERENCE";
                    break;
                default:
                    cmdString << " [Non alpha-numeric arg]";
                    break;
            }
        }
        cmdString << "\"";
        std::cerr << "[P4] {php_run} " << cmdString.Text() << std::endl;
    }

    // run command with args.
    client->Run(strArgs, argc, return_value);
    efree(args);
    for (int i = 0; i < argc; i++ ) {
        zend_string_release(strArgs[i]);
    }
    efree(strArgs);
}

/* }}} */

void enumerate_how(zval *record, zval *integrations, zend_string *key)
{
    zval* record_elem;
    int counter = 0;

    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(record), record_elem)
    {
	zval* integration;
	// fetch the actual P4_Integration instance out of the array we
	// allocated earlier.
	if ( (integration = zend_hash_index_find(Z_ARRVAL_P(integrations), counter)) == NULL ) {
	    php_error(E_WARNING, "Could not retrieve P4_Integration instance", 1);
	    continue;
	}

  #if ( PHP_VERSION_ID < 80000) 
    zend_update_property_ex(get_p4_integration_ce(), integration, key, record_elem);
  #else 
    zend_update_property_ex(get_p4_integration_ce(), Z_OBJ_P(integration), key, record_elem);
  #endif
	counter++;
    }
    ZEND_HASH_FOREACH_END();
}

void
enumerate_revisions(zend_class_entry* ce, zval& p4_depotfile, zval *val, zval* revision, int i, zval* integrations)
{
    // loop through each element of the hash representing this revision:
    // {
    //    'depotFile' => '//depot/foo',
    //    'client' => [ 'client', 'client', 'client', 'client' ],
    //    'how' => [ [ 'ignored' ], None, None, [ 'branch from' ] ]
    // }

    zval * values;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(val), values)
    {
	/*****************************************************************
	 * There are three types of values we will encounter in this loop:
	 *
	 * 1. Scalars - We only expect 'depotFile' which is assigned to the
	 *              P4_DepotFile instance as it will be the same for
	 *              each revision.
	 *
	 * 2. Arrays  - Such as 'client', 'rev', 'action'. We will extract
	 *              the element that corresponds to the revision we are
	 *              currently working on (determined by the loop counter)
	 *              and assign the element as a property on the revision.
	 *
	 * 3. Arrays of Arrays - Such as 'how', 'srev'. These indicate
	 *              integrations on the specific revision. Like all arrays
	 *              we will extract the element (which will be an array)
	 *              that corresponds to the revision we are currently
	 *              working with. If the value is NULL, we can assume that
	 *              there are no integrations on the revision. If the value
	 *              is an array, we must collect each of the elements,
	 *              assigning them to the corresponding P4_Integration
	 *              object for this revision.
	 ******************************************************************/
	// If we found a non-array entry, it's the depotFile - assign it
	// and move on
	if ( Z_TYPE_P(values) != IS_ARRAY ) {
    #if ( PHP_VERSION_ID < 80000) 
	    zend_update_property(ce, &p4_depotfile, ZEND_STRS("depotFile") - 1, values);
    #else 
      zend_update_property(ce, Z_OBJ_P(&p4_depotfile), ZEND_STRS("depotFile") - 1, values);
    #endif
	    continue;
	}

	HashTable *ht = Z_ARRVAL_P(values);
	HashPosition pos = 0;
	zend_string *key;
	zend_ulong index;

	int cur = zend_hash_get_current_key_ex(ht, &key, &index, &pos);

	if ( cur == HASH_KEY_IS_LONG ) {
	    // Not quite sure what encountering an element with a numeric
	    // key would mean... probably best to just move along and pretend
	    // we didn't see it. *whistles*
	    continue;
	}
	// depotFile needs to be set on the P4_DepotFile instance
	// if (strncmp(key, "depotFile", strlen("depotFile")) == 0) {
	//     zend_update_property(ce, p4_depotfile, key, key_len - 1,
	//                          *values TSRMLS_CC);
	// }
	// if it's not an array, simply set the property on the
	// P4_Revision object.
	if ( Z_TYPE_P(values) != IS_ARRAY ) {
    #if ( PHP_VERSION_ID < 80000) 
	    zend_update_property_ex(get_p4_revision_ce(), revision, key, values);
    #else
      zend_update_property_ex(get_p4_revision_ce(), Z_OBJ_P(revision), key, values);
    #endif
	    continue;
	}
	// extract the element from the values array that corresponds
	// to the revision object we're dealing with currently
	// if no element exists, act as though it is null and continue
	zval* record;
	if ( (record = zend_hash_index_find(Z_ARRVAL_P(values), i )) == NULL ) {
	    continue;
	}
	if ( Z_TYPE_P(record) != IS_ARRAY ) {
    #if ( PHP_VERSION_ID < 80000)
	    zend_update_property_ex(get_p4_revision_ce(), revision, key, record);
    #else
      zend_update_property_ex(get_p4_revision_ce(), Z_OBJ_P(revision), key, record);
    #endif
	    continue;
	}
	// at this point we're probably dealing with an integration
	// record (e.g. 'how', 'srev', 'erev', ...). double check
	if ( zend_string_equals_literal(key, "how") != 0
		&& zend_string_equals_literal(key, "file") != 0
		&& zend_string_equals_literal(key, "srev") != 0
		&& zend_string_equals_literal(key, "erev") != 0 ) {
	    if ( PHP_PERFORCE_DEBUG )
		std::cerr << "Unexpected element: " << key << std::endl;

	    continue;
	}
	// at this point, key, record will be something like:
	// erev, Array( [0] => #1, [1] => #2 )
	if ( integrations == NULL ) {
	    php_error(E_WARNING, "Error parsing integrations.", 1);
	    continue;
	}
	enumerate_how(record, integrations, key);
    }
    ZEND_HASH_FOREACH_END();
}

/* {{{ proto array P4::run_filelog(string fileSpec)
 *  Runs a p4 filelog and returns an array of P4_DepotFile objects. */
PHP_METHOD(P4, run_filelog)
{
    char *filespec;
    size_t filespec_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
           &filespec, &filespec_len) == FAILURE) {
        RETURN_NULL();
    }

    // get raw 'filelog' results.
    zval results = p4php_run_filelog(filespec, getThis());


    if (Z_TYPE(results) == IS_NULL) {
        // return an empty array.
        array_init(return_value);
        zval_dtor(&results);

        return;
    }

    if (!p4php_client_is_tagged(getThis())) {
        // if tagged output is disabled, our job is mostly done.
        // just copy the value in results to return_value and free it.
        RETVAL_ZVAL(&results, 1, 1);
        return;
    }

    /*
     * Filelog results are returned as an array of hashes. The elements of
     * the hashes will be arrays in most cases or strings. We need to
     * transform these hashes into P4_DepotFile instances. Each
     * P4_DepotFile instance has a depotFile field and a revisions array.
     * Each element in the revisions array is a P4_Revision instance. Each
     * P4_Revision instance has a number of scalar fields and an array of
     * P4_Integration instances.
     *
     * For example, the following:
     * [ { rev: [2, 1], depotFile : '//depot/file.txt', action: ['edit', 'add'] } ]
     *
     * Would become the following:
     * [ P4_DepotFile(
     *      depotFile = '//depot/file.txt',
     *      revisions = [
     *          P4_Revision( rev = 1, action = add ),
     *          P4_Revision( rev = 2, action = edit )
     *      ]
     *   )
     * ]
     */


    array_init(return_value);


    // tagged results. results will be an array of hashes, each hash will be
    // represented by one P4_DepotFile instance. Each P4_DepotFile instance
    // can have one or more P4_Revision instances and each P4_Revision
    // instance can have zero or more P4_Integration instances.
    zval *val;
    ZEND_HASH_FOREACH_VAL(Z_ARRVAL(results), val)
    {
        // elem should be an array of hashes which will be represented as
        // a P4_DepotFile object:
        //
        // [
        //     {
        //         'depotFile' => '//depot/bar',
        //         'rev' => [4, 3, 2, 1],
        //         'how' => [ [ 'ignored' ], None, None, [ 'branch from' ] ],
        //                    ^ list of integrations in the first revision
        //         ...
        //     }
        // ]

        // bail if element is not a hash for some reason.
        if (Z_TYPE_P(val) != IS_ARRAY) {
            continue;
        }

        // Create a P4_DepotFile object to represent this hash.
        zend_class_entry *ce = get_p4_depotfile_ce();
        zval p4_depotfile = p4php_create_p4_object(ce);

        // Create P4_Revision objects for this P4_DepotFile instance.
        zval revisions = p4php_create_revision_objects(val);

        // loop over each of the revisions in this P4_DepotFile instance
        int nrevs = zend_hash_num_elements(Z_ARRVAL(revisions));

        // in the above example, there are 4 revisions (one for each element
        // in each of the array elements).
        for (int i = 0; i < nrevs; i++) {
            zval *revision;

            // fetch the actual P4_Revision instance out of the array we
            // allocated earlier.
            if ((revision = zend_hash_index_find(Z_ARRVAL(revisions), i)) == NULL)
            {
                php_error(E_WARNING,
                  "Could not retrieve P4_Revision instance", 1);
                continue;
            }

            // create P4_Integration objects for this revision.
            zval integrations = p4php_create_integration_objects(val, i);

            // loop through each element of the hash representing this revision:
            // {
            //    'depotFile' => '//depot/foo',
            //    'client' => [ 'client', 'client', 'client', 'client' ],
            //    'how' => [ [ 'ignored' ], None, None, [ 'branch from' ] ]
            // }


            enumerate_revisions(ce, p4_depotfile, val, revision, i, &integrations);

            if (!Z_ISNULL(integrations)) {
              #if ( PHP_VERSION_ID < 80000) 
                zend_update_property(get_p4_revision_ce(), revision,
                                     ZEND_STRS("integrations")-1, &integrations TSRMLS_CC);
              #else
                zend_update_property(get_p4_revision_ce(), Z_OBJ_P(revision),
                                     ZEND_STRS("integrations")-1, &integrations TSRMLS_CC);
              #endif
                Z_TRY_DELREF(integrations);
            }

        } /* for (int i = 0; i < nrevs; i++) */


        for (int i = 0; i < nrevs; i++) {
            zval *revision;
            if ((revision = zend_hash_index_find(Z_ARRVAL(revisions), i)) == NULL)
            {
                php_error(E_WARNING, "Problem parsing revision output.", 1);
            }
        }

        #if ( PHP_VERSION_ID < 80000) 
          zend_update_property(ce, &p4_depotfile, ZEND_STRS("revisions")-1, &revisions TSRMLS_CC);
        #else 
          zend_update_property(ce, Z_OBJ_P(&p4_depotfile), ZEND_STRS("revisions")-1, &revisions TSRMLS_CC);
        #endif
        Z_TRY_DELREF(revisions);
        add_next_index_zval(return_value, &p4_depotfile);
    }
    ZEND_HASH_FOREACH_END();

    zval_dtor(&results);
}
/* }}} */

/* {{{ proto array P4::run_login(string password).
    Runs p4 login using a password set by the user. */
PHP_METHOD(P4, run_login)
{
    zval *object;
    PHPClientAPI *client;
    zval params[1];
    zval param0;
    zval *password;
    zval func;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"z", &password) == FAILURE) {
        RETURN_NULL();
    }

    ZVAL_STRING(&func, "run");
    ZVAL_STRING(&param0, "login");

    object = getThis();
    client = get_client_api(object);
    client->SetInput(password);

    params[0] = param0;

    call_user_function(NULL, object, &func, return_value, 1,  params TSRMLS_CC);

    zval_dtor(&func);
    zval_dtor(&param0);
}
/* }}} */

/* {{{ proto array P4::run_password(string oldpass, string newpass).
    A thin wrapper to make it easy to change your password. */
PHP_METHOD(P4, run_password)
{
    zval *object;

    zval *oldpass, *newpass;

    zval params[1];
    zval func, param0;

    zval input;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"zz",
            &oldpass, &newpass) == FAILURE) {
        RETURN_NULL();
    }

    ZVAL_STRING(&func, "run");
    ZVAL_STRING(&param0, "passwd");

    params[0] = param0;

    array_init(&input);
    add_next_index_zval(&input, oldpass);
    add_next_index_zval(&input, newpass);
    add_next_index_zval(&input, newpass);

    object = getThis();
    client = get_client_api(object);
    client->SetInput(&input);

    call_user_function(NULL, object, &func, return_value, 1, params TSRMLS_CC);

    zval_dtor(&func);
    zval_dtor(&param0);
    zval_dtor(&input);
}
/* }}} */

/* {{{ proto array P4::run_resolve([P4_Resolver resolver, array args]).
    Run a p4 resolve command. Interactive resolves need a resolver parameter. */
PHP_METHOD(P4, run_resolve)
{
    zval *object;
    PHPClientAPI *client;
    zval *args;
    int argc;
    zval func, param0;
    zval *params;

    argc = ZEND_NUM_ARGS();
    args = (zval *) safe_emalloc(argc, sizeof(zval), 0);
    if (argc < 1 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
	efree(args);
        WRONG_PARAM_COUNT;
    }

    // set up function names
    ZVAL_STRING(&func, "run");
    ZVAL_STRING(&param0, "resolve");

    // caller passed us a P4_Resolver object.
    object = getThis();
    if (argc == 1 && Z_TYPE(args[0]) != IS_STRING) {
        client = get_client_api(object);
        client->SetResolver(&args[0]);
        params = (zval *)safe_emalloc(argc, sizeof(zval), 0);
        params[0] = param0;
    } else {
        // treat this as an ordinary call to resolve (with varargs)
        params = (zval *)safe_emalloc(argc + 1, sizeof(zval), 0);
        params[0] = param0;
        for (int i = 0; i < argc; i++) {
            params[i + 1] = args[i];
        }
        argc += 1;
    }

    // call p4->run(resolve, [...])
    call_user_function(NULL, object, &func, return_value, argc, params TSRMLS_CC);

    zval_dtor(&func);
    zval_dtor(&param0);
    efree(args);
    efree(params);
}
/* }}} */

/* {{{ proto array P4::run_submit().
    Submit a changelist to the server. */
PHP_METHOD(P4, run_submit)
{
    zval *object;
    PHPClientAPI *client;
    zval *args;
    int argc = ZEND_NUM_ARGS();
    zval func;
    zval param0;
    zval param1;
    zval *params;

    args = (zval *)safe_emalloc(argc, sizeof(zval), 0);
    if (argc < 1 || zend_get_parameters_array_ex(argc, args) == FAILURE) {
        efree(args);
        WRONG_PARAM_COUNT;
    }

    ZVAL_STRING(&func, "run");
    ZVAL_STRING(&param0, "submit");
    ZVAL_STRING(&param1, "-i");

    object = getThis();

    if (argc == 1) {
        params = (zval *)safe_emalloc(argc + 2, sizeof(zval), 0);
        params[0] = param0;
        params[1] = param1;
        argc = 2;
        // third parameter will be a hash.
        client = get_client_api(object);
        // convert args[0] to an array.
        if (Z_TYPE(args[0]) == IS_ARRAY) {
            client->SetInput(&args[0]);
        }
    } else if (argc == 2) {
        params = (zval *)safe_emalloc(argc + 1, sizeof(zval), 0);
        params[0] = param0;
        // first parameter will be submit flag(s).
        params[1] = args[0];
        // convert args[1] to an array
        if (Z_TYPE(args[1]) == IS_ARRAY) {
            // second parameter will be a hash.
            client = get_client_api(object);
            client->SetInput(&args[1]);
        } else {
            // otherwise treat it as a parameter.
            argc = 3;
            params[2] = args[1];
        }
    } else {
        params = (zval *)safe_emalloc(argc + 1, sizeof(zval), 0);
        params[0] = param0;
        // feed parameters into run submit.
        for (int i = 0; i < argc; i++) {
            params[i + 1] = args[i];
        }
    }

    call_user_function(NULL, object, &func, return_value, argc, params TSRMLS_CC);

    zval_dtor(&func);
    zval_dtor(&param0);
    zval_dtor(&param1);
    efree(params);
    efree(args);
}
/* }}} */

/* {{{ proto null P4::set_protocol(string key, string value)
    Set a protocol key/value for the Perforce connection. */
PHP_METHOD(P4, set_protocol)
{
    char *key, *value;
    size_t key_len, value_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"ss",
            &key, &key_len, &value, &value_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    client->SetProtocol(key, value);

    RETURN_NULL();
}
/* }}} */

/* {{{ proto null P4::set_var(string key, string value)
    Set a var key/value for the Perforce connection. */
PHP_METHOD(P4, set_var)
{
    char *key, *value;
    size_t key_len, value_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"ss",
            &key, &key_len, &value, &value_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    client->SetVar(key, value);

    RETURN_NULL();
}
/* }}} */

/* {{{ proto null P4::set_evar(string key, string value)
    Set a extended var key/value for the Perforce connection. */
PHP_METHOD(P4, set_evar)
{
    char *key, *value;
    size_t key_len, value_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"ss",
            &key, &key_len, &value, &value_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    client->SetEVar(key, value);

    RETURN_NULL();
}
/* }}} */

/* {{{ proto null P4::set_trace(string file, string level)
    Enables debug trace logging for the Perforce connection. */
PHP_METHOD(P4, set_trace)
{
    char *file, *level;
    size_t file_len, level_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"ss",
            &file, &file_len, &level, &level_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    client->SetTrace(file, level);

    RETURN_NULL();
}
/* }}} */


/* {{{ proto string P4::env(string var)
    Get a extended value from the environment following the Perforce conventions,
    including honouring P4CONFIG files, etc. */
PHP_METHOD(P4, get_evar)
{
    char *var;
    size_t var_len;
    PHPClientAPI *client;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, (char *)"s",
            &var, &var_len) == FAILURE) {
        RETURN_NULL();
    }

    client = get_client_api(getThis());
    RETURN_STRING(client->GetEVar(var));
}
/* }}} */

/* Helper function. Extract first element from the array return_value and
 * modify return_value to point to it. If return_value is not an array or
 * has no elements, this function will return immediately, leaving return_value
 * unmodified.
 */
static void p4php_promote_first_element(zval *return_value)
{
    zval tmp, *e;

    // not an array, nothing to do!
    if (Z_TYPE_P(return_value) != IS_ARRAY)
        return;

    // empty array, nothing to do!
    if (zend_hash_num_elements(Z_ARRVAL_P(return_value)) <= 0)
        return;

    // get the first element from return_value
    e = zend_hash_index_find(Z_ARRVAL_P(return_value), 0);

    ZVAL_DUP(&tmp, e);
    zval_dtor(return_value);

    // now assign the value of the temp zval to return_value
    *return_value = tmp;
}

/* Helper function for fetch_ magic methods. */
static void p4php_fetch_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS)
{
    zval *params;

    HashTable *php_args = Z_ARRVAL_P(args);
    int php_argc = zend_hash_num_elements(php_args);

    params = (zval *)safe_emalloc(php_argc + 2, sizeof(zval), 0);

    // params[0] will always be the spectype
    ZVAL_STRING(&params[0], spectype);

    // params[1] will always be the -o flag
    ZVAL_STRING(&params[1], "-o");

    // set the rest of the params
    int i = 2;
    zval *e;

    ZEND_HASH_FOREACH_VAL(php_args, e)
    {
        convert_to_string(e);
        ZVAL_STRINGL(&params[i], Z_STRVAL_P(e), Z_STRLEN_P(e));
        i++;
    }
    ZEND_HASH_FOREACH_END();

    call_user_function(NULL, getThis(), &func, return_value, php_argc + 2,
        params TSRMLS_CC);

    // clean up params
    zval_dtor(&params[0]);
    zval_dtor(&params[1]);
    for (int i = 0; i < php_argc; i++) {
        zval_dtor(&params[i + 2]);
    }
    efree(params);

    p4php_promote_first_element(return_value);
}

/* Helper function for delete_ magic methods. */
static void p4php_delete_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS)
{
    zval *params;

    HashTable *php_args = Z_ARRVAL_P(args);
    int php_argc = zend_hash_num_elements(php_args);

    params = (zval *)safe_emalloc(php_argc + 2, sizeof(zval), 0);

    // params[0] will always be the spectype
    ZVAL_STRING(&params[0], spectype);
    // params[1] will always be the -d flag
    ZVAL_STRING(&params[1], (char *)"-d");

    // set the rest of the params
    int i = 2;
    zval *e;

    ZEND_HASH_FOREACH_VAL(php_args, e)
    {
        convert_to_string(e);
        ZVAL_STRINGL(&params[i], Z_STRVAL_P(e), Z_STRLEN_P(e));
        i++;
    }
    ZEND_HASH_FOREACH_END();

    call_user_function(NULL, getThis(), &func, return_value, php_argc + 2, params TSRMLS_CC);

    // clean up params
    zval_dtor(&params[0]);
    zval_dtor(&params[1]);
    for (int i = 0; i < php_argc; i++) {
        zval_dtor(&params[i + 2]);
    }

    efree(params);
}

/* Helper function for parse_ and format_ magic methods. */
static void p4php_parse_or_format_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS)
{
    zval params[2], *e;
    zval param0;

    if (zend_hash_num_elements(Z_ARRVAL_P(args)) <= 0) {
        WRONG_PARAM_COUNT;
    }

    // params[0] will always be the spec type
    ZVAL_STRING(&param0, spectype);
    params[0] = param0;

    // obtain the first (and only) argument, the hash or string to format
    e = zend_hash_index_find(Z_ARRVAL_P(args), 0);
    params[1] = *e;

    call_user_function(NULL, getThis(), &func, return_value, 2, params TSRMLS_CC);

    zval_dtor(&param0);
    // TODO: what about the other parameter?
}

/* Helper function for run_ magic methods. */
static void p4php_run_cmd(char *cmd, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS)
{
    zval *params;

    HashTable *php_args = Z_ARRVAL_P(args);
    int php_argc = zend_hash_num_elements(php_args);
    params = (zval *)safe_emalloc(php_argc + 1, sizeof(zval), 0);

    ZVAL_STRING(&params[0], cmd);

    int i = 1;
    zval *e;

    ZEND_HASH_FOREACH_VAL(php_args, e)
    {
        convert_to_string(e);
        ZVAL_STRINGL(&params[i], Z_STRVAL_P(e), Z_STRLEN_P(e));
        i++;
    }
    ZEND_HASH_FOREACH_END();

    call_user_function(NULL, getThis(), &func, return_value, php_argc + 1, params TSRMLS_CC);

    zval_dtor(&params[0]);
    for (int i = 0; i < php_argc; i++) {
        zval_dtor(&params[i+1]);
    }
    efree(params);
}

/* Helper function for save_ magic methods. */
static void p4php_save_spec(char *spectype, zval func, zval *args, INTERNAL_FUNCTION_PARAMETERS)
{
    zval *params;

    HashTable *php_args = Z_ARRVAL_P(args);
    int php_argc = zend_hash_num_elements(php_args);

    if (php_argc < 1) {
        WRONG_PARAM_COUNT;
    }

    // TODO: why not stack based allocated like all the other functions?
    params = (zval *)safe_emalloc(2, sizeof(zval), 0);

    // params[0] will always be the spectype
    ZVAL_STRING(&params[0], spectype);
    // params[1] will always be '-i'
    ZVAL_STRING(&params[1], "-i");

    zval *hash_or_spec;

    if ( (hash_or_spec = zend_hash_index_find(php_args, 0)) != NULL) {
        PHPClientAPI *client = get_client_api(getThis());
        client->SetInput(hash_or_spec);
    }

    call_user_function(NULL, getThis(), &func, return_value, 2, params TSRMLS_CC);

    // clean up params
    zval_dtor(&params[0]);
    zval_dtor(&params[1]);
    efree(params);
}

/* Create and return an instance of the class referenced by the provided
 * zend_class_entry. */
static zval p4php_create_p4_object(zend_class_entry *ce)
{
    zval obj, tmp;
    zval *params = 0;
    zval func;

    TSRMLS_FETCH();

    if (object_init_ex(&obj, ce) != SUCCESS) {
        php_error(E_WARNING, "Couldn't create instance.", 1);
    }

    ZVAL_STRING(&func, "__construct");
    call_user_function(NULL, &obj, &func, &tmp, 0, params TSRMLS_CC);

    zval_dtor(&func);
    return obj;
}

/* Debugging function. Recursively print a zend value. */
void p4php_print_r(zval *arg)
{
    zval f, p[1], tmp;

    TSRMLS_FETCH();

    p[0] = *arg;
    ZVAL_STRING( &f, "print_r");
    call_user_function( EG(function_table), NULL, &f, &tmp, 1, p TSRMLS_CC );

    zval_dtor(&f);
}

/* Convenience function. Wraps the call_user_function call to run filelog. */
static zval p4php_run_filelog(char *filespec, zval *this_ptr)
{
    zval func;
    zval params[2];
    zval results;
    zval param0, param1;

    ZVAL_STRING(&func, (char *)"run");
    ZVAL_STRING(&param0, (char *)"filelog");
    ZVAL_STRING(&param1, filespec);
    params[0] = param0;
    params[1] = param1;

    TSRMLS_FETCH();

    // call run.
    call_user_function(NULL, this_ptr, &func, &results, 2, params TSRMLS_CC);

    zval_dtor(&func);
    zval_dtor(&param0);
    zval_dtor(&param1);
    return results;
}

/* Convenience function. Create the appropriate number of P4_Revision
 * objects for the given hash. */
static zval p4php_create_revision_objects(zval *hash)
{
    // extract the rev element. it should be an array and the length
    // will be the number of objects we will need to create.
    zval *revs;
    if ( (revs = zend_hash_str_find(Z_ARRVAL_P(hash), "rev", strlen("rev"))) == NULL) {
        php_error(E_ERROR, "Could not create revision object.", 1);
    }
    if (Z_TYPE_P(revs) != IS_ARRAY) {
        php_error(E_ERROR, "Could not create revision object.", 1);
    }
    zend_class_entry *ce = get_p4_revision_ce();
    int nrevs = zend_hash_num_elements(Z_ARRVAL_P(revs));

    // create P4_Revision instances.
    zval revisions;
    array_init(&revisions);
    for (int i = 0; i < nrevs; i++) {
	zval object = p4php_create_p4_object(ce);
        add_next_index_zval(&revisions, &object);
    }
    return revisions;
}

/* Convenience function. Create the appropriate number of P4_Integrate
 * objects for the given hash and index representing a revision. */
static zval p4php_create_integration_objects(zval *hash, int i)
{
    // extract the how element. it should be an array and the length
    // will be the number of objects we will need to create.
    zval *how;
    zval integrations;

    // set to NULL to begin with
    ZVAL_NULL(&integrations);

    if (Z_TYPE_P(hash) != IS_ARRAY) {
        return integrations;
    }
    if ( (how = zend_hash_str_find(Z_ARRVAL_P(hash), "how", strlen("how"))) == NULL) {
        // no integrations, so just return
        return integrations;
    }
    if (Z_TYPE_P(how) != IS_ARRAY) {
        // no integrations, so just return
        return integrations;
    }

    // 'how' will be an array of arrays. extract the array that
    // corresponds to the revision we want to work with.
    zval *integration;
    if ( (integration = zend_hash_index_find(Z_ARRVAL_P(how), i)) == NULL)
    {
        // error - could not find an integration that corresponds
        // to the revision we're working with.
        return integrations;
    }

    if (Z_ISNULL_P(integration)) {
        // no integrations for this revision
        return integrations;
    }

    zend_class_entry *ce = get_p4_integration_ce();
    int nintegs = zend_hash_num_elements(Z_ARRVAL_P(integration));

    // create P4_Integration instances.
    array_init(&integrations);
    for (int j = 0; j < nintegs; j++) {
	zval object = p4php_create_p4_object(ce);
        add_next_index_zval(&integrations, &object);
    }
    return integrations;
}

/* Convenience function. Determine whether output is tagged or not. */
static bool p4php_client_is_tagged(zval *this_ptr)
{
    // determine whether we want tagged or untagged results.
    zval tagged;
    PHPClientAPI *client = get_client_api(this_ptr);
    client->GetTagged(&tagged);
    return ( Z_TYPE(tagged) == IS_TRUE );
}
