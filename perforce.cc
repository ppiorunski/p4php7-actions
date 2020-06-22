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

#define PHP_PERFORCE_EXTVER   "0.1"
#define PHP_PERFORCE_EXTNAME  "perforce"


#include <clientapi.h>
#include <ident.h>
#include <strtable.h>

#include "undefdups.h"

extern "C"
{
    #include "php.h"
}
#include "php_macros.h"
#include "ext/standard/info.h"


#include "specmgr.h"
#include "php_p4result.h"
#include "php_clientuser.h"
#include "php_clientapi.h"
#include "php_mergedata.h"
#include "php_p4_depotfile.h"
#include "php_p4_exception.h"
#include "php_p4_integration.h"
#include "php_p4_map.h"
#include "php_p4_mergedata.h"
#include "php_p4_resolver.h"
#include "php_p4_output_handler.h"
#include "php_p4_revision.h"

#include "php_p4.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

Ident ident = {
    IdentMagic "P4PHP" "/" ID_OS "/" ID_REL "/" ID_PATCH " (" ID_API " API)",
    ID_Y "/" ID_M "/" ID_D
};

/* Module Initialization */
PHP_MINIT_FUNCTION(perforce)
{
    /* Register P4 Classes */
    register_p4_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_depotfile_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_exception_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_integration_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_map_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_mergedata_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_resolver_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_revision_class(INIT_FUNC_ARGS_PASSTHRU);
    register_p4_output_handler(INIT_FUNC_ARGS_PASSTHRU);

    return SUCCESS;
}

/* Module Information */
PHP_MINFO_FUNCTION(perforce)
{
    StrBuf s;
    ident.GetMessage(&s);
    php_info_print_table_start();
    php_info_print_table_row(2, "Perforce Module", "enabled");
    php_info_print_table_row(2, "Version Information", s.Text());
    php_info_print_table_end();
}

BEGIN_EXTERN_C()
/* Module entry structure. */
	zend_module_entry perforce_module_entry = {
	#if ZEND_MODULE_API_NO >= 20010901
		STANDARD_MODULE_HEADER,
	#endif
		PHP_PERFORCE_EXTNAME,
		NULL,                 /* Functions */
		PHP_MINIT(perforce),  /* MINIT */
		NULL,                 /* MSHUTDOWN */
		NULL,                 /* RINIT */
		NULL,                 /* RSHUTDOWN */
		PHP_MINFO(perforce),  /* MINFO */
	#if ZEND_MODULE_API_NO >= 20010901
		PHP_PERFORCE_EXTVER,
	#endif
		STANDARD_MODULE_PROPERTIES
	};
END_EXTERN_C()

#ifdef COMPILE_DL_PERFORCE
ZEND_GET_MODULE(perforce)
#endif
