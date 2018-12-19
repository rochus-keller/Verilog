#ifndef VLFRONTEND_H
#define VLFRONTEND_H

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

#include <QObject>
#include <QHash>
#include <QDir>
#include <QStack>
#include <QSet>

namespace Vl
{
    class Preprocessor;
    class PpLexer;
    class Parser;
    class PpSymbols;
    class Errors;
    class Includes;

    class Frontend : public QObject
    {
        Q_OBJECT
    public:
        explicit Frontend(QObject *parent = 0);

        bool process( const QString& file );

    private:
        struct Work
        {
            QString d_file;
            Preprocessor* d_pp;
            PpLexer* d_lex;
            Parser* d_par;
            Errors* d_err;
        };
        PpSymbols* d_ppSyms;
        Includes* d_incs;
    };
}

#endif // VLFRONTEND_H
