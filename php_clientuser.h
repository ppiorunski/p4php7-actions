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
 * php_clientuser.h - Implementation of ClientUser. Handles I/O.
 *
 * Classes:
 *
 *  PHPClientUser - Implementation of ClientUser.
 *
 *  The PHPClientUser class subclasses ClientUser to handle all I/O for Perforce
 *  commands.
 *
 * Methods:
 *
 *  PHPClientUser::HandleError() - Virtual method. Process error data on failure
 *  PHPClientUser::OutputText() - Virtual method. Output textual data
 *  PHPClientUser::OutputInfo() - Virtual method. Output tabular data
 *  PHPClientUser::OutputStat() - Virtual method. Process tagged output
 *  PHPClientUser::OutputBinary() - Virtual method. Output binary data
 *  PHPClientUser::InputData() - Virtual method. Provide input data to commands
 *  PHPClientUser::Diff() - Virtual method. Diff two methods and display results
 *  PHPClientUser::Prompt() - Virtual method. Prompt the user and get a response
 *  PHPClientUser::Resolve() - Virtual method. Perform a resolve
 *  PHPClientUser::Finished() - Virtual method. To be called after a command
 *  PHPClientUser::GetInput() - Get the user input to be used for next command
 *  PHPClientUser::SetInput() - Set the user input to be used for next command
 *  PHPClientUser::GetResolver() - Get the resolver used when resolving
 *  PHPClientUser::SetResolver() - Set the resolver used when resolving
 *  PHPClientUser::SetCommand() - Set the name of the command being run
 *  PHPClientUser::GetResults() - Get the results collected from a command
 *  PHPClientUser::ErrorCount() - Get the error count
 *  PHPClientUser::Reset() - Reset all data, doesn't touch user input
 *  PHPClientUser::SetDebug() - Toggle the debug flag
 */

#ifndef PHP_CLIENT_USER_H
#define PHP_CLIENT_USER_H

class PHPClientSSO : public ClientSSO
{
    public:
        PHPClientSSO( SpecMgr *s );
        virtual ~PHPClientSSO();

    // Client SSO methods overridden here
    virtual ClientSSOStatus Authorize( StrDict &vars, int maxLength,
                                       StrBuf &result );

    // Local methods
    bool EnableSSO( zval *e );
    void SSOEnabled( zval *retval );
    bool SetPassResult( zval *i );
    void GetPassResult( zval *retval );
    bool SetFailResult( zval *i );
    void GetFailResult( zval *retval );
    void GetSSOVars( zval *retval );

    private:

    bool SetResult( zval *i );

    int         ssoEnabled;
    int         resultSet;

    StrBufDict  ssoVars;
    SpecMgr *   specMgr;

    zval        result;
};

class PHPClientUser : public ClientUser, public KeepAlive
{
    public:

        PHPClientUser( SpecMgr *s );
        virtual ~PHPClientUser();

    // Client User methods overridden here
    virtual void HandleError( Error *e );
    virtual void OutputText( const char *data, int length );
    virtual void OutputInfo( char level, const char *data );
    virtual void OutputStat( StrDict *values );
    virtual void OutputBinary( const char *data, int length );
    virtual void InputData( StrBuf *strBuf, Error *e );
    virtual void Diff( FileSys *f1, FileSys *f2, int doPage, char *diffFlags,
                       Error *e );
    virtual void Prompt( const StrPtr &msg, StrBuf &rsp, int noEcho, Error *e );
    virtual int  Resolve( ClientMerge *m, Error *e );
    virtual int  Resolve( ClientResolveA *m, int preview, Error *e );
    virtual void Finished();

    int          IsAlive() { return alive; }

    // Local methods
    bool SetInput( zval *i );
    void GetInput( zval *retval );

    bool SetResolver( zval *i );
    void GetResolver( zval *retval);

    bool SetHandler( zval *h );
    void GetHandler( zval *retval );

    void SetCommand( const char *c ) { cmd = c; }
    P4Result &GetResults() { return results; }

    zval MkMergeInfo( ClientMerge *m, StrPtr &hint );

    bool EnableSSO( zval *e )        { return ssoHandler->EnableSSO( e ); }
    void SSOEnabled(zval *retval )   { ssoHandler->SSOEnabled( retval ); }
    bool SetSSOPassResult( zval *i ) { return ssoHandler->SetPassResult( i ); }
    void GetSSOPassResult( zval *retval ){ssoHandler->GetPassResult( retval );}
    bool SetSSOFailResult( zval *i ) { return ssoHandler->SetFailResult( i ); }
    void GetSSOFailResult( zval *retval ){ssoHandler->GetFailResult( retval );}
    void GetSSOVars( zval *retval )  { ssoHandler->GetSSOVars( retval ); }

    int ErrorCount();
    void Reset();

    // Debugging Support
    void SetDebug( int d ) { debug = d; }
    static void ArraySlice( zval *input, long offset, long length );

    private:

    P4Result    results;
    StrBuf      cmd;
    SpecMgr *   specMgr;

    int         debug;
    int         alive;

    zval        input;
    zval        resolver;
    zval        handler;

    PHPClientSSO * ssoHandler; 

    bool CallOutputMethod( const char * method, zval *data);
    void ProcessOutput( const char * method, zval *data);
};

#endif /* PHP_CLIENT_USER_H */
