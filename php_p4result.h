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
 * php_p4result.h - collects results from Perforce
 *
 * Classes:
 * 
 *  P4Result - Collects results, errors and warnings from Perforce commands. 
 * 
 * Methods:
 * 
 *  P4Result::AddOutput() - Add output to the collection of results
 *  P4Result::AddError() - Add an error to the error collection
 *  P4Result::GetOutput() - Get all output 
 *  P4Result::GetErrors() - Get all errors
 *  P4Result::GetWarnings() - Get all warnings
 *  P4Result::FmtErrors() - Format error messages
 *  P4Result::FmtWarnings() - Format warnings
 *  P4Result::ErrorCount() - Get the number of errors encountered
 *  P4Result::WarningCount() - Get the number of warnings encountered
 *  P4Result::Reset() - Reset warnings, errors, output
 */

#ifndef PHP_P4RESULT_H
#define PHP_P4RESULT_H

class P4Result
{
    public:

        P4Result();
        ~P4Result();

    // Setting
    void   AddOutput( const char *msg );
    void   AddOutput( zval *out );
    void   AddError( Error *e );

    // Getting
    void GetOutput( zval *retval );
    void GetErrors( zval *retval );
    void GetWarnings( zval *retval );

    // Get errors/warnings as a formatted string
    void   FmtErrors( StrBuf &buf );
    void   FmtWarnings( StrBuf &buf );

    // Testing
    int    ErrorCount();
    int    WarningCount();

    // Clear previous results
    void   Reset();

    private:
    
    int   Length( zval *ary );
    void  Fmt( const char *label, zval& list, StrBuf &buf );
    void  AppendString( zval *list, const char *str );

    zval output;
    zval errors;
    zval warnings;
};

#endif /* PHP_P4RESULT_H */ 
