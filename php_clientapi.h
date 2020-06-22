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
 * php_clientapi.h - acts as a bridge between PHP and the P4API
 *
 * Classes:
 *
 *  PHPClientAPI - a bridget between PHP and the P4API
 *
 *  Most of the methods in the PHPClientAPI class are accessors and
 *  mutators that simply expose attributes to PHP userspace scripts.
 *  These accessors and mutators must have a common interface so that
 *  they can be used in the p4_properties[] table in the P4 PHP class
 *  implementation.
 *
 * Methods:
 *
 *  PHPClientAPI::Run() - Run a command against the Perforce server
 *  PHPClientAPI::Connect() - Connect to the Perforce server
 *  PHPClientAPI::Disconnect() - Disconnect from the Perforce server
 *  PHPClientAPI::Connected() - Determine whether a connection is open
 *  PHPClientAPI::GetInput() - Get input to be used for next command
 *  PHPClientAPI::SetInput() - Set input to be used for next command
 *  PHPClientAPI::GetApiLevel() - Get the api level of the server
 *  PHPClientAPI::SetApiLevel() - Set the api level
 *  PHPClientAPI::GetCharset() - Get the character set being used
 *  PHPClientAPI::SetCharset() - Set the character set to be used
 *  PHPClientAPI::GetClient() - Get the client workspace
 *  PHPClientAPI::SetClient() - Set the client workspace
 *  PHPClientAPI::GetCwd() - Get the current working directory
 *  PHPClientAPI::SetCwd() - Set the current working directory
 *  PHPClientAPI::GetEnv() - Get the value of the given Perforce environment variable.
 *  PHPClientAPI::GetErrors() - Get the list of errors encountered
 *  PHPClientAPI::GetExceptionLevel() - Get the exception level
 *  PHPClientAPI::SetExceptionLevel() - Set the exception level
 *  PHPClientAPI::GetHost() - Get the host used when connecting
 *  PHPClientAPI::SetHost() - Set the host used when connecting
 *  PHPClientAPI::GetMaxLockTime() - Get the maxlocktime
 *  PHPClientAPI::SetMaxLockTime() - Set the maxlocktime
 *  PHPClientAPI::GetMaxScanRows() - Get maxscanrows
 *  PHPClientAPI::SetMaxScanRows() - Set maxscanrows
 *  PHPClientAPI::GetConfig() - Get the current config used
 *  PHPClientAPI::GetPassword() - Get the user password
 *  PHPClientAPI::SetPassword() - Set the user password
 *  PHPClientAPI::GetPort() - Get the user port
 *  PHPClientAPI::SetPort() - Set the user port
 *  PHPClientAPI::GetProg() - Get the program name
 *  PHPClientAPI::SetProg() - Set the program name
 *  PHPClientAPI::GetResolver() - Get the resolver to be used with resolve
 *  PHPClientAPI::SetResolver() - Set the resolver to be used with resolve
 *  PHPClientAPI::GetServerLevel() - Get the server protocol level
 *  PHPClientAPI::GetTagged() - Determine if tagged output is enabled
 *  PHPClientAPI::SetTagged() - Enable / Disable tagged output
 *  PHPClientAPI::GetStreams() - Determine if stream support is enabled
 *  PHPClientAPI::SetStreams() - Enable / Disable stream support
 *  PHPClientAPI::GetTicketFile() - Get the ticket file path
 *  PHPClientAPI::SetTicketFile() - Set the ticket file to use
 *  PHPClientAPI::GetUser() - Get the username of the user connecting
 *  PHPClientAPI::SetUser() - Set the username to connect as
 *  PHPClientAPI::GetVersion() - Get the version of the script
 *  PHPClientAPI::SetVersion() - Set the version of the script
 *  PHPClientAPI::GetExpandSequences() - Determine if numeric sequences in tagged output get expanded.
 *  PHPClientAPI::SetExpandSequences() - Enable / Disable expansion of numeric sequences in tagged output.
 *  PHPClientAPI::GetWarnings() - Get the list of warnings encountered
 *  PHPClientAPI::ParseSpec() - Parses a Perforce spec into an array
 */

#ifndef PHP_CLIENT_API_H
#define PHP_CLIENT_API_H

class Enviro;
class PHPClientAPI
{
    public:
            PHPClientAPI();
            ~PHPClientAPI();

    void Run( zend_string **argv, int argc, zval *retval );

    zval Connect();
    void Disconnect();
    bool Connected()  { return connected; }

    const char *GetEnv( char *var );
    zend_string *FormatSpec( const char *type, zval *hash );

    // The input for the next command.
    void SetInput( zval *i );
    void GetInput( zval *retval );

    // Set API level for backwards compatibility
    void SetApiLevel( zval *level );
    void GetApiLevel( zval *retval );

    // Character set used by unicode enabled servers.
    void SetCharset( zval *charset );
    void GetCharset( zval *retval );

    // Client workspace.
    void SetClient( zval *c );
    void GetClient( zval *retval );

    // Current Working Directory.
    void SetCwd( zval *c );
    void GetCwd( zval *retval );

    // Errors
    void GetErrors( zval *retval );

    // Exception levels:
    //
    //      0 - No exceptions raised
    //      1 - Exceptions raised for errors
    //      2 - Exceptions raised for errors and warnings
    void SetExceptionLevel( zval *level );
    void GetExceptionLevel( zval *retval );

    // The name of the current host.
    void SetHost( zval *h );
    void GetHost( zval *retval );

    // The amount of time (in milliseconds) spent during data scans
    void SetMaxLockTime( zval *v );
    void GetMaxLockTime( zval *retval );

    // The number of results Perforce permits for subsequence commands
    void SetMaxResults( zval *v );
    void GetMaxResults( zval *retval );

    // The number of db records Perforce scans for subsequent commands
    void SetMaxScanRows( zval *v );
    void GetMaxScanRows( zval *retval );

    // The name of the current P4CONFIG file, if any
    void GetConfig( zval *retval );

    // The password for the Perforce user
    void SetPassword( zval *p );
    void GetPassword( zval *retval );

    // The port of the Perforce server
    void SetPort( zval *p );
    void GetPort( zval *retval );

    // Name of the program currently running
    void SetProg( zval *p );
    void GetProg( zval *retval );

    // The resolver used for resolve commands
    void SetResolver( zval *r );
    void GetResolver( zval *retval );

    // The current Perforce server level
    void GetServerLevel( zval *retval );

    // Tagged mode - can be enabled/disabled on a per-command basis
    void SetTagged( zval *enable );
    void GetTagged( zval *retval );

    // Streams mode - can be enabled/disabled on a per-connection basis
    void SetStreams( zval *enable );
    void GetStreams( zval *retval );

    // The location of the P4TICKETS file
    void SetTicketFile( zval *t );
    void GetTicketFile( zval *retval );

    //  ClientApi Accessor / Mutator Wrappers
    void SetUser( zval *u );
    void GetUser( zval *retval );

    // P4-PHP Version
    void SetVersion( zval *v );
    void GetVersion( zval *retval );

    // Expand numeric sequences in tagged output - can be toggled per-command.
    void SetExpandSequences( zval *v );
    void GetExpandSequences( zval *retval );

    // Set an output handler to use the streaming data interface
    void SetHandler( zval *h );
    void GetHandler( zval *retval );

    // Warnings array
    void GetWarnings( zval *retval );

    // Parse a Perforce form / spec into an array
    void ParseSpec( const char *type, const char *form, zval *retval );

    // Expose setProtocol/Var
    void SetProtocol( const char *key, const char *value ) { client.SetProtocol(key, value); }
    void SetVar( const char *key, const char *value ) { client.SetVar(key, value); }

    // Extended variable handling
    void SetEVar( const char *key, const char *value) { 
        StrRef k(key); 
        StrRef v(value); 
        client.SetEVar(k, v); 
    }
    char * GetEVar( const char *key ) { 
        StrRef k(key); 
        return client.GetEVar(k)->Text(); 
    }

    // SSO handler
    void SetEnableSSO( zval *e );
    void GetEnableSSO( zval *retval );
    void GetSSOVars( zval *retval );
    void SetSSOPassResult( zval *r );
    void GetSSOPassResult( zval *retval );
    void SetSSOFailResult( zval *r );
    void GetSSOFailResult( zval *retval );


    private:

    ClientApi client;
    PHPClientUser ui;
    Enviro *enviro;
    SpecMgr specMgr;
    StrBuf charset;
    StrBuf ticketFile;
    StrBuf prog;
    StrBuf version;

    int  depth;
    bool connected;
    int  apiLevel;
    int  exceptionLevel;
    int  server2;
    int  mode;

    int  maxResults;
    int  maxScanRows;
    int  maxLockTime;

    void Except( const char *func, const char *msg );
    void Except( const char *func, Error *e );
    void Except( const char *func, const char *msg, const char *cmd );

    void RunCmd( const char *cmd, int argc, zend_string **argv );
};

#endif /* PHP_CLIENT_API_H */
