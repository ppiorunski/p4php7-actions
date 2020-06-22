AC_PROG_CXX

PHP_ARG_WITH(perforce, for Perforce Support,
    [  --with-perforce[=DIR]     Include Perforce support. DIR is the p4api base directory])

PHP_ARG_ENABLE(openssl-dynamic, dynamically link P4PHP with system OpenSSL libs,
    [  --enable-openssl-dynamic     Dynamically link P4PHP with system openssl libraries], no, no)

PHP_ARG_ENABLE(openssl-static, statically link P4PHP with OpenSSL libs in P4API lib dir,
    [  --enable-openssl-static     Statically link P4PHP with user-provided OpenSSL libraries in P4API/lib], no, no)

if test "$PHP_PERFORCE" != "no"; then
    PHP_SUBST(PERFORCE_SHARED_LIBADD)
    PHP_NEW_EXTENSION(perforce, specmgr.cc php_clientapi.cc php_mergedata.cc \
        php_clientuser.cc php_p4result.cc php_p4.cc \
        php_p4_depotfile.cc php_p4_map.cc php_p4mapmaker.cc \
        php_p4_exception.cc php_p4_revision.cc php_p4_integration.cc \
        php_p4_resolver.cc php_p4_output_handler.cc php_p4_mergedata.cc \
        perforce.cc, $ext_shared)

    dnl this is used to perform any shell expansion necessary (such as dealing with '~')
    PHP_PERFORCE_DIR=`cd "$PHP_PERFORCE" && pwd`

    dnl look for the p4api include directory
    for i in "$PHP_PERFORCE_DIR" /usr/local /usr /opt; do
        if test -r "$i/include/p4/clientapi.h"; then
            PERFORCE_DIR="$i"
            PERFORCE_INC_DIR="$i/include/p4"
            PERFORCE_LIB_DIR="$i/lib"
            break
        fi
    done

    dnl check dirs
    AC_MSG_CHECKING([(P4PHP) for P4API directory])
    if [ test -z "$PERFORCE_DIR" && ! test -d "$PERFORCE_DIR" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) bad P4API directory])
    fi
    AC_MSG_RESULT([yes])

    AC_MSG_CHECKING([(P4PHP) for P4API include directory])
    if [ test -z "$PERFORCE_INC_DIR" && ! test -d "$PERFORCE_INC_DIR" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) bad P4API include directory])
    fi
    AC_MSG_RESULT([yes])

    AC_MSG_CHECKING([(P4PHP) for P4API lib directory])
    if [ test -z "$PERFORCE_LIB_DIR" && ! test -d "$PERFORCE_LIB_DIR" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) bad P4API lib directory])
    fi
    AC_MSG_RESULT([yes])

    dnl add p4api include
    PHP_ADD_INCLUDE($PERFORCE_INC_DIR)

    dnl other checks
    API_LIBCLIENT="$PERFORCE_LIB_DIR/libclient.a"
    AC_MSG_CHECKING([(P4PHP) for P4API libclient.a])
    if [ ! test -r "$API_LIBCLIENT" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) cannot find P4API client library ($API_LIBCLIENT)])
    fi
    AC_MSG_RESULT([yes])
    API_LIBSUPP="$PERFORCE_LIB_DIR/libsupp.a"
    AC_MSG_CHECKING([(P4PHP) for P4API libsupp.a])
    if [ ! test -r "$API_LIBSUPP" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) cannot find P4API Supplemental library ($API_LIBSUPP)])
    fi
    AC_MSG_RESULT([yes])
    API_LIBRPC="$PERFORCE_LIB_DIR/librpc.a"
    AC_MSG_CHECKING([(P4PHP) for P4API librpc.a])
    if [ ! test -r "$API_LIBRPC" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) cannot find P4API RPC library ($API_LIBRPC)])
    fi
    AC_MSG_RESULT([yes])

    dnl Check to see if we include OpenSSL libraries or not
    OPENSSL="no"

    dnl if we're statically linking, check that libssl.a and libcrypto.a
    dnl are present in P4API lib dir
    if test "$PHP_OPENSSL_STATIC" != "no"; then
        AC_MSG_CHECKING([(P4PHP) for P4API libssl.a and libcrypto.a])
        if [ ! test -r "$PERFORCE_LIB_DIR/libssl.a" ]; then
            AC_MSG_RESULT([no, error])
            AC_MSG_ERROR([(P4PHP) cannot find libssl.a in ($PERFORCE_LIB_DIR), required for --enable-openssl-static])
        fi
        if [ ! test -r "$PERFORCE_LIB_DIR/libcrypto.a" ]; then
            AC_MSG_RESULT([no, error])
            AC_MSG_ERROR([(P4PHP) cannot find libcrypto.a in ($PERFORCE_LIB_DIR), required for --enable-openssl-static])
        fi
        OPENSSL="yes"
    fi

    dnl if we're dynamically linking, just set OPENSSL to "yes"
    if test "$PHP_OPENSSL_DYNAMIC" != "no"; then
        OPENSSL="yes"
    fi

    API_VERSION_FILE="$PERFORCE_DIR/sample/Version"
    AC_MSG_CHECKING([(P4PHP) for P4API sample/Version])
    if [ ! test -r "$API_VERSION_FILE" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) cannot find P4API Version file ($API_VERSION_FILE)])
    fi
    AC_MSG_RESULT([yes])

    dnl get information about p4api version
    ID_API_REL=`cat "$PERFORCE_DIR/sample/Version" | grep "^RELEASE" | sed -e "s/.* = \(.*\) ;.*/\1/;s/ /./g"`
    ID_API_PATCH=`cat "$PERFORCE_DIR/sample/Version" | grep "^PATCHLEVEL" | sed -e "s/.* = \(.*\) ;.*/\1/;s/ /./g"`
    ID_API_OS=`strings "$API_LIBCLIENT" | grep "^@(#)P4API" | awk -F/ '{print $2}'`

    AC_MSG_CHECKING([(P4PHP) P4API Version file has RELEASE])
    if test -z "$ID_API_REL"; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) RELEASE not found in P4API Version file ($API_VERSION_FILE)])
    fi
    AC_MSG_RESULT([$ID_API_REL])

    AC_MSG_CHECKING([(P4PHP) P4API Version file has PATCHLEVEL])
    if test -z "$ID_API_PATCH"; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) PATCHLEVEL not found in P4API Version file ($API_VERSION_FILE)])
    fi
    AC_MSG_RESULT([$ID_API_PATCH])

    AC_MSG_CHECKING([(P4PHP) P4API embedded version string OS portion])
    if test -z "$ID_API_OS"; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) OS portion not found in P4API libclient.a embedded version string])
    fi
    AC_MSG_RESULT([$ID_API_OS])

    dnl check for Version file to use for P4PHP
    AC_MSG_CHECKING([(P4PHP) whether Version file exists in p4 source])
    if test -r ../p4/Version; then
        cp -f ../p4/Version Version
        chmod a+w Version
        AC_MSG_RESULT([copied])
    fi
    AC_MSG_RESULT([no])

    AC_MSG_CHECKING([(P4PHP) for Version file])
    if [ ! test -r Version ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) no Version file found])
    fi
    AC_MSG_RESULT([yes])

    dnl harvest fields from Version file
    ID_REL=`cat Version | grep "^RELEASE" | sed -e "s/.* = \(.*\) ;.*/\1/;s/ /./g"`
    ID_Y=`cat Version | grep "^SUPPDATE" | awk '{print $3}' | tr -d '\012'`
    ID_M=`cat Version | grep "^SUPPDATE" | awk '{print $4}' | tr -d '\012'`
    ID_D=`cat Version | grep "^SUPPDATE" | awk '{print $5}' | tr -d '\012'`
    ID_PATCH=`cat Version | grep "^PATCHLEVEL" | awk '{print $3}' | tr -d '\012'`

    AC_MSG_CHECKING([(P4PHP) Version file has RELEASE])
    if test -z "$ID_REL"; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) RELEASE not found in Version file])
    fi
    AC_MSG_RESULT([$ID_REL])

    AC_MSG_CHECKING([(P4PHP) Version file has SUPPDATE])
    if [ test -z "$ID_Y" || test -z "$ID_M" || test -z "$ID_D" ]; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) SUPPDATE not found in Version file])
    fi
    AC_MSG_RESULT([$ID_Y/$ID_M/$ID_D])

    AC_MSG_CHECKING([(P4PHP) Version file has PATCHLEVEL])
    if test -z "$ID_PATCH"; then
        AC_MSG_RESULT([no, error])
        AC_MSG_ERROR([(P4PHP) PATCHLEVEL not found in Version file])
    fi
    AC_MSG_RESULT([$ID_PATCH])

    dnl define preprocessor macros
    AC_DEFINE_UNQUOTED(ID_API,   "$ID_API_REL/$ID_API_PATCH",   [Setting API ID])
    AC_DEFINE_UNQUOTED(ID_REL,   "$ID_REL",   [Setting release ID])
    AC_DEFINE_UNQUOTED(ID_Y,     "$ID_Y",     [Setting build year])
    AC_DEFINE_UNQUOTED(ID_M,     "$ID_M",     [Setting build month])
    AC_DEFINE_UNQUOTED(ID_D,     "$ID_D",     [Setting build day])
    AC_DEFINE_UNQUOTED(ID_PATCH, "$ID_PATCH", [Setting the patch level])

    dnl determine OS
    AC_CANONICAL_HOST
    OS=${host}
    case "${host}" in
        dnl Cygwin
        i*86-*-cygwin* )
            OS="CYGWINX86"
        ;;
        dnl Darwin (Mac)
        *darwin* )
            CFLAGS="-framework CoreFoundation"
            AC_MSG_NOTICE([(P4PHP) CFLAGS=$CFLAGS])

            DARWIN_VER=`echo "${host}" | sed -e "s,^.*darwin\(.*\)\..*\..*$,\1,"`
            AC_MSG_NOTICE([(P4PHP) Darwin version: $DARWIN_VER])
            OS="DARWIN${DARWIN_VER}"
        ;;
        dnl Linux
        *linux* )
            PHP_ADD_LIBRARY(rt, 1, PERFORCE_SHARED_LIBADD)

            LINUX_VER=26
            AC_MSG_NOTICE([(P4PHP) Linux version: $LINUX_VER])

            case "${host}" in
                linux-gnu )
                    OS="LINUX${LINUX_VER}"
                ;;
                i*86-*-linux-* )
                    OS="LINUX${LINUX_VER}X86"
                ;;
                x86_64-*-linux* )
                    OS="LINUX${LINUX_VER}X86_64"
                ;;
            esac
        ;;
        dnl FreeBSD
        i*86-*-freebsd5* )
            OS="FREEBSD5X86"
        ;;
        x86_64-*-freebsd5* )
            OS="FREEBSD5X86_64"
        ;;
        i*86-*-freebsd6* )
            OS="FREEBSD6X86"
        ;;
        x86_64-*-freebsd6* )
            OS="FREEBSD6X86_64"
        ;;
        i*86-*-freebsd7* )
            OS="FREEBSD7X86"
        ;;
        x86_64-*-freebsd7* )
            OS="FREEBSD7X86_64"
        ;;
        i*86-*-freebsd8* )
            OS="FREEBSD8X86"
        ;;
        x86_64-*-freebsd8* )
            OS="FREEBSD8X86_64"
        ;;
        dnl Solaris
        sparc-*-solaris2.10 )
            OS="SOLARIS10SPARC"
        ;;
        i*86-*-solaris2.10 )
            OS="SOLARIS10X86"
        ;;
    esac
    AC_MSG_NOTICE([(P4PHP) OS portion of version string: $OS])

    AC_DEFINE_UNQUOTED(ID_OS, "${OS}", [Setting OS ID])

    dnl link p4api libraries
    PHP_ADD_LIBRARY_WITH_PATH(supp,   $PERFORCE_LIB_DIR, PERFORCE_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(rpc,    $PERFORCE_LIB_DIR, PERFORCE_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(client, $PERFORCE_LIB_DIR, PERFORCE_SHARED_LIBADD)

    dnl If OpenSSL enabled, add openssl libs
    if test "$OPENSSL" == "yes"; then
        PHP_ADD_LIBRARY(ssl,    1, PERFORCE_SHARED_LIBADD)
        PHP_ADD_LIBRARY(crypto, 1, PERFORCE_SHARED_LIBADD)
        if test "$PHP_OPENSSL_STATIC" != "no"; then
            AC_MSG_NOTICE([(P4PHP) OpenSSL libraries being linked from ($PERFORCE_LIB_DIR) (static)])
        else
            AC_MSG_NOTICE([(P4PHP) OpenSSL libraries being linked from system libs (dynamic)])
        fi
    else
        AC_MSG_NOTICE([(P4PHP) No OpenSSL being linked, are you sure this is right?])
    fi

    PHP_ADD_LIBRARY(stdc++, 1, PERFORCE_SHARED_LIBADD)
fi
