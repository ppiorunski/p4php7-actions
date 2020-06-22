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

#include "php_p4_map.h"


zend_class_entry *p4_map_ce;
static zend_object_handlers p4map_object_handlers;

/* P4_Map Class Methods */
static zend_function_entry perforce_p4_map_methods[] = {
    PHP_ME(P4_Map, __construct,  NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(P4_Map, join,         NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    PHP_ME(P4_Map, clear,        NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, count,        NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, is_empty,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, insert,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, translate,    NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, includes,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, reverse,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, lhs,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, rhs,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, as_array,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(P4_Map, set_case_sensitive, NULL, ZEND_ACC_PUBLIC)
    { NULL, NULL, NULL }
};

/* Get the zend_class_entry for P4_Map. */
zend_class_entry *get_p4_map_ce(void)
{
    return p4_map_ce;
}

static void p4_map_object_destroy(zend_object *object TSRMLS_DC)
{
    p4map_object *obj = php_p4map_object_fetch_object(object);
    zend_objects_destroy_object(object);
}

/* free storage allocated for P4_Map class. */
void p4_map_object_free_storage(zend_object *object TSRMLS_DC)
{
    p4map_object *obj = php_p4map_object_fetch_object(object);
    delete obj->mapmaker;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

/* create_object handler for P4_Map class. */
zend_object * p4_map_create_object(zend_class_entry *type TSRMLS_DC)
{
    p4map_object *obj = (p4map_object *) ecalloc(1, sizeof(struct p4map_object) + zend_object_properties_size(type));

    zend_object_std_init(&obj->std, type TSRMLS_CC);

    obj->std.handlers = &p4map_object_handlers;

    return &obj->std;
}

/* Register the P4_Map Class. */

void register_p4_map_class(INIT_FUNC_ARGS)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "P4_Map", perforce_p4_map_methods);
    p4_map_ce = zend_register_internal_class(&ce TSRMLS_CC);
    p4_map_ce->create_object = p4_map_create_object;
    memcpy(&p4map_object_handlers, 
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));

    p4map_object_handlers.clone_obj = NULL;
    p4map_object_handlers.offset = XtOffsetOf(struct p4map_object, std);
    p4map_object_handlers.free_obj = p4_map_object_free_storage;
    p4map_object_handlers.dtor_obj = p4_map_object_destroy;

}

/* Convenience method. Get the internal class associated with the PHP class. */
P4MapMaker *get_map_maker(zval *this_ptr)
{
    P4MapMaker *mapmaker;
    TSRMLS_FETCH();
    p4map_object *obj = Z_P4MAP_OBJ_P(this_ptr);
    mapmaker = obj->mapmaker;
    return mapmaker;
}

/* {{{ proto void P4_Map::__construct()
    Create a new P4_Map object instance. */
PHP_METHOD(P4_Map, __construct)
{
    zval *arg1 = NULL;
    zval *arg2 = NULL;
    P4MapMaker *mapmaker = NULL;
    zval *object = getThis();

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
            "|zz", &arg1, &arg2) == FAILURE) {
        RETURN_NULL();
    }

    mapmaker = new P4MapMaker;

    if (ZEND_NUM_ARGS() == 1 && arg1 != NULL) {
        if (Z_TYPE_P(arg1) == IS_ARRAY) {
            zval *e;
            HashTable *ht = Z_ARRVAL_P(arg1);
            ZEND_HASH_FOREACH_VAL(ht, e)
            {
                if (Z_TYPE_P(e) == IS_STRING) {
                    mapmaker->Insert(e);
                }
            }
            ZEND_HASH_FOREACH_END();
        } else if (Z_TYPE_P(arg1) == IS_STRING) {
            mapmaker->Insert(arg1);
        }
    } else if (ZEND_NUM_ARGS() == 2 && arg1 != NULL && arg2 != NULL) {
        if (Z_TYPE_P(arg1) == IS_STRING && Z_TYPE_P(arg2) == IS_STRING) {
            mapmaker->Insert(arg1, arg2);
        }
    }

    p4map_object *obj = Z_P4MAP_OBJ_P(object);
    obj->mapmaker = mapmaker;
}
/* }}} */

/* {{{ proto string P4_Map::join(string map1, string map2)
 *  Class method. Joins two maps to create a third. */
PHP_METHOD(P4_Map, join)
{
    zval *map1, *map2;
    P4MapMaker *mapmaker = NULL;
    P4MapMaker *retval   = NULL;

    if (ZEND_NUM_ARGS() != 2) {
        WRONG_PARAM_COUNT;
    }

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
            "zz", &map1, &map2) == FAILURE) {
        RETURN_NULL();
    }

    mapmaker = new P4MapMaker; 
    
    P4MapMaker *map_one = get_map_maker(map1);
    P4MapMaker *map_two = get_map_maker(map2);
    
    retval = mapmaker->Join(map_one, map_two);

    // Create a P4_Map instance 
    zend_class_entry *ce;
    if ( (ce = (zend_class_entry *) zend_hash_str_find_ptr(EG(class_table), "p4_map", strlen("p4_map"))) == NULL) {
        RETURN_NULL();
    }
    if (object_init_ex(return_value, ce) != SUCCESS) {
        php_error(E_WARNING, "Couldn't create P4_Map instance.", 1);
    }

    // call constructor
    zval func;
    zval dummy;
    ZVAL_STRING(&func, (char *)"__construct");
    call_user_function(NULL, return_value, &func, &dummy, 0, NULL TSRMLS_CC);

    // replace P4MapMaker instance 
    p4map_object *obj = Z_P4MAP_OBJ_P(return_value);
    obj->mapmaker = retval;

    zval_dtor(&func);
}
/* }}} */

/* {{{ proto void P4_Map::clear()
 *  Empties the map. */
PHP_METHOD(P4_Map, clear)
{
    P4MapMaker *mapmaker = get_map_maker(getThis());

    if (mapmaker != NULL) {
        mapmaker->Clear();
    }
}
/* }}} */

/* {{{ proto int P4_Map::count()
 *  Returns the number of entries in the map. */
PHP_METHOD(P4_Map, count)
{
    int count = 0;

    P4MapMaker *mapmaker = get_map_maker(getThis());

    if (mapmaker != NULL) {
        count = mapmaker->Count();
    }

    RETURN_LONG(count);        
}
/* }}} */

/* {{{ proto bool P4_Map::is_empty()
 *  Returns True if the map is empty. */
PHP_METHOD(P4_Map, is_empty)
{
    int count = 0;
    P4MapMaker *mapmaker = get_map_maker(getThis());
    if (mapmaker != NULL) {
        count = mapmaker->Count();
        if (count > 0) {
            RETURN_TRUE;
        }
    }
    RETURN_FALSE;
}
/* }}} */

/* {{{ proto void P4_Map::insert(string entry)
 *  Adds new entries to the map. */
PHP_METHOD(P4_Map, insert)
{
    zval *l, *r;
    P4MapMaker *mapmaker = get_map_maker(getThis());

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", 
            &l, &r) == FAILURE) {
        RETURN_NULL();
    }
    if (mapmaker != NULL) {
        if (ZEND_NUM_ARGS() == 1) {
            mapmaker->Insert(l);
        } 
        else 
        if (ZEND_NUM_ARGS() == 2) {
            mapmaker->Insert(l, r);
        }
        else {
            WRONG_PARAM_COUNT;
        }
    }
}
/* }}} */

/* {{{ proto string P4_Map::translate(string str, bool forward)
 *  Translates a string through the map. */
PHP_METHOD(P4_Map, translate)
{
    zend_bool fwd = 1;
    zval *str;
    P4MapMaker *mapmaker = get_map_maker(getThis());

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b",     
            &str, &fwd) == FAILURE) {
        RETURN_NULL();
    }

    if (Z_TYPE_P(str) != IS_STRING) {
        RETURN_NULL();
    }

    zend_string *ret = mapmaker->Translate(str, fwd);

    if (ret) {
        RETURN_STR(ret);
    } else {
        RETVAL_NULL();
    }
}
/* }}} */

/* {{{ proto bool P4_Map::includes(string str) 
 *  Returns True if the supplied string is visible through the map. */
PHP_METHOD(P4_Map, includes)
{
    char *path;
    size_t path_len;
    zval zpath;
    P4MapMaker *mapmaker;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", 
            &path, &path_len) == FAILURE) {
        RETURN_NULL();
    }

    ZVAL_STRING(&zpath, path);

    mapmaker = get_map_maker(getThis());
    zend_string * result = mapmaker->Translate(&zpath, 1);

    zval_dtor(&zpath);

    if (result == NULL) {
        RETVAL_FALSE;
    } else {
        zend_string_release(result);
        RETVAL_TRUE;
    }
}
/* }}} */

/* {{{ proto object P4_Map::reverse()
 *  Returns a new map with reversed mappings. */
PHP_METHOD(P4_Map, reverse)
{
    zend_class_entry *ce;
    P4MapMaker *mapmaker; 
    P4MapMaker *mapmaker_cp;
    
    // first make a copy of this instance's mapmaker
    mapmaker = get_map_maker(getThis());
    mapmaker_cp = new P4MapMaker(*mapmaker);
    
    // then reverse the copy
    mapmaker_cp->Reverse();

    // now, create new P4_Map instance
    if ( (ce = (zend_class_entry *) zend_hash_str_find_ptr(EG(class_table), "p4_map", strlen("p4_map"))) == NULL) {
        RETURN_NULL();
    }
    object_init_ex(return_value, ce);
    
    // call constructor on new P4_Map instance
    zval func;
    zval dummy;
    ZVAL_STRING(&func, (char *)"__construct");
    call_user_function(NULL, return_value, &func, &dummy, 0, NULL TSRMLS_CC);
    
    // create and assign new mapmaker
    p4map_object *obj = Z_P4MAP_OBJ_P(return_value);
    obj->mapmaker = mapmaker_cp;
    zval_dtor(&func);
}

/* }}} */

/* {{{ proto array P4_Map::lhs()
 *  Returns the left-hand-side of the map. */
PHP_METHOD(P4_Map, lhs)
{
    P4MapMaker *mapmaker = get_map_maker(getThis());
    mapmaker->Lhs(return_value);
}
/* }}} */

/* {{{ proto array P4_Map::rhs() 
 *  Returns the right-hand-side of the map. */
PHP_METHOD(P4_Map, rhs)
{
    P4MapMaker *mapmaker = get_map_maker(getThis());
    mapmaker->Rhs(return_value);
}
/* }}} */

/* {{{ proto array P4_Map::as_array()
 *  Returns the map as an array. */
PHP_METHOD(P4_Map, as_array)
{
    P4MapMaker *mapmaker = get_map_maker(getThis());
    mapmaker->ToA(return_value);
}
/* }}} */


/* {{{ proto void P4_Map::set_case_sensitive(bool sensitive)
 *  Sets the case-sensitivity for the map. */
PHP_METHOD(P4_Map, set_case_sensitive)
{
    zend_bool sensitive = 1;
    P4MapMaker *mapmaker = get_map_maker(getThis());

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b",
            &sensitive) == FAILURE) {
        RETURN_NULL();
    }

    if (mapmaker != NULL) {
        mapmaker->SetCaseSensitivity(sensitive);
    }
}
/* }}} */

