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

#ifndef PHP_P4_MERGEDATA_H
#define PHP_P4_MERGEDATA_H 

zend_class_entry *get_p4_mergedata_ce(void);

PHP_METHOD(P4_MergeData, __construct);
PHP_METHOD(P4_MergeData, __get);
PHP_METHOD(P4_MergeData, run_merge);

void register_p4_mergedata_class(INIT_FUNC_ARGS);

//
// Properties table
//

typedef void (PHPMergeData::*method_t) (zval *);

struct merge_property_t {
    const char *property;
    method_t method;
};

// 
// Internal representation of a P4_MergeData object. 
// 

struct p4_mergedata_object {
    zend_object std;
    PHPMergeData *mergedata;
};

static inline struct p4_mergedata_object * php_p4_mergedata_object_fetch_object(zend_object *obj) {
      return (struct p4_mergedata_object *)((char *)obj - XtOffsetOf(struct p4_mergedata_object, std));
}

#define Z_P4_MERGEDATA_OBJ_P(zv) php_p4_mergedata_object_fetch_object(Z_OBJ_P(zv));

#endif /* PHP_P4_MERGEDATA_H */
