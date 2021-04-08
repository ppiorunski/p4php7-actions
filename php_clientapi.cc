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
#include "strtable.h"

#include "errorlog.h"
#include "debug.h"

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}
#include "Zend/zend_exceptions.h"
#include "php_macros.h"

#include "specmgr.h"
#include "php_p4result.h"
#include "php_clientuser.h"
#include "php_clientapi.h"
#include "php_p4_exception.h"

#include <iostream>

#define M_TAGGED       0x01
#define M_PARSE_FORMS  0x02
#define M_STREAMS      0x40

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

//
// Create a new client api object.
//

PHPClientAPI::PHPClientAPI() : ui( &specMgr )
{
    connected = false;
    depth = 0;
    server2 = 0;
    exceptionLevel = 2;
    enviro = new Enviro;
    prog = "P4PHP";
    version = ID_REL "/" ID_OS "/" ID_PATCH " (" ID_API " API)";

    apiLevel = atoi( P4Tag::l_client );

    maxResults = 0;
    maxScanRows = 0;
    maxLockTime = 0;

    p4debugHelper = 0;
    reportLog = 0;

    // Enable form parsing, streams and set tagged mode on by default
    mode = M_PARSE_FORMS | M_TAGGED | M_STREAMS;
    client.SetProtocol( "specstring", "" );

    // Load the current working directory, and any P4CONFIG file in place
    HostEnv henv;
    StrBuf  cwd;

    henv.GetCwd( cwd, enviro );
    if ( cwd.Length() )
        enviro->Config( cwd );

    // Load the current tickets file. Start with the default, and then
    // override it if P4TICKETS is set.
    const char *t;

    henv.GetTicketFile( ticketFile );

    if ( ( t = enviro->Get( "P4TICKETS" ) ) ) {
        ticketFile = t;
    }

    // Do the same for P4CHARSET
    char *lc;
    if ( ( lc = enviro->Get( "P4CHARSET" ) ) ) {
        zval charset;
        ZVAL_STRING(&charset, lc);
        SetCharset( &charset );
    }
}

//
// Deconstructor. Clean up and close connection if not already closed.
//

PHPClientAPI::~PHPClientAPI()
{
    if ( connected )
    {
        Error e;
        client.Final( &e );
    }

    delete enviro;

    delete p4debugHelper;
    delete reportLog;
}

//
// RunCmd is a private function to work around an obscure protocol
// bug in 2000.[12] servers. Running a "p4 -Ztag client -o" messes up the
// protocol so if they're running this command then we disconnect and
// reconnect to refresh it. For efficiency, we only do this if the
// server2 protocol is either 9 or 10 as other versions aren't affected.
//

void
PHPClientAPI::RunCmd( const char *cmd, int argc, zend_string **argStrs )
{
    // ClientApi::SetProg() was introduced in 2004.2
    client.SetProg( &prog );

    // ClientApi::SetVersion() was introduced in 2005.2
    if ( version.Length() )
        client.SetVersion( &version );

    if ( mode & M_TAGGED )
        client.SetVar( "tag" );

    if ( (mode & M_STREAMS) && apiLevel > 69 )
        client.SetVar( "enableStreams" );

    // if maxresults or maxscanrows is set, enforce them now
    if ( maxResults  ) client.SetVar( "maxResults",  maxResults );
    if ( maxScanRows ) client.SetVar( "maxScanRows", maxScanRows );
    if ( maxLockTime ) client.SetVar( "maxLockTime", maxLockTime );

    // we had used client.SetArgv in the past but it isn't
    // binary safe on the input strings. the below loop will
    // use SetVar for each param which allows us to specify 
    // the length and not rely on null termination.
    // note: argv[0] holds the command so we skip it.
    for( int i = 1; i < argc; i++ )
    {
        if ( PHP_PERFORCE_DEBUG ) {
            std::cerr << "SetVar(" << ZSTR_VAL(argStrs[i]) << ")" << std::endl;
        }
        client.SetVar( StrRef::Null(), StrRef( ZSTR_VAL( argStrs[i] ), ZSTR_LEN( argStrs[i] ) ) );
    }

    if ( PHP_PERFORCE_DEBUG ) {
        std::cerr << "Running cmd: " << cmd << std::endl;
    }
    client.Run( cmd, &ui );

    // Have to request server2 protocol *after* a command has been run. I
    // don't know why, but that's the way it is.
    // [SEK] Because it comes from the server - not the client.

    if ( !server2 )
    {
        StrPtr *pv = client.GetProtocol( "server2" );
        if ( pv )
            server2 = pv->Atoi();
    }
}

//
// Execute the perforce command and arguments given in argv.
//

void
PHPClientAPI::Run( zend_string **strArgs, int argc, zval *retval )
{
    StrBuf cmdString;
    int i;

    if ( depth )
    {
        php_error( E_WARNING,
             "P4::run() - Can't execute nested Perforce commands.", 1 );
        ZVAL_FALSE( retval );
        return;
    }

    // throw if possible when we are not connected
    if ( !connected && exceptionLevel ) {
        Except( "P4.run()", "not connected." );
        ZVAL_FALSE( retval );
        return;
    }

    // double check connection; we may have exceptions disabled 
    // preventing the earlier check. Also verify we have at least
    // one argument as we need a command if nothing else.
    if ( !connected || !argc ) {
        ZVAL_FALSE( retval );
        return;
    }

    // generate the cmdString for use with exceptions 
    // and debugging if we need it
    if ( exceptionLevel || PHP_PERFORCE_DEBUG ) {
        cmdString << "\"p4";
        for ( i = 0; i < argc; i++ )
        {
            cmdString << " " << ZSTR_VAL( strArgs[i] );
        }
        cmdString << "\"";
    }

    if ( PHP_PERFORCE_DEBUG )
    {
        std::cerr << "[PHPClientAPI::Run] ";
        std::cerr << cmdString.Text() << std::endl;
    }

    ui.SetCommand( ZSTR_VAL(strArgs[0]) );
    ui.Reset();

    depth++;
    RunCmd( ZSTR_VAL(strArgs[0]), argc, strArgs );
    depth--;

    P4Result &results = ui.GetResults();
    results.GetOutput( retval );

    if ( results.ErrorCount() && exceptionLevel )
    {
        Except( "P4.run()", "Errors during command execution", 
            cmdString.Text() );
    }

    if ( results.WarningCount() && exceptionLevel > 1 )
    {
        Except( "P4.run()", "Warnings during command execution",
            cmdString.Text() );
    }
}

//
// Try to connect to the Perforce server.
//

zval
PHPClientAPI::Connect()
{
    zval retval;
    Error e;

    if ( PHP_PERFORCE_DEBUG )
    {
        std::cerr << "[P4] {PHPClientAPI::Connect()} "
             << "Connecting to Perforce" << std::endl;
    }

    if ( connected )
    {
        php_error( E_WARNING,
            "P4::connect() - Perforce client already connected!", 1 );
        ZVAL_TRUE( &retval );
        return retval;
    }

    if ( PHP_PERFORCE_DEBUG )
    {
        std::cerr << "[P4] {PHPClientAPI::Connect()} "
             << "Initializing ClientAPI" << std::endl;
    }
    
    client.Init( &e );

    if ( e.Test() && exceptionLevel )
    {
        connected = false;
        Except( "P4.connect()", &e );
        ZVAL_FALSE( &retval );
        return retval;
    }

    if ( PHP_PERFORCE_DEBUG )
        std::cerr << "[P4] Client Connected" << std::endl;

    connected = true;
    ZVAL_TRUE( &retval );
    return retval;
}

//
// Disconnect from the Perforce server.
//

void
PHPClientAPI::Disconnect()
{
    if ( !connected )
    {
        php_error( E_WARNING,
            "P4::disconnect() - Not connected!", 1 );
        return;
    }

    Error e;
    client.Final( &e );

    // Clear the specdef cache.
    specMgr.Reset();

    connected = false;
}

//
// Converts a hash supplied by the user into a string using the specstring
// from the server. We may have to fetch the specstring first.
//

zend_string *
PHPClientAPI::FormatSpec( const char *type, zval *hash )
{
    if ( !specMgr.HaveSpecDef( type ) )
    {
        if ( exceptionLevel )
        {
            StrBuf m;
            m = "No spec definition for ";
            m.Append( type );
            m.Append( " objects." );
            Except( "P4.format_spec()", m.Text() );
        }

        return NULL;
    }

    // Got a specdef so now we can attempt to convert
    StrBuf  buf;
    Error   e;

    specMgr.SpecToString( type, hash, buf, &e );

    if ( !e.Test() )
    {
        return zend_string_init(buf.Text(), buf.Length(), 0 );
    }

    if ( exceptionLevel )
    {
        StrBuf m;
        m = "Error converting hash to string.";
        if ( e.Test() ) e.Fmt( m, EF_PLAIN );
        Except( "P4.format_spec()", m.Text() );
    }

    return NULL;
}

//
// Get the value of a Perforce environment variable
//

const char *
PHPClientAPI::GetEnv( char *var )
{
    return enviro->Get( var );
}

//
// Get array of error messages and return as a zval.
//

void
PHPClientAPI::GetErrors( zval *retval )
{
    P4Result &results = ui.GetResults();
    results.GetErrors( retval );
}

//
// Get array of warning messages and return as a zval.
//

void
PHPClientAPI::GetWarnings( zval *retval )
{
    P4Result &results = ui.GetResults();
    results.GetWarnings( retval );
}

//
// Determine if tagged output is enabled or not.
//

void
PHPClientAPI::GetTagged( zval *retval )
{
    ZVAL_BOOL(retval, mode & M_TAGGED);
}

//
// Toggle between tagged / non-tagged output.
//

void
PHPClientAPI::SetTagged( zval *enable )
{
    convert_to_boolean(enable);
    bool tagged = (Z_TYPE_P(enable) == IS_TRUE);

    if ( tagged )
        mode |= M_TAGGED;
    else
        mode &= ~M_TAGGED;
}

//
// Determine if streams support is enabled or not.
//

void
PHPClientAPI::GetStreams( zval *retval )
{
    ZVAL_BOOL(retval, mode & M_STREAMS);
}

//
// Toggle between streams enabled / not-enabled.
//

void
PHPClientAPI::SetStreams( zval *enable )
{
    convert_to_boolean(enable);
    bool streams = (Z_TYPE_P(enable) == IS_TRUE);

    if( streams )
        mode |= M_STREAMS;
    else
        mode &= ~M_STREAMS;
}

//
// Set the charset.
//

void
PHPClientAPI::SetCharset( zval *c )
{
    if ( Z_TYPE_P( c ) != IS_STRING ) return;

    CharSetApi::CharSet cs = CharSetApi::Lookup( Z_STRVAL_P(c) );

    if ( cs < 0 ) {
        if ( exceptionLevel ) {
            StrBuf m;
            m = "Unknown or unsupported charset: ";
            m.Append( Z_STRVAL_P(c) );
            Except( "SetCharSet", m.Text() );
        }
    }
    charset = Z_STRVAL_P(c);
    client.SetTrans( cs, cs, cs, cs );
}

//
// Get the current charset being used.
//

void
PHPClientAPI::GetCharset( zval *retval )
{
    ZVAL_STRING( retval, charset.Text());
}

//
// Set current working directory.
//

void
PHPClientAPI::SetCwd( zval *cwd )
{
    if ( Z_TYPE_P( cwd ) != IS_STRING ) return;
    client.SetCwd( Z_STRVAL_P( cwd ) );
    enviro->Config( StrRef( Z_STRVAL_P( cwd ) ) );
}

//
// Get current working directory.
//

void
PHPClientAPI::GetCwd( zval *retval )
{
    ZVAL_STRING( retval, client.GetCwd().Text() );
}

//
// Set the location of the P4TICKETS file.
//

void
PHPClientAPI::SetTicketFile( zval *t )
{
    ticketFile = Z_STRVAL_P( t );
    client.SetTicketFile( ticketFile.Text() );
}

//
// The location of the P4TICKETS file
//

void
PHPClientAPI::GetTicketFile( zval *retval )
{
    ZVAL_STRING( retval, ticketFile.Text());
}


//
// Set the input for the next command.
//

void
PHPClientAPI::SetInput( zval *i )
{
    ui.SetInput( i );
}

//
// Get the input for the next command.
//

void
PHPClientAPI::GetInput( zval *retval )
{
    ui.GetInput( retval );
}

//
// Set the API level for backwards compatibility.
//

void
PHPClientAPI::SetApiLevel( zval *level )
{
    StrBuf  b;
    apiLevel = (int) Z_LVAL_P( level );
    b << apiLevel;
    client.SetProtocol( "api", b.Text() );
}

//
// Get the API level.
//

void
PHPClientAPI::GetApiLevel( zval *retval )
{
    ZVAL_LONG( retval, (long)apiLevel );
}

//
// Set the client workspace to use.
//

void
PHPClientAPI::SetClient( zval *c )
{
    if ( Z_TYPE_P( c ) != IS_STRING ) return;
    client.SetClient( Z_STRVAL_P( c ) );
}

//
// Get the name of the current client workspace.
//

void
PHPClientAPI::GetClient( zval *retval )
{
    ZVAL_STRING( retval, client.GetClient().Text());
}

// Exception levels:
//
//      0 - No exceptions raised
//      1 - Exceptions raised for errors
//      2 - Exceptions raised for errors and warnings

void
PHPClientAPI::SetExceptionLevel( zval *level )
{
    if ( Z_TYPE_P( level ) != IS_LONG ) return;
    exceptionLevel = (int) Z_LVAL_P( level );
}

void
PHPClientAPI::GetExceptionLevel( zval *retval )
{
    ZVAL_LONG( retval, (long)exceptionLevel );
}

//
// The name of the current host.
//

void
PHPClientAPI::SetHost( zval *h )
{
    if ( Z_TYPE_P( h ) != IS_STRING ) return;
    client.SetHost( Z_STRVAL_P( h ) );
}

void
PHPClientAPI::GetHost( zval *retval )
{
    ZVAL_STRING( retval, client.GetHost().Text() );
}

//
// The amount of time (in milliseconds) spent during data scans
//

void
PHPClientAPI::SetMaxLockTime( zval *v )
{
    if ( Z_TYPE_P( v ) != IS_LONG ) return;
    maxLockTime = (int) Z_LVAL_P( v );
}

void
PHPClientAPI::GetMaxLockTime( zval *retval )
{
    ZVAL_LONG( retval, (long) maxLockTime );
}

//
// The number of results Perforce permits for subsequence commands
//

void
PHPClientAPI::SetMaxResults( zval *v )
{
    if ( Z_TYPE_P( v ) != IS_LONG ) return;
    maxResults  = (int) Z_LVAL_P( v );
}

void
PHPClientAPI::GetMaxResults( zval *retval )
{
    ZVAL_LONG( retval, (long) maxResults );
}

//
// The number of db records Perforce scans for subsequent commands
//

void
PHPClientAPI::SetMaxScanRows( zval *v )
{
    if ( Z_TYPE_P( v ) != IS_LONG ) return;
    maxScanRows = (int) Z_LVAL_P( v );
}

void
PHPClientAPI::GetMaxScanRows( zval *retval )
{
    ZVAL_LONG( retval, (long) maxScanRows );
}

//
// The name of the current P4CONFIG file, if any
//

void
PHPClientAPI::GetConfig( zval *retval )
{
    ZVAL_STRING( retval, client.GetConfig().Text() );
}

//
// The password for the Perforce user
//

void
PHPClientAPI::SetPassword( zval *p )
{
    if ( Z_TYPE_P( p ) != IS_STRING ) // what else would it be?
        convert_to_string( p );
    client.SetPassword( Z_STRVAL_P( p ) );
}

void
PHPClientAPI::GetPassword( zval *retval )
{
    ZVAL_STRING( retval, client.GetPassword().Text() );
}

//
// The port of the Perforce server
//

void
PHPClientAPI::SetPort( zval *p )
{
    if ( Z_TYPE_P( p ) != IS_STRING )  { // now that is nice, P4Python bombs out here
        convert_to_string( p );
    }
    client.SetPort( Z_STRVAL_P( p ) );
}

void PHPClientAPI::GetPort( zval *retval )
{
    ZVAL_STRING( retval, client.GetPort().Text() );
}

//
// Name of the program currently running
//

void
PHPClientAPI::SetProg( zval *p )
{
    if ( Z_TYPE_P( p ) != IS_STRING ) return;
    prog = Z_STRVAL_P( p );
}

void
PHPClientAPI::GetProg( zval *retval )
{
    ZVAL_STRING( retval, prog.Text() );
}

//
// Resolver object to be used with resolve commands
//

void
PHPClientAPI::SetResolver( zval *r )
{
    if ( PHP_PERFORCE_DEBUG_CALLS ) 
        std::cerr << "[P4] Received resolver used for resolve" << std::endl;
    
    if ( ui.SetResolver( r ) ) 
    {
        return;
    } 
    else if ( exceptionLevel )  
    {
        Except( "P4#resolver", 
            "Error setting resolver. Must be an instance of P4_Resolver" );
    }
}

void
PHPClientAPI::GetResolver( zval *retval )
{
    ui.GetResolver( retval );
}

//
// The current Perforce server level
//

void
PHPClientAPI::GetServerLevel( zval *retval )
{
    ZVAL_LONG( retval, server2 );
}

//
//  ClientApi Accessor / Mutator Wrappers
//

void
PHPClientAPI::SetUser( zval *u )
{
    if ( Z_TYPE_P( u ) != IS_STRING ) return;
    client.SetUser( Z_STRVAL_P( u ) );
}

void
PHPClientAPI::GetUser( zval *retval )
{
    ZVAL_STRING( retval, client.GetUser().Text() );
}

//
// P4-PHP Version
//

void
PHPClientAPI::SetVersion( zval *v )
{
    if ( Z_TYPE_P( v ) != IS_STRING ) return;
    version = Z_STRVAL_P( v );
}

void
PHPClientAPI::GetVersion( zval *retval )
{
    ZVAL_STRING( retval, version.Text() );
}

//
// Determine if numeric sequences in tagged output will be expanded.
//

void
PHPClientAPI::GetExpandSequences( zval *retval )
{
    ZVAL_BOOL( retval, specMgr.GetExpandSequences() );
}

//
// Enable / Disable expansion of numeric sequences in tagged output.
//

void
PHPClientAPI::SetExpandSequences( zval *enable )
{
    specMgr.SetExpandSequences( Z_TYPE_P(enable) == IS_TRUE );
}

void
PHPClientAPI::SetEnableSSO( zval *e )
{
    ui.EnableSSO( e );
}

void
PHPClientAPI::GetEnableSSO( zval *retval )
{
    ui.SSOEnabled( retval );
}

void
PHPClientAPI::GetSSOVars( zval *retval )
{
    ui.GetSSOVars( retval );
}

void
PHPClientAPI::SetSSOPassResult( zval *r )
{
    ui.SetSSOPassResult( r );
}

void
PHPClientAPI::GetSSOPassResult( zval *retval )
{
    ui.GetSSOPassResult( retval );
}

void
PHPClientAPI::SetSSOFailResult( zval *r )
{
   ui.SetSSOFailResult( r );
}

void
PHPClientAPI::GetSSOFailResult( zval *retval )
{
    ui.GetSSOFailResult( retval );
}


//
// Set the handler
//

void
PHPClientAPI::SetHandler( zval *h )
{
    if ( ui.SetHandler( h ) && Z_TYPE_P( h ) == IS_OBJECT ) {
        client.SetBreak( &ui );
    } else {
        client.SetBreak( NULL );
    }
}

//
// Get the handler
//

void
PHPClientAPI::GetHandler( zval *retval )
{
    ui.GetHandler( retval );
}

//
// Parse a Perforce form / spec object into an array
//

void 
PHPClientAPI::ParseSpec( const char *type, const char *form, zval *retval )
{
    if ( !specMgr.HaveSpecDef( type ) )
    {
        if ( exceptionLevel ) 
        {
            StrBuf m;
            m = "No spec definition for ";
            m.Append( type );
            m.Append( " objects." );
            Except( "P4.parse_spec()", m.Text() );
        }

        return;
    }   

    // Got a specdef so now we can attempt to parse it.
    Error e;

    // TODO: why can't I just return the spec object the specMgr created?
    // Need to revisit this

    zval spec;
    specMgr.StringToSpec( type, form, &e, &spec );

    // Copy spec to retval and free spec
    ZVAL_COPY_VALUE(retval, &spec);

    // zval_dtor(&spec);

    // Test for errors
    if ( e.Test() )
    {
        if ( exceptionLevel ) 
            Except( "P4.parse_spec()", &e );

        return;
    }
}

//
// Private method. Throw an exception.
//

void
PHPClientAPI::Except( const char *func, Error *e )
{
    StrBuf m;

    e->Fmt( &m );
    Except( func, m.Text() );
}

//
// Private method. Throw an exception.
//

void
PHPClientAPI::Except( const char *func, const char *msg, const char *cmd )
{
    StrBuf m;

    m << msg;
    m << "( " << cmd << " )";
    Except( func, m.Text() );
}

//
// Private method. Throw an exception.
//

void
PHPClientAPI::Except( const char *func, const char *msg )
{
    StrBuf  m;
    StrBuf  errors;
    StrBuf  warnings;
    bool    terminate = false;

    m << "[" << func << "] " << msg;

    // Now append any errors and warnings to the text
    ui.GetResults().FmtErrors( errors );
    ui.GetResults().FmtWarnings( warnings );

    if ( errors.Length() )
    {
        m << "\n" << errors;
        terminate = true;
    }

    if ( exceptionLevel > 1 && warnings.Length() )
    {
        m << "\n" << warnings;
        terminate = true;
    }

    if ( terminate )
        m << "\n\n";

    TSRMLS_FETCH();

    zend_throw_exception_ex( get_p4_exception_ce(),
        0 TSRMLS_CC, m.Text() );
}

//
// Set up the logging
//

void
PHPClientAPI::SetTrace( const char *file, const char *level )
{
    if( !p4debugHelper )
        p4debugHelper = new P4DebugConfig();
    if( !reportLog )
        reportLog = new ErrorLog();

    reportLog->SetLog( file );
    p4debugHelper->Install();
    p4debugHelper->SetErrorLog( reportLog );

    p4debug.SetLevel( level );
}

