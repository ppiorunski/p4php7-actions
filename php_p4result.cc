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
#include "Zend/zend_exceptions.h"

#include "php_macros.h"
#include "php_p4result.h"

#include "php_p4_exception.h"

#include <iostream>

/* Create a new P4Result instance. Call Reset whenever creating an instance. */
P4Result::P4Result()
{   
    ZVAL_NULL(&output);
    ZVAL_NULL(&errors);
    ZVAL_NULL(&warnings);
    Reset();
}

/* Destroy P4Result instance. */
P4Result::~P4Result()
{    
    if ( Z_TYPE(output) == IS_ARRAY ) {
        zval_dtor(&output);
        ZVAL_NULL(&output);
    }
    if ( Z_TYPE(errors) == IS_ARRAY ) {
        zval_dtor(&errors);
        ZVAL_NULL(&errors);
    }
    if ( Z_TYPE(warnings) == IS_ARRAY ) {
        zval_dtor(&warnings);
        ZVAL_NULL(&warnings);
    }
}

void 
P4Result::GetOutput( zval *retval )
{
    *retval = output;
    ZVAL_DUP(retval, &output);
}

void 
P4Result::GetWarnings( zval *retval )
{
    ZVAL_DUP(retval, &warnings);
}

void 
P4Result::GetErrors( zval *retval )
{
    ZVAL_DUP(retval, &errors);
}

//
// Reset P4Result instance. Clear error, warning and output buffers. 
//

void 
P4Result::Reset()
{
    if ( Z_TYPE(output) == IS_ARRAY ) {
        zval_dtor(&output);
        ZVAL_NULL(&output);
    }
    if ( Z_TYPE(errors) == IS_ARRAY ) {
        zval_dtor(&errors);
        ZVAL_NULL(&errors);
    }
    if ( Z_TYPE(warnings) == IS_ARRAY ) {
        zval_dtor(&warnings);
        ZVAL_NULL(&warnings);
    }
    
    array_init( &warnings );

    array_init( &output );

    array_init( &errors);
}

void 
P4Result::AppendString( zval *list, const char *str )
{
    if (add_next_index_string(list, (char *)str) == FAILURE) {
        StrBuf m;
        TSRMLS_FETCH();
        m << "P4Result::AppendString() - Error adding string to list.";
        zend_throw_exception_ex( get_p4_exception_ce(), 0 TSRMLS_CC, m.Text() );
    }
}

void 
P4Result::AddOutput( const char *msg )
{
    AppendString(&output, msg);
}

void 
P4Result::AddOutput( zval *out )
{
    add_next_index_zval(&output, out);
}

//
// Add an error to the error or warning buffer depending on severity. 
//

void 
P4Result::AddError(Error *e)
{
    StrBuf  m;
    e->Fmt( &m, EF_PLAIN );

    int s;
    s = e->GetSeverity();

    /* Empty and informational messages are pushed out as output as nothing
     * worthy of error handling has occurred. Warnings go into the warnings
     * list and the rest are lumped together as errors. 
     */
    if ( s == E_EMPTY || s == E_INFO )
    {
        AddOutput( m.Text() );
        return;
    }

    if ( s == E_WARN ) 
        AppendString( &warnings, m.Text() );
    else
        AppendString( &errors, m.Text() );

}

// 
// Return the number of errors that have been encountered. 
//

int 
P4Result::ErrorCount()
{
    return zend_hash_num_elements( Z_ARRVAL(errors) );
}

// 
// Return the number of warnings that have been encountered. 
//

int 
P4Result::WarningCount()
{
    return zend_hash_num_elements( Z_ARRVAL(warnings) );
}

void 
P4Result::FmtErrors( StrBuf &buf ) 
{
    Fmt( "[Error]: ", errors, buf );
}

void 
P4Result::FmtWarnings( StrBuf &buf )
{
    Fmt( "[Warning]: ", warnings, buf );
}

int 
P4Result::Length( zval *list )
{
    return zend_hash_num_elements( Z_ARRVAL_P(list) );
}

void 
P4Result::Fmt( const char *label, zval& list, StrBuf &buf )
{
    HashTable *ht;

    ht = Z_ARRVAL(list);
    buf.Clear();
    // If the array is empty, then we just return
    if( zend_hash_num_elements( ht ) == 0 ) return;

    // This is the string we'll use to prefix each entry in the array
    StrBuf csep;

    csep << "\n\t" << label;

    // Join the array elements together, using our separator as glue. 
    // Append the result to the buffer. 
    int size;

    size = zend_hash_num_elements(ht);
    if ( size > 0 )
        buf << csep;

    int i = 0;
    zval* val;

    ZEND_HASH_FOREACH_VAL(ht, val) {
        convert_to_string(val);
        buf << Z_STRVAL_P(val);
        if ( i < (size - 1) )
            buf << csep;
        i++;
    }
    ZEND_HASH_FOREACH_END();
}
