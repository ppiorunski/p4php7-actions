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

#ifndef PHP_P4_MAP_H
#define PHP_P4_MAP_H

#include "php_p4mapmaker.h"

zend_class_entry *get_p4_map_ce(void);

PHP_METHOD(P4_Map, __construct);
PHP_METHOD(P4_Map, join);
PHP_METHOD(P4_Map, clear);
PHP_METHOD(P4_Map, count);
PHP_METHOD(P4_Map, is_empty);
PHP_METHOD(P4_Map, insert);
PHP_METHOD(P4_Map, translate);
PHP_METHOD(P4_Map, includes);
PHP_METHOD(P4_Map, reverse);
PHP_METHOD(P4_Map, lhs);
PHP_METHOD(P4_Map, rhs);
PHP_METHOD(P4_Map, as_array);
PHP_METHOD(P4_Map, set_case_sensitive);

void register_p4_map_class(INIT_FUNC_ARGS);

//
// Internal representation of a P4_Map object.
//

struct p4map_object {
    P4MapMaker *mapmaker;
    zend_object std;
};

static inline struct p4map_object * php_p4map_object_fetch_object(zend_object *obj) {
      return (struct p4map_object *)((char *)obj - XtOffsetOf(struct p4map_object, std));
}

#define Z_P4MAP_OBJ_P(zv) php_p4map_object_fetch_object(Z_OBJ_P(zv));

#endif /* PHP_P4_MAP_H */
