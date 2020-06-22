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
#include "php_mergedata.h"

PHPMergeData::PHPMergeData( ClientUser *ui, ClientMerge *m, StrPtr &hint )
{
    this->debug = 0;
    this->ui = ui;
    this->merger = m;
    this->hint = hint;

    // Extract (forcibly) the paths from the RPC buffer.
    StrPtr *t;
    if( ( t = ui->varList->GetVar( "baseName" ) ) ) base = t->Text();
    if( ( t = ui->varList->GetVar( "yourName" ) ) ) yours = t->Text();
    if( ( t = ui->varList->GetVar( "theirName" ) ) ) theirs = t->Text();

}

void PHPMergeData::GetYourName(zval *return_value)
{
    char *yoursStr = yours.Text();
    ZVAL_STRINGL(return_value, yoursStr, strlen(yoursStr));
}

void PHPMergeData::GetTheirName(zval *return_value)
{
    char *theirsStr = theirs.Text();
    ZVAL_STRINGL(return_value, theirsStr, strlen(theirsStr));
}

void PHPMergeData::GetBaseName(zval *return_value)
{
    char *baseStr = base.Text();
    ZVAL_STRINGL(return_value, baseStr, strlen(baseStr));
}

void PHPMergeData::GetYourPath(zval *return_value)
{
    char *yourFileName = merger->GetYourFile()->Name();
    ZVAL_STRINGL(return_value, yourFileName, strlen(yourFileName));
}

void PHPMergeData::GetTheirPath(zval *return_value)
{
    char *theirFileName = merger->GetTheirFile()->Name();
    ZVAL_STRINGL(return_value, theirFileName, strlen(theirFileName));
}

void PHPMergeData::GetBasePath(zval *return_value)
{
    char *baseFileName = merger->GetBaseFile()->Name();
    ZVAL_STRINGL(return_value, baseFileName, strlen(baseFileName));
}

void PHPMergeData::GetResultPath(zval *return_value)
{
    char *resultFileName = merger->GetResultFile()->Name();
    ZVAL_STRINGL(return_value, resultFileName, strlen(resultFileName));
}

void PHPMergeData::GetMergeHint(zval *return_value)
{
    char *hintStr = hint.Text();
    ZVAL_STRINGL(return_value, hintStr, strlen(hintStr));
}

void PHPMergeData::RunMergeTool(zval *return_value)
{
    Error e;
    ui->Merge( merger->GetBaseFile(), merger->GetTheirFile(),
               merger->GetYourFile(), merger->GetResultFile(), &e );

    if( e.Test() )
    {
        ZVAL_FALSE(return_value);
    }
    else
    {
        ZVAL_TRUE(return_value);
    }
}
