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

/*
 * specmgr.h - Manage Perforce Specs.
 *
 * Classes:
 *
 *  SpecMgr - Provides methods for handling Perforce specifications.
 *
 * Methods:
 *
 *  SpecMgr::SetDebug() - Toggle Debug flag
 *  SpecMgr::Reset() - Clear spec cache and revert to defaults
 *  SpecMgr::AddSpecDef() - Add a spec to the cache
 *  SpecMgr::HaveSpecDef() - Check that a type of spec is known
 *  SpecMgr::StringToSpec() - Parse routine - converts strings into PHP hashes
 *  SpecMgr::SpecToString() - Converts a Spec object to a string
 *  SpecMgr::StrDictToHash() - Converts a Perforce StrDict into a PHP hash
 *  SpecMgr::StrDictToSpec() - Converts a Perforce StrDict into a Spec object
 *  SpecMgr::SpecFields() - Returns a list of fields for a given type of spec
 */

#ifndef SPEC_MGR_H
#define SPEC_MGR_H

class StrBufDict;

class SpecMgr
{
    public:

    SpecMgr();
    ~SpecMgr();

    void  SetDebug( int i )   { debug = i; }

    // Clear the spec cache and revert to internal defaults
    void  Reset();

    // Add a spec to the cache
    void  AddSpecDef( const char *type, StrPtr &specDef );
    void  AddSpecDef( const char *type, const char *specDef );

    // Check that a type of spec is known
    int HaveSpecDef( const char *type );

    // Parse routine: converts strings into PHP hashes.
    void StringToSpec( const char *type, const char *spec, Error *e, zval *retval );

    // Format routine. updates a StrBuf object with the form;
    // that can then be converted to a PHP string where required. ONe
    // routine for when we know the type of spec, and one for when we
    // don't.
    void SpecToString( const char *type, zval *hash, StrBuf &buf, Error *e );

    // Convert a Perforce StrDict into a PHP hash.
    void StrDictToHash( StrDict *dict, zval *retval );

    // Convert a Perforce StrDict into an array. This is for
    // 2005.2 and later servers where the forms are supplied pre-parsed
    // into a dictionary - we just need to convert them. In this case,
    // we have the spec, but we don't know what type of spec it is.
    void StrDictToSpec( StrDict *dict, StrPtr *specDef, zval *retval );

    // Return a list of the fields in a given type of spec. Return NULL
    // if the spec type is not known.
    void SpecFields( const char *type, zval *retval );

    // Expand numeric sequences in tagged output - can be enabled/disabled.
    void SetExpandSequences( bool enabled ) { expandSequences = enabled; }
    bool GetExpandSequences() { return expandSequences; }

    private:

    int debug;
    bool expandSequences;
    StrBufDict *specs;

    void SplitKey( const StrPtr *key, StrBuf &base, StrBuf &index );
    void InsertItem( zval *hash, const StrPtr *var, const StrPtr *val );
    void SpecFields( StrPtr *specDef, zval *retval );
};

#endif /* SPEC_MGR_H */
