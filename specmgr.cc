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
#include "strtable.h"
#include "spec.h"

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}

#include "php_macros.h"

#include "specmgr.h"

#include <strops.h>

struct defaultspec {
    const char *type;
    const char *spec;
} speclist[] = {

    {
	"branch",
	"Branch;code:301;rq;ro;fmt:L;len:32;;"
	"Update;code:302;type:date;ro;fmt:L;len:20;;"
	"Access;code:303;type:date;ro;fmt:L;len:20;;"
	"Owner;code:304;fmt:R;len:32;;"
	"Description;code:306;type:text;len:128;;"
	"Options;code:309;type:line;len:32;val:"
	"unlocked/locked;;"
	"View;code:311;type:wlist;words:2;len:64;;"
    },
    {
	"change",
	"Change;code:201;rq;ro;fmt:L;seq:1;len:10;;"
	"Date;code:202;type:date;ro;fmt:R;seq:3;len:20;;"
	"Client;code:203;ro;fmt:L;seq:2;len:32;;"
	"User;code:204;ro;fmt:L;seq:4;len:32;;"
	"Status;code:205;ro;fmt:R;seq:5;len:10;;"
	"Type;code:211;seq:6;type:select;fmt:L;len:10;"
	"val:public/restricted;;"
	"ImportedBy;code:212;type:line;ro;fmt:L;len:32;;"
	"Identity;code:213;type:line;;"
	"Description;code:206;type:text;rq;seq:7;;"
	"JobStatus;code:207;fmt:I;type:select;seq:9;;"
	"Jobs;code:208;type:wlist;seq:8;len:32;;"
    "Stream;code:214;type:line;len:64;;"
	"Files;code:210;type:llist;len:64;;"
    },
    {
	"client",
	"Client;code:301;rq;ro;seq:1;len:32;;"
	"Update;code:302;type:date;ro;seq:2;fmt:L;len:20;;"
	"Access;code:303;type:date;ro;seq:4;fmt:L;len:20;;"
	"Owner;code:304;seq:3;fmt:R;len:32;;"
	"Host;code:305;seq:5;fmt:R;len:32;;"
	"Description;code:306;type:text;len:128;;"
	"Root;code:307;rq;type:line;len:64;;"
	"AltRoots;code:308;type:llist;len:64;;"
	"Options;code:309;type:line;len:64;val:"
	"noallwrite/allwrite,noclobber/clobber,nocompress/compress,"
	"unlocked/locked,nomodtime/modtime,normdir/rmdir;;"
	"SubmitOptions;code:313;type:select;fmt:L;len:25;val:"
	"submitunchanged/submitunchanged+reopen/revertunchanged/"
	"revertunchanged+reopen/leaveunchanged/leaveunchanged+reopen;;"
	"LineEnd;code:310;type:select;fmt:L;len:12;val:"
	"local/unix/mac/win/share;;"
	"Stream;code:314;type:line;len:64;;"
	"StreamAtChange;code:316;type:line;len:64;;"
	"ServerID;code:315;type:line;ro;len:64;;"
    "Type;code:318;type:select;len:10;val:"  
    "writeable/readonly/graph/partitioned;;"
	"Backup;code:319;type:select;len:10;val:enable/disable;;"
	"View;code:311;type:wlist;words:2;len:64;;"
	"ChangeView;code:317;type:llist;len:64;;"
    },
    {
	"depot",
	"Depot;code:251;rq;ro;len:32;;"
	"Owner;code:252;len:32;;"
	"Date;code:253;type:date;ro;len:20;;"
	"Description;code:254;type:text;len:128;;"
	"Type;code:255;rq;len:10;;"
	"Address;code:256;len:64;;"
	"Suffix;code:258;len:64;;"
	"StreamDepth;code:260;len:64;;"
	"Map;code:257;rq;len:64;;"
	"SpecMap;code:259;type:wlist;len:64;;"
    },
    {
	"group",
	"Group;code:401;rq;ro;len:32;;"
	"MaxResults;code:402;type:word;len:12;;"
	"MaxScanRows;code:403;type:word;len:12;;"
	"MaxLockTime;code:407;type:word;len:12;;"
	"MaxOpenFiles;code:413;type:word;len:12;;"
	"Timeout;code:406;type:word;len:12;;"
	"PasswordTimeout;code:409;type:word;len:12;;"
	"LdapConfig;code:410;type:line;len:128;;"
	"LdapSearchQuery;code:411;type:line;len:128;;"
	"LdapUserAttribute;code:412;type:line;len:128;;"
    "LdapUserDNAttribute;code:414;type:line;len:128;;"
	"Subgroups;code:404;type:wlist;len:32;opt:default;;"
	"Owners;code:408;type:wlist;len:32;opt:default;;"
	"Users;code:405;type:wlist;len:32;opt:default;;"
    },
    {
	"job",
	"Job;code:101;rq;len:32;;"
	"Status;code:102;type:select;rq;len:10;"
	"pre:open;val:open/suspended/closed;;"
	"User;code:103;rq;len:32;pre:$user;;"
	"Date;code:104;type:date;ro;len:20;pre:$now;;"
	"Description;code:105;type:text;rq;pre:$blank;;"
    },
    {
	"label",
	"Label;code:301;rq;ro;fmt:L;len:32;;"
	"Update;code:302;type:date;ro;fmt:L;len:20;;"
	"Access;code:303;type:date;ro;fmt:L;len:20;;"
	"Owner;code:304;fmt:R;len:32;;"
	"Description;code:306;type:text;len:128;;"
	"Options;code:309;type:line;len:64;val:"
	"unlocked/locked,noautoreload/autoreload;;"
	"Revision;code:312;type:word;words:1;len:64;;"
	"ServerID;code:315;type:line;ro;len:64;;"
	"View;code:311;type:wlist;len:64;;"
    },
    {
	"ldap",
	"Name;code:801;rq;len:32;;"
	"Host;code:802;rq;type:word;words:1;len:128;;"
	"Port;code:803;rq;type:word;words:1;len:5;;"
	"Encryption;code:804;rq;len:10;val:"
	"none/ssl/tls;;"
	"BindMethod;code:805;rq;len:10;val:"
	"simple/search/sasl;;"
	"Options;code:816;type:line;len:64;val:"
	"nodowncase/downcase,nogetattrs/getattrs,"
	"norealminusername/realminusername;;"
	"SimplePattern;code:806;type:line;len:128;;"
	"SearchBaseDN;code:807;type:line;len:128;;"
	"SearchFilter;code:808;type:line;len:128;;"
	"SearchScope;code:809;len:10;val:"
	"baseonly/children/subtree;;"
	"SearchBindDN;code:810;type:line;len:128;;"
	"SearchPasswd;code:811;type:line;len:128;;"
	"SaslRealm;code:812;type:word;words:1;len:128;;"
	"GroupBaseDN;code:813;type:line;len:128;;"
	"GroupSearchFilter;code:814;type:line;len:128;;"
	"GroupSearchScope;code:815;len:10;val:"
	"baseonly/children/subtree;;"
	"AttributeUid;code:817;type:word;len:128;;"
	"AttributeName;code:818;type:line;len:128;;"
	"AttributeEmail;code:819;type:word;len:128;;"
    },
    {
	"license",
	"License;code:451;len:32;;"
	"License-Expires;code:452;len:10;;"
	"Support-Expires;code:453;len:10;;"
	"Customer;code:454;type:line;len:128;;"
	"Application;code:455;len:32;;"
	"IPaddress;code:456;len:24;;"
	"IPservice;code:461;type:wlist;len:24;;"
	"Platform;code:457;len:32;;"
	"Clients;code:458;len:8;;"
	"Users;code:459;len:8;;"
	"Files;code:460;len:8;;"
        "Repos;code:462;len:8;;"
    },
    {
	"protect",
	"SubPath;code:502;ro;len:64;;"
    "Update;code:503;type:date;ro;fmt:L;len:20;;"
    "Protections;code:501;fmt:C;type:wlist;words:5;opt:default;z;len:64;;"
    },
    {
	"remote",
	"RemoteID;code:851;rq;ro;fmt:L;len:32;;"
	"Address;code:852;rq;type:line;len:32;;"
	"Owner;code:853;fmt:R;len:32;;"
	"RemoteUser;code:861;fmt:R;len:32;;"
	"Options;code:854;type:line;len:32;val:"
	"unlocked/locked,nocompress/compress,copyrcs/nocopyrcs;;"
	"Update;code:855;type:date;ro;fmt:L;len:20;;"
	"Access;code:856;type:date;ro;fmt:L;len:20;;"
	"Description;code:857;type:text;len:128;;"
	"LastFetch;code:858;fmt:L;len:10;;"
	"LastPush;code:859;fmt:L;len:10;;"
	"DepotMap;code:860;type:wlist;words:2;len:64;;"
	"ArchiveLimits;code:862;type:wlist;words:2;len:64;;"
    },
    {
    "repo",
    "Repo;code:1001;rq;ro;len:128;;"
    "Owner;code:1002;len:32;;"
    "Created;code:1003;type:date;ro;fmt:L;len:20;;"
    "Pushed;code:1004;type:date;ro;fmt:L;len:20;;"
    "ForkedFrom;code:1005;ro;len:128;;"
    "Description;code:1006;type:text;len:128;;"
    "DefaultBranch;code:1007;len:32;;"
    "MirroredFrom;code:1008;len:32;;"
    "Options;code:1009;type:select;len:10;val:lfs/nolfs;;"
    "GconnMirrorServerId;code:1010;len:32;;"
    },
    {
	"server",
	"ServerID;code:751;rq;ro;len:32;;"
	"Type;code:752;rq;len:32;;"
	"Name;code:753;type:line;len:32;;"
	"Address;code:754;type:line;len:32;;"
	"ExternalAddress;code:755;type:line;len:32;;"
	"Services;code:756;rq;len:128;;"
    "Options;code:764;type:line;len:32;val:"
    "nomandatory/mandatory;;"
    "ReplicatingFrom;code:765;type:line;len:32;;"
	"Description;code:757;type:text;len:128;;"
	"User;code:761;type:line;len:64;;"
    "AllowedAddresses;code:763;type:wlist;len:64;;"
    "UpdateCachedRepos;code:766;type:wlist;len:64;;"
	"ClientDataFilter;code:758;type:wlist;len:64;;"
	"RevisionDataFilter;code:759;type:wlist;len:64;;"
	"ArchiveDataFilter;code:760;type:wlist;len:64;;"
	"DistributedConfig;code:762;type:text;len:128;;"
    },
    {
	"spec",
	"Fields;code:351;type:wlist;words:5;rq;;"
	"Words;code:352;type:wlist;words:2;;"
	"Formats;code:353;type:wlist;words:3;;"
	"Values;code:354;type:wlist;words:2;;"
	"Presets;code:355;type:wlist;words:2;;"
	"Openable;code:362;type:wlist;words:2;;"
    "Maxwords;code:361;type:wlist;words:2;;"
	"Comments;code:356;type:text;;"
    },
    {
	"stream",
	"Stream;code:701;rq;ro;len:64;;"
	"Update;code:705;type:date;ro;fmt:L;len:20;;"
	"Access;code:706;type:date;ro;fmt:L;len:20;;"
    "Owner;code:704;len:32;open:isolate;;"
    "Name;code:703;rq;type:line;len:32;open:isolate;;"
	"Parent;code:702;rq;len:64;open:isolate;;"
	"Type;code:708;rq;len:32;open:isolate;;"
	"Description;code:709;type:text;len:128;open:isolate;;"
	"Options;code:707;type:line;len:64;val:"
	"allsubmit/ownersubmit,unlocked/locked,"
	"toparent/notoparent,fromparent/nofromparent,"
    "mergedown/mergeany;open:isolate;;"
	"Paths;code:710;rq;type:wlist;words:2;maxwords:3;len:64;open:propagate;fmt:C;;"
	"Remapped;code:711;type:wlist;words:2;len:64;open:propagate;fmt:C;;"
	"Ignored;code:712;type:wlist;words:1;len:64;open:propagate;fmt:C;;"
	"View;code:713;type:wlist;words:2;len:64;;"
	"ChangeView;code:714;type:llist;ro;len:64;;"
    },
    {
	"triggers",
	"Triggers;code:551;type:wlist;words:4;len:64;opt:default;z;;"
    },
    {
	"typemap",
	"TypeMap;code:601;type:wlist;words:2;len:64;opt:default;z;;"
    },
    {
	"user",
	"User;code:651;rq;ro;seq:1;len:32;;"
	"Type;code:659;ro;fmt:R;len:10;;"
	"Email;code:652;fmt:R;rq;seq:3;len:32;;"
	"Update;code:653;fmt:L;type:date;ro;seq:2;len:20;;"
	"Access;code:654;fmt:L;type:date;ro;len:20;;"
	"FullName;code:655;fmt:R;type:line;rq;len:32;;"
	"JobView;code:656;type:line;len:64;;"
	"Password;code:657;len:32;;"
    "AuthMethod;code:662;fmt:L;len:10;val:"
    "perforce/perforce+2fa/ldap/ldap+2fa;;"
	"Reviews;code:658;type:wlist;len:64;;"
    },
    { 0, 0}
};

SpecMgr::SpecMgr()
{
    debug = 0;
    specs = 0;

    // Enable expansion of numeric sequences by default.
    expandSequences = true;

    Reset();
}

SpecMgr::~SpecMgr()
{
    delete specs;
}

void
SpecMgr::AddSpecDef( const char *type, StrPtr &specDef )
{
    if ( specs->GetVar( type ) )
        specs->RemoveVar( type );
    specs->SetVar( type, specDef );
}

void
SpecMgr::AddSpecDef( const char *type, const char *specDef )
{
    if ( specs->GetVar( type ) )
        specs->RemoveVar( type );
    specs->SetVar( type, specDef );
}

void
SpecMgr::Reset()
{
    delete specs;
    specs = new StrBufDict;

    for ( struct defaultspec *sp = &speclist[ 0 ]; sp->type; sp++ )
        AddSpecDef( sp->type, sp->spec );
}

int
SpecMgr::HaveSpecDef( const char *type )
{
    return specs->GetVar( type ) != 0;
}

//
// Convert a Perforce StrDict into a PHP hash. Convert multi-level
// data (Files0, Files1, etc. ) into (nested) array members of the hash.
//

void
SpecMgr::StrDictToHash( StrDict *dict, zval *retval )
{
    StrRef  var, val;
    int     i;

    if ( PHP_PERFORCE_DEBUG_CALLS )
        fprintf( stderr, "[P4] StrDictToHash()\n" );

    array_init( retval );

    for ( i = 0; dict->GetVar( i, var, val ); i++ )
    {
        if ( var == "specdef" || var == "func" || var == "specFormatted" )
            continue;

        InsertItem( retval, &var, &val );
    }
}

//
// Convert a Perforce StrDict into a P4.Spec object
//
// TODO: convert this to use a SpecData object instead (might require rewrite)

void
SpecMgr::StrDictToSpec( StrDict *dict, StrPtr *specDef, zval *retval )
{
    StrRef  var, val;
    int     i;

    if( PHP_PERFORCE_DEBUG_CALLS )
        fprintf( stderr, "[P4] StrDictToSpec()\n" );

    array_init( retval );

    // create a few variables to hold the spec definition
    // a spec element and any errors that occur
    Error       e;
    SpecElem*   se;
    Spec        s( specDef->Text(), "", &e );

    for ( i = 0; dict->GetVar( i, var, val ); i++ )
    {
        if ( var == "specdef" || var == "func" || var == "specFormatted" )
            continue;

        // if we are expanding sequences and have successfully
        // fetched the spec definition, skip expansion for
        // spec fields that legitimately end in a digit
        if (GetExpandSequences()
            && !e.Test()
            && var.Length() > 1 && isdigit(*(var.End()-1))
            && (se = s.Find(var))
        ) {
            SetExpandSequences(false);
            InsertItem( retval, &var, &val );
            SetExpandSequences(true);

            continue;
        }

        InsertItem( retval, &var, &val );
    }

}

//
// Parse routine: converts a string into a PHP hash.
//

void
SpecMgr::StringToSpec( const char *type, const char *form, Error *e, zval *retval )
{
    SpecDataTable   specData;
    StrPtr *        specDef = specs->GetVar( type );

    Spec        s( specDef->Text(), "", e );

    if ( !e->Test() )
        s.ParseNoValid( form, &specData, e );

    if ( e->Test() )
    {
        ZVAL_NULL( retval );
        return;
    }

    StrDictToSpec( specData.Dict(), specDef, retval );
}

//
// Format routine, updates a StrBuf object with the form content.
// The StrBuf can then be converted to a PHP string where required.
//

void
SpecMgr::SpecToString( const char *type, zval *hash, StrBuf &b, Error *e )
{
    StrPtr *specDef = specs->GetVar( type );

    if ( !specDef )
    {
        e->Set( E_FAILED, "No specdef available. Cannot convert hash to a "
                 "Perforce form" );
        return;
    }

    SpecDataTable specData;

    Spec s( specDef->Text(), "", e );
    if ( e->Test() ) return;

    int cur;
    zend_string * key;
    unsigned int key_len;
    unsigned long index;
    zval *elem, *subval;
    HashTable *ht, *ht2;
    StrBuf os, eStr; //eStr added by Karl

    TSRMLS_FETCH();

    ht = Z_ARRVAL_P( hash );

    ZEND_HASH_FOREACH_KEY_VAL(ht, index, key, elem)
    {
        if ( Z_TYPE_P(elem) == IS_REFERENCE ) {
            elem = Z_REFVAL_P( elem );
        }

        switch ( Z_TYPE_P( elem ) ) {

        case IS_ARRAY:

            // Need to flatten the list
            ht2 = Z_ARRVAL_P( elem );
            for (int idx2 = 0; idx2 < zend_hash_num_elements( ht2 ); idx2++ ) {
                subval = zend_hash_index_find(ht2, idx2);
                os.Clear();

                os << key->val << idx2;
                if ( Z_TYPE_P(subval) == IS_REFERENCE ) {
                    subval = Z_REFVAL_P( subval );
                }
                if ( Z_TYPE_P(subval) != IS_STRING ) {
                    eStr.Set("Encountered non-string value while parsing spec ");
                    eStr << StrNum( Z_TYPE_P(subval) );
                    php_error( E_WARNING, eStr.Text(), 1 );
                    continue;
                }
                specData.Dict()->SetVar( os.Text(), Z_STRVAL_P(subval) );

                if ( PHP_PERFORCE_DEBUG_DATA )
                {                    
                    eStr.Clear();
                    eStr << "... " << os << " -> " << Z_STRVAL_P( subval ) << "\n";
                    fprintf( stderr, "%s", eStr.Text() );
                }
            }

            break;

        case IS_STRING:
            specData.Dict()->SetVar( key->val, Z_STRVAL_P( elem ) );
	    if( PHP_PERFORCE_DEBUG_DATA )
	    {
                eStr.Clear();
                eStr << "... " << key->val << Z_STRVAL_P( elem ) << "\n";
                fprintf( stderr, "%s", eStr.Text() );
            }
            break;

        }
    } ZEND_HASH_FOREACH_END();

    s.Format( &specData, &b );
}

//
// This method returns a hash describing the valid fields in the spec. To
// make it easy on our users, we map the lowercase name to the name defined
// in the spec (with an underscore in front).
// Thus, the users can always user lowercase, and if the field
// should be in mixed case, it will be.
//

void
SpecMgr::SpecFields( const char *type, zval *retval )
{
    return SpecFields( specs->GetVar( type ), retval );
}

void
SpecMgr::SpecFields( StrPtr *specDef, zval *retval )
{
    if ( !specDef )
    {
        ZVAL_NULL( retval );
        return;
    }

    //
    // There's no trivial way to do this using the API (and get it right), so
    // for now, we parse the string manually. We're ignoring the type of
    // the field, and any constraints it may be under; what we're interested
    // in is solely the field name
    //

    array_init( retval );

    const char *b, *e;
    const char *sep = ";";
    const char *fsep = ";;";
    const char *seek = sep;

    for ( e = b = specDef->Text(); e && b ; )
    {
        e = strstr( b, seek );
        if ( e && seek == sep )
        {
            StrBuf k;
            k.Set( b, e - b );

            StrBuf v(k);
            StrOps::Lower( k );

            add_assoc_string( retval, k.Text(), v.Text() );

            b = ++e;
            seek = fsep;
        }
        else if ( e )
        {
            b = e += 2;
            seek = sep;
        }
    }
}

//
// Split a key into its base name and its index. i.e. for a key "how1,0"
// the base name is "how" and the index is "1,0". We work backwards from
// the end of the key looking for the first char that is neither a
// digit, nor a coma.
//

void
SpecMgr::SplitKey( const StrPtr *key, StrBuf &base, StrBuf &index )
{
    int i = 0;

    // Don't break off the index if expand sequences is disabled.
    if ( GetExpandSequences() == false ) {
        base.Set( key->Text(), key->Length() );
        index.Set( StrBuf("") );
        return;
    }

    base = *key;
    index = "";
    for ( i = key->Length(); i; i-- )
    {
        char   prev = (*key)[ i-1 ];
        if ( !isdigit( prev ) && prev != ',' )
        {
            base.Set( key->Text(), i );
            index.Set( key->Text() + i );
            break;
        }
    }
}

//
// Insert an element into the response structure. The element may need to
// be inserted into an array nested deeply within the enclosing hash.
//

void
SpecMgr::InsertItem( zval *hash, const StrPtr *var, const StrPtr *val )
{
    zval    ary;
    zval    *data;
    StrBuf  base, index;
    StrRef  comma( "," );

    TSRMLS_FETCH();

    HashTable * ht = Z_ARRVAL_P( hash );
    SplitKey( var, base, index );

    if ( PHP_PERFORCE_DEBUG ) {
        StrBuf eStr;
        eStr << "[P4] {SpecMgr::InsertItem} \n";
        eStr << "[P4] {SpecMgr::InsertItem} Var: \n"
             << var->Text() << ", val: " << val->Text() << "\n";
        eStr << "[P4] {SpecMgr::InsertItem} SplitKey(var, base, index)\n";
        eStr << "[P4] {SpecMgr::InsertItem} base.Text(): "
             << base.Text() << ", index.Text(): " << index.Text() << "\n";
        fprintf( stderr, "%s", eStr.Text() );
    }

    // If there's no index, then we insert into the top level hash
    // but if the key is already defined then we need to rename the key. This
    // is probably one of those special keys like otherOpen which can be
    // both an array element and a scalar. The scalar comes last, so we
    // just rename it to "otherOpens" to avoid trashing the previous key
    // value
    if ( index == "" )
    {
        StrBuf key( *var );
        if ( zend_hash_str_exists( ht, var->Text(), var->Length() ) )
            key << "s";
        add_assoc_stringl( hash, key.Text(), val->Text(), val->Length());
        if ( PHP_PERFORCE_DEBUG ) {
            StrBuf eStr;
            eStr << "[P4] {SpecMgr::InsertItem} Adding top-level: "
                 << var->Text() << ": " << val->Text() << "\n";
            fprintf( stderr, "%s", eStr.Text() );
        }
        return;
    }

    //
    // Get or create the parent array from the hash.
    //
    if ( (data = zend_hash_str_find( ht, base.Text(), base.Length())) == NULL )
    {
        array_init( &ary );
        add_assoc_zval( hash, base.Text(), &ary );
        if ( PHP_PERFORCE_DEBUG ) {
	    StrBuf eStr;
            eStr << "[P4] {SpecMgr::InsertItem} Adding empty array\n";
            fprintf( stderr, "%s", eStr.Text() );
        }
    }
    else
    {
        if ( PHP_PERFORCE_DEBUG ) {
	    StrBuf eStr;
            eStr << "[P4] {SpecMgr::InsertItem} Found " << base.Text() << " in hash\n";
            fprintf( stderr, "%s", eStr.Text() );
        }
        ary = *data;
    }

    // Ensure that the parent array is in fact an array.
    // If it is not, just store value (val) under original name (var)
    if ( Z_TYPE( ary ) != IS_ARRAY )
    {
        add_assoc_stringl( hash, var->Text(), val->Text(), val->Length());
        if ( PHP_PERFORCE_DEBUG ) {
	    StrBuf eStr;
            eStr << "[P4] {SpecMgr::InsertItem} Adding " <<
                var->Text() << "=" << val->Text() << " and returning\n";
            fprintf( stderr, "%s", eStr.Text() );
        }
        return;
    }

    // The index may be a simple digit, or it could be a comma separated
    // list of digits. For each "level" in the index, we need a containing
    // array.
    for ( const char *c = 0; ( c = index.Contains( comma ) ); )
    {
        StrBuf level;
        level.Set( index.Text(), c - index.Text() );
        index.Set( c + 1 );

        // Found another level so we need to get/create a nested array
        // under the current entry. We use the level as an index so that
        // missing entries are left empty deliberately.
        int levelValue = level.Atoi();
        zval tlist;

        if ( PHP_PERFORCE_DEBUG ) {
            StrBuf eStr;
            eStr << "[P4] {SpecMgr::InsertItem} levelValue=" << levelValue << "\n";
            eStr << "[P4] {SpecMgr::InsertItem} zend_hash_num_elements( Z_ARRVAL_P( ary ) )="
                 << zend_hash_num_elements( Z_ARRVAL( ary ) ) << "\n";
            fprintf( stderr, "%s", eStr.Text() );
        }

        if ( levelValue >= zend_hash_num_elements( Z_ARRVAL( ary ) ) ) {
            for (int i = zend_hash_num_elements( Z_ARRVAL( ary ) ); i < levelValue; i++) {
                add_index_null( &ary, i );
            }
            array_init( &tlist );
            add_next_index_zval( &ary, &tlist );
            ary = tlist;
        } else {
            zval *tmp;
            if ( (tmp = zend_hash_index_find( Z_ARRVAL( ary ), levelValue)) == NULL )
            {
                array_init( &tlist );
                add_next_index_zval( &ary, &tlist );
            } else {
                ary = *tmp;
            }
        }
    }

    // Fill in any intervening keys that are missing with null.
    for ( int i = index.Atoi(); i > zend_hash_num_elements( Z_ARRVAL( ary ) ); ) {
        add_next_index_null( &ary );
    }

    add_next_index_string( &ary, val->Text() );
}
