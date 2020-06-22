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
#include "spec.h"
#include "diff.h"
#include "strtable.h"

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}
#include "php_macros.h"
#include "specmgr.h"
#include "php_p4result.h"
#include "php_clientuser.h"
#include "php_p4_resolver.h"
#include "php_p4_output_handler.h"
#include "php_mergedata.h"
#include "php_p4_mergedata.h"

#include <iostream>
#include <iomanip>

PHPClientUser::PHPClientUser( SpecMgr *s )
{
    specMgr = s;
    debug   = 0;
    SetSSOHandler( ssoHandler = new PHPClientSSO( s ) );

    ZVAL_NULL(&input);
    ZVAL_NULL(&resolver);
    ZVAL_NULL(&handler);

    Reset();
}

PHPClientUser::~PHPClientUser()
{
    // I don't think we have to do any memory deallocation
    // We might have to decrease the reference count
    // Let's assume NULL is handled correctly, or this will blow up

    if ( Z_TYPE(input) != IS_NULL ) {
	zval_dtor(&input);
        ZVAL_NULL(&input);
    }
    if ( Z_TYPE(resolver) != IS_NULL ) {
	zval_dtor(&resolver);
        ZVAL_NULL(&resolver);
    }
    if ( Z_TYPE(handler) != IS_NULL ) {
	zval_dtor(&handler);
        ZVAL_NULL(&handler);
    }
}

void PHPClientUser::Reset()
{
    alive = 1;

    results.Reset();
    // input data is untouched
}

void PHPClientUser::Finished()
{
    if ( PHP_PERFORCE_DEBUG_CALLS && ! Z_ISNULL( input )  )
        std::cerr << "[P4] Cleaning up saved input" << std::endl;

    zval_dtor(&input);
    ZVAL_NULL(&input);
}

void PHPClientUser::HandleError( Error *e )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] HandleError()" << std::endl;

    if ( PHP_PERFORCE_DEBUG_DATA )
    {
        StrBuf t;
        e->Fmt( t, EF_PLAIN );
        std::cerr << "... [" << e->FmtSeverity() << "] " << t.Text() << std::endl;
    }

    // track if we should add output to result
    // default to true for the case of no handler
    bool report = true;

    // if we have a handler; determine the output method
    // to use and track whether or not we need to add to
    // the result object.
    if (! Z_ISNULL(handler))
    {
        StrBuf m;
        zval str;

        e->Fmt( &m, EF_PLAIN );
        ZVAL_STRINGL(&str, (char *)m.Text(), m.Length());

        switch (e->GetSeverity())
        {
            case E_EMPTY:
            case E_INFO:
                report = CallOutputMethod("outputInfo",    &str);
                break;
            case E_WARN:
                report = CallOutputMethod("outputWarning", &str);
                break;
            default:
                report = CallOutputMethod("outputError",   &str);
        }
    }

    if (report)
    {
        results.AddError( e );
    }
}

bool PHPClientUser::CallOutputMethod( const char * method, zval *data)
{
    zval func, result;
    int  answer = HANDLER_REPORT;

    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] CallOutputMethod()" << std::endl;

    TSRMLS_FETCH();

    ZVAL_STRING(&func, method);
    call_user_function(NULL, &handler, &func, &result, 1, data TSRMLS_CC);

    convert_to_long(&result);

    if ( Z_LVAL(result) & HANDLER_CANCEL) {
        alive = 0;
    }

    answer = Z_LVAL(result) & HANDLER_HANDLED;

    return ( answer == 0 );
}

void PHPClientUser::ProcessOutput( const char * method, zval *data)
{
    if ( Z_ISNULL(handler) || CallOutputMethod( method, data ) )
    {
        results.AddOutput(data);
    } else {
	// TODO: this does not look right
        zval_ptr_dtor(data);
    }
}

void PHPClientUser::OutputText( const char *data, int length )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] OutputText()" << std::endl;
    if ( PHP_PERFORCE_DEBUG_DATA )
        std::cerr << "... [" << length << "]" << std::setw(length) << data << std::endl;

    zval str;
    ZVAL_STRINGL(&str, (char *)data, length);

    ProcessOutput("outputText", &str);
}

void PHPClientUser::OutputInfo( char level, const char *data )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] OutputInfo()" << std::endl;
    if ( PHP_PERFORCE_DEBUG_DATA )
        std::cerr << "... [" << level << "] " << data << std::endl;

    zval str;
    ZVAL_STRING(&str, (char *)data);

    ProcessOutput("outputInfo", &str);
}

void PHPClientUser::OutputBinary( const char *data, int length )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] OutputBinary()" << std::endl;
    //
    // Binary is just stored in a string. Since the char * version of
    // P4Result::AddOutput() assumes it can strlen() to find the length,
    // we'll make the string object here.
    //
    zval tmp;
    ZVAL_STRINGL(&tmp, (char *)data, length);

    ProcessOutput("outputBinary", &tmp);
}

void PHPClientUser::OutputStat( StrDict *values )
{
    StrPtr  *spec = values->GetVar( "specdef" );
    StrPtr  *data = values->GetVar( "data" );
    StrPtr  *sf   = values->GetVar( "specFormatted" );
    StrDict *dict = values;

    SpecDataTable specData;
    Error         e;

    //
    // Determine whether or not the data we've got contains a spec in one form
    // or another. 2000.1 -> 2005.1 servers supplied the form in a data variable
    // and we use the spec variable to parse the form. 2005.2 and later servers
    // supply the spec ready-parsed but set the 'specFormatted' variable to tell
    // the client what's going on. Either way, we need the specdef variable set
    // to enable spec parsing.
    //
    int   isspec  = spec && ( sf || data );

    // Save the spec definition for later
    if ( spec )
        specMgr->AddSpecDef( cmd.Text(), spec->Text() );

    // Parse any form supplied in the 'data' variable and convert it into a
    // dictionary.
    if ( spec && data )
    {
        // 2000.1 -> 2005.1 server's handle tagged form output by supplying the form
        // as text in the 'data' variable. We need to convert it to a dictionary
        // using the supplied spec.
        if ( PHP_PERFORCE_DEBUG_CALLS )
            std::cerr << "[P4] OutputStat() - parsing form" << std::endl;

        // Parse the form. Use the ParseNoValid() interface to prevent
        // errors caused by the use of invalid defaults for select items in
        // jobspecs.
        Spec s( spec->Text(), "", &e );

        if ( !e.Test() )
            s.ParseNoValid( data->Text(), &specData, &e );

        if ( e.Test() )
        {
            HandleError( &e );
            return;
        }

        dict = specData.Dict();
    }

    // If what we've got is a parsed form, then we'll convert it to a P4::Spec
    // object. Otherwise it's a plain dict.
    if ( isspec )
    {
        if ( PHP_PERFORCE_DEBUG_CALLS )
            std::cerr << "[P4] OutputStat() - Converting to P4::Spec object" << std::endl;

	zval retval;
	specMgr->StrDictToSpec( dict, spec, &retval );

        ProcessOutput("outputStat",  &retval);

    } else {
        if ( PHP_PERFORCE_DEBUG_CALLS )
            std::cerr << "[P4] OutputStat() - Converting to hash" << std::endl;

        zval retval;
        specMgr->StrDictToHash( dict, &retval );

        ProcessOutput("outputStat", &retval );
    }
}

//
// Diff support for PHP API. Since the Diff class only writes its output
// to files, we run the requested diff putting the output into a temporary
// file. Then we read the file in and add its contents line by line to the
// results.
//

void PHPClientUser::Diff( FileSys *f1, FileSys *f2, int doPage,
                char *diffFlags, Error *e )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] Diff() - comparing files" << std::endl;

    //
    // Duck binary files. Much the same as ClientUser::Diff, we just
    // put the output into PHP space rather than stdout.
    //
    if ( !f1->IsTextual() || !f2->IsTextual() )
    {
        if ( f1->Compare( f2, e ) )
            results.AddOutput( "(... files differ ...)" );
        return;
    }

    // Time to diff the two text files. Need to ensure that the
    // files are in binary mode, so we have to create new FileSys
    // objects to do this.

    FileSys *f1_bin = FileSys::Create( FST_BINARY );
    FileSys *f2_bin = FileSys::Create( FST_BINARY );
    FileSys *t = FileSys::CreateGlobalTemp( f1->GetType() );

    f1_bin->Set( f1->Name() );
    f2_bin->Set( f2->Name() );

    {
        //
        // In its own block to make sure that the diff object is deleted
        // before we delete the FileSys objects.
        //
        ::Diff d;

        d.SetInput( f1_bin, f2_bin, diffFlags, e );
        if ( ! e->Test() ) d.SetOutput( t->Name(), e );
        if ( ! e->Test() ) d.DiffWithFlags( diffFlags );
        d.CloseOutput( e );

        // OK, now we have the diff output, read it in and add it to
        // the output.
        if ( ! e->Test() ) t->Open( FOM_READ, e );
        if ( ! e->Test() )
        {
            StrBuf b;
            while( t->ReadLine( &b, e ) )
                results.AddOutput( b.Text() );
        }
    }

    delete t;
    delete f1_bin;
    delete f2_bin;

    if ( e->Test() ) HandleError( e );
}

//
// Convert input from the user into a form digestible to Perforce. This
// involves either (a) converting any supplied dict / associative array
// to a Perforce form, or (b) converting whatever we were given to a string.
//

void PHPClientUser::InputData( StrBuf *strbuf, Error *e )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] InputData(). Using supplied input" << std::endl;

    if ( Z_TYPE( input ) == IS_ARRAY )
    {
        // determine if input is a hash or a vector.
        HashTable *ht = Z_ARRVAL( input );
        HashPosition pos;

        zend_hash_internal_pointer_reset_ex( ht, &pos );

        zend_string *k;
        zend_ulong index;
        uint ktype = zend_hash_get_current_key_ex( ht, &k, &index, &pos );
        bool isHash = ( ktype == HASH_KEY_IS_STRING );

        // treat a hash as a spec, copy it's string representation to strbuf.
        if ( isHash ) {
            StrPtr *specDef = varList->GetVar( "specdef" );
            specMgr->AddSpecDef( cmd.Text(), specDef->Text() );
            specMgr->SpecToString( cmd.Text(), &input, *strbuf, e );
            return;
        }

        // handle one element of a vector at a time. Note, only strings are
        // supported at this time.
        uint argc = zend_hash_num_elements( ht );
        zval *val = zend_hash_index_find( ht, 0 );

        if ( Z_TYPE_P( val ) == IS_STRING ) {
	    ArraySlice( &input, 1, argc );
	    strbuf->Set( Z_STRVAL_P(val)  );
        }
    }

    // strings are simple, just feed the string into strbuf
    if ( Z_TYPE( input ) == IS_STRING )
    {
        strbuf->Set( Z_STRVAL( input ), Z_STRLEN( input ) );
    }
}

void PHPClientUser::Prompt( const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e )
{
    InputData( &rsp, e );
}

int PHPClientUser::Resolve( ClientMerge *m, Error *e )
{
    TSRMLS_FETCH();

    // if no resolver is defined, default to using the merger's resolve
    // if p4->input is set. otherwise bail out of the resolve.
    if ( Z_TYPE(resolver) == IS_NULL || Z_TYPE( resolver ) != IS_OBJECT ||
         !instanceof_function(Z_OBJCE(resolver), get_p4_resolver_ce() TSRMLS_CC) )
    {
        if ( Z_TYPE( input ) != IS_NULL )
        {
            return m->Resolve( e );
        }
        else {
             php_error( E_WARNING,
                 "P4::resolve() - Resolve called with no resolver and no input -> skipping resolve.", 1 );
             return CMS_QUIT;
        }
    }

    //
    // First detect what the merger thinks the result ought to be
    //
    StrBuf t;
    MergeStatus autoMerge = m->AutoResolve( CMF_FORCE );

    // Now convert that to a string;
    switch( autoMerge )
    {
    case CMS_QUIT:	 t = "q";   break;
    case CMS_SKIP:	 t = "s";   break;
    case CMS_MERGED: t = "am";  break;
    case CMS_EDIT:	 t = "e";   break;
    case CMS_YOURS:	 t = "ay";  break;
    case CMS_THEIRS: t = "at";  break;
    }

    zval mergeData = MkMergeInfo( m, t );

    for( int loop=0 ; loop < 10 ; loop++ )
    {
        // call resolve method on resolver
        TSRMLS_FETCH();

        zval func;
        zval result;

        ZVAL_STRING(&func, "resolve");

        zval params[1];
        params[0] = mergeData;

        if (call_user_function(NULL, &resolver, &func, &result, 1, params TSRMLS_CC) != SUCCESS) {
            php_error( E_WARNING,
                "[P4::Resolve] Could not call resolver::resolve()", 1 );
        }

        zval_dtor(&func);

        if (Z_TYPE(result) != IS_STRING) {
            return CMS_QUIT;
        }

        StrBuf reply = Z_STRVAL( result );

	if( reply == "ay" )         return CMS_YOURS;
	else if( reply == "at" )    return CMS_THEIRS;
	else if( reply == "am" )    return CMS_MERGED;
	else if( reply == "ae" )    return CMS_EDIT;
	else if( reply == "s" )	    return CMS_SKIP;
	else if( reply == "q" )     return CMS_QUIT;
	else {
	    StrBuf warning("[P4::Resolve] Illegal response : '");
            warning << reply;
            warning << "', skipping resolve";

            php_error( E_WARNING, warning.Text(), 1);
            return CMS_QUIT;
        }
    }

    php_error( E_WARNING,
         "P4::resolve() - Aborting resolve after 10 attempts.", 1 );

    return CMS_QUIT;
}

// don't allow action resovle; we don't support it in this version
int PHPClientUser::Resolve( ClientResolveA *m, int preview, Error *e )
{
    php_error( E_ERROR,
        "P4::resolve() - Action resolve is not supported in this version of p4-php.", 1 );

    return CMS_QUIT;
}

zval PHPClientUser::MkMergeInfo( ClientMerge *m, StrPtr &hint )
{
    zval obj; /* P4_MergeData object */
    zval func;
    zval *params = 0;
    zval tmp;
    p4_mergedata_object *merge_obj;

    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] MkMergeInfo()" << std::endl;

    // create a PHP object (P4_MergeData) instance
    TSRMLS_FETCH();
    if (object_init_ex(&obj, get_p4_mergedata_ce()) != SUCCESS)
        php_error(E_WARNING, "Couldn't create P4_MergeData instance.", 1);
    ZVAL_STRING(&func, "__construct");
    call_user_function(NULL, &obj, &func, &tmp, 0, params TSRMLS_CC);
    zval_dtor(&func);

    if (Z_TYPE(obj) != IS_NULL) {
        merge_obj = (p4_mergedata_object *) Z_OBJ(obj);
        merge_obj->mergedata = new PHPMergeData( this, m, hint );
    }
    else {
        php_error( E_WARNING,
            "P4::resolve() - Failed to create object in MkMergeInfo", 1 );
    }

    return obj;
}

bool PHPClientUser::SetInput( zval *i )
{
    // It will probably not happen often, but if you first assign one value and them replace it
    // with another value, you would leave a memory leak

    if ( Z_TYPE(input) != IS_NULL ) {
	zval_dtor(&input);
    }

    if ( Z_TYPE_P( i ) == IS_OBJECT )
    {
        ZVAL_DUP(&input, i);
        return true;
    }

    if ( Z_TYPE_P( i ) == IS_NULL     ||
	 Z_TYPE_P( i ) == IS_LONG     ||
	 Z_TYPE_P( i ) == IS_DOUBLE   ||
	 Z_TYPE_P( i ) == IS_TRUE     ||
	 Z_TYPE_P( i ) == IS_FALSE    ||
	 Z_TYPE_P( i ) == IS_RESOURCE )
    {
        convert_to_string( i );
    }

    if ( Z_TYPE_P( i ) == IS_ARRAY )
    {
	zval tmp;
        array_init( &input );
        zend_hash_copy( Z_ARRVAL( input ), Z_ARRVAL_P( i ), (copy_ctor_func_t)zval_add_ref );
        return true;
    }
    else
    if ( Z_TYPE_P( i ) == IS_STRING )
    {
        ZVAL_STRINGL( &input, Z_STRVAL_P(i), Z_STRLEN_P(i));
        return true;
    }

    return false;
}

void PHPClientUser::GetInput( zval *retval )
{
    ZVAL_COPY(retval, &input);
}

bool PHPClientUser::SetResolver( zval *r )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] SetResolver()" << std::endl;

    TSRMLS_FETCH();

    // It will probably not happen often, but if you first assign one value and them replace it
    // with another value, you would leave a memory leak

    if ( Z_TYPE(resolver) != IS_NULL ) {
	zval_dtor(&resolver);
    }

    // check # 1 - make sure r is an object
    if ( Z_TYPE_P(r) != IS_OBJECT )
        return false;

    // check # 2 - make sure r is an instance of P4_Resolver
    if ( !instanceof_function(Z_OBJCE_P(r), get_p4_resolver_ce() TSRMLS_CC) )
        return false;

    // TODO: should be identical code to SetHandler. Let's revisit after it compiles
    // For example, assignment of NULL is not handled

    ZVAL_DUP(&resolver, r);

    return true;
}

void PHPClientUser::GetResolver( zval *retval )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] GetResolver()" << std::endl;

    ZVAL_COPY(retval, &resolver);
}

bool PHPClientUser::SetHandler( zval *h )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] SetHandler()" << std::endl;

    TSRMLS_FETCH();

    if ( Z_TYPE_P( h ) == IS_OBJECT
        && instanceof_function( Z_OBJCE_P( h ), get_p4_output_handler_interface_ce() TSRMLS_CC))
    {
	zval_dtor(&handler);
        ZVAL_COPY(&handler, h);

        alive = 1;
        return true;
    }
    else if ( Z_TYPE_P( h ) == IS_NULL )
    {
	zval_dtor(&handler);
        ZVAL_NULL(&handler);

        alive = 1;
        return true;
    }

    return false;
}

void PHPClientUser::GetHandler( zval *retval )
{
    if ( PHP_PERFORCE_DEBUG_CALLS )
        std::cerr << "[P4] GetHandler()" << std::endl;

    ZVAL_COPY(retval, &handler);
}

//
// Replace the array input with the sequence of elements as specified
// by the offset and length parameters.
//

void PHPClientUser::ArraySlice( zval *input, long offset, long length  )
{
    zval func;
    zval params[3];
    zval param1, param2;

    zval tmp;

    // ignore non-array input.
    if ( Z_TYPE_P( input ) != IS_ARRAY )
    {
        return;
    }

    // call the array_slice user function.
    ZVAL_STRING(&func, "array_slice");

    ZVAL_LONG(&param1, offset);
    ZVAL_LONG(&param2, length);

    params[0] = *input;
    params[1] = param1;
    params[2] = param2;

    TSRMLS_FETCH();
    call_user_function(EG(function_table), NULL, &func, &tmp, 2, params TSRMLS_CC);

    // copy result to input array, then free result.
    ZVAL_COPY_VALUE(input, &tmp);

    zval_dtor(&func);
}

PHPClientSSO::PHPClientSSO( SpecMgr *s )
{
    specMgr = s;
    resultSet = 0;
    ssoEnabled = 0;
    ZVAL_NULL(&result);
}

PHPClientSSO::~PHPClientSSO()
{
    if( Z_TYPE(result) != IS_NULL ) {
        zval_dtor(&result);
    }
}

ClientSSOStatus
PHPClientSSO::Authorize( StrDict &vars, int maxLength, StrBuf &strbuf )
{
    ssoVars.Clear();

    if( !ssoEnabled )
        return CSS_SKIP;

    if( ssoEnabled < 0 )
        return CSS_UNSET;

    if( resultSet )
    {
        strbuf.Clear();

        if( PHP_PERFORCE_DEBUG_CALLS )
            std::cerr << "[P4] ClientSSO::Authorize(). Using supplied input"
                      << std::endl;

        if( Z_TYPE( result ) == IS_ARRAY )
        {
            // determine if input is a hash or a vector.
            HashTable *ht = Z_ARRVAL( result );
            HashPosition pos;

            zend_hash_internal_pointer_reset_ex( ht, &pos );

            zend_string *k;
            zend_ulong index;
            uint ktype = zend_hash_get_current_key_ex( ht, &k, &index, &pos );
            bool isHash = ( ktype == HASH_KEY_IS_STRING );

            if( !isHash )
            {
                // handle one element of a vector at a time. Note, only strings
                // are supported at this time.
                uint argc = zend_hash_num_elements( ht );
                zval *val = zend_hash_index_find( ht, 0 );

                if( Z_TYPE_P( val ) == IS_STRING ) {
                    PHPClientUser::ArraySlice( &result, 1, argc );
                    strbuf.Set( Z_STRVAL_P(val)  );
                }
            }
        }

        // strings are simple, just feed the string into strbuf
        if( Z_TYPE( result ) == IS_STRING )
        {
            strbuf.Set( Z_STRVAL( result ), Z_STRLEN( result ) );
        }

        if ( Z_TYPE(result) != IS_NULL ) {
            zval_dtor(&result);
            ZVAL_NULL(&result);
        }

        return resultSet == 2 ? CSS_FAIL
                              : CSS_PASS;
    }

    ssoVars.CopyVars( vars );
    return CSS_EXIT;
}

bool
PHPClientSSO::EnableSSO( zval *e )
{
    if ( Z_TYPE_P( e ) == IS_NULL )
    {
        ssoEnabled = 0;
        return true;
    }

    if ( Z_TYPE_P( e ) == IS_TRUE )
    {
        ssoEnabled = 1;
        return true;
    }

    if ( Z_TYPE_P( e ) == IS_FALSE )
    {
        ssoEnabled = -1;
        return true;
    }

    return false;
}

void
PHPClientSSO::SSOEnabled( zval *retval )
{
    if( ssoEnabled == 1 )
    {
        ZVAL_TRUE(retval);
    }
    else if( ssoEnabled == -1 )
    {
        ZVAL_FALSE(retval);
    }
    else
    {
        ZVAL_NULL(retval);
    }
}

bool
PHPClientSSO::SetPassResult( zval *i )
{
    resultSet = 1;
    return SetResult( i );
}

void
PHPClientSSO::GetPassResult( zval *retval )
{
    if( resultSet == 1 )
    {
        ZVAL_COPY(retval, &result);
    }
    else
    {
        ZVAL_NULL(retval);
    }
}

bool
PHPClientSSO::SetFailResult( zval *i )
{
    resultSet = 2;
    return SetResult( i );
}

void
PHPClientSSO::GetFailResult( zval *retval )
{
    if( resultSet == 2 )
    {
        ZVAL_COPY(retval, &result);
    }
    else
    {
        ZVAL_NULL(retval);
    }
}

bool
PHPClientSSO::SetResult( zval *i )
{
    // It will probably not happen often, but if you first assign one value and
    // them replace it with another value, you would leave a memory leak

    if ( Z_TYPE(result) != IS_NULL ) {
        zval_dtor(&result);
    }

    if ( Z_TYPE_P( i ) == IS_OBJECT )
    {
        return false;
    }

    if ( Z_TYPE_P( i ) == IS_NULL     ||
         Z_TYPE_P( i ) == IS_LONG     ||
         Z_TYPE_P( i ) == IS_DOUBLE   ||
         Z_TYPE_P( i ) == IS_TRUE     ||
         Z_TYPE_P( i ) == IS_FALSE    ||
         Z_TYPE_P( i ) == IS_RESOURCE )
    {
        convert_to_string( i );
    }

    if ( Z_TYPE_P( i ) == IS_ARRAY )
    {
        zval tmp;
        array_init( &result );
        zend_hash_copy( Z_ARRVAL( result ), Z_ARRVAL_P( i ),
                        (copy_ctor_func_t)zval_add_ref );
        return true;
    }
    
    if ( Z_TYPE_P( i ) == IS_STRING )
    {
        ZVAL_STRINGL( &result, Z_STRVAL_P(i), Z_STRLEN_P(i));
        return true;
    }

    return false;
}

void
PHPClientSSO::GetSSOVars( zval *retval )
{
    specMgr->StrDictToHash( &ssoVars, retval );
}

