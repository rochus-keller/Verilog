/*
* Copyright 2018 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Verilog parser library.
*
* The following is the license that applies to this copy of the
* library. For a license to use the library under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include "VlFrontend.h"
#include "VlPreprocessor.h"
#include "VlPpLexer.h"
#include "VlPpSymbols.h"
#include "VlParser.h"
#include "VlErrors.h"
#include "VlIncludes.h"
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QElapsedTimer>
using namespace Vl;

Frontend::Frontend(QObject *parent) : QObject(parent),d_incs(0)
{
    d_ppSyms = new PpSymbols(this);

    // TEST e200
    d_ppSyms->addSymbol( "DISABLE_SV_ASSERTION", "", Tok_String );
    d_ppSyms->addSymbol( "E203_MTVEC_TRAP_BASE", "0", Tok_Natural );
    d_ppSyms->addSymbol( "E203_ITCM_DATA_WIDTH_IS_32", "1", Tok_Natural );
    // TEST oh
    d_ppSyms->addSymbol( "CFG_ASIC", "1", Tok_Natural );
    d_ppSyms->addSymbol( "CFG_PLATFORM", "", Tok_String );

    d_incs = new Includes(this);
    d_incs->setAutoAdd(true);
}

bool Vl::Frontend::process(const QString& file)
{
    QElapsedTimer t;
    t.start();
    Work w;
    w.d_err = new Errors(this);
    w.d_err->setShowWarnings(false);
    w.d_lex = new PpLexer(this);
    w.d_lex->setErrors(w.d_err);
    w.d_lex->setSyms(d_ppSyms);
    w.d_lex->setIncs(d_incs);
    w.d_lex->setStream(file, true);
    w.d_par = new Parser(this);
    w.d_file = file;
    const bool res = w.d_par->parseFile(w.d_lex,w.d_err);
    if( w.d_err->getErrCount() != 0 && w.d_err->getWrnCount() != 0 )
        qDebug() << t.elapsed() << "####" << w.d_err->getErrCount() << "Errors and" <<
                    w.d_err->getWrnCount() << "Warnings in" << file;
    else if( w.d_err->showWarnings() && w.d_err->getWrnCount() != 0 )
        qDebug() << t.elapsed() << "####" << w.d_err->getWrnCount() << "Warnings in" << file;
    else if( w.d_err->getErrCount() != 0)
        qDebug() << t.elapsed() << "####" << w.d_err->getErrCount() << "Errors in" << file;
//    else
//        qDebug() << t.elapsed() << "#### No Warnings or Errors in" << file;
    return res;
}


