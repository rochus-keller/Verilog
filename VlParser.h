#ifndef PARSER_H
#define PARSER_H

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
#include <Verilog/VlSynTree.h>

namespace Vl
{
    class PpLexer;
    class Errors;

    class Parser : public QObject
    {
    public:
        Parser(QObject* = 0);
        ~Parser();

        bool parseFile(Vl::PpLexer*, Errors* = 0 );

        QList<SynTree*> getResult(bool transfer = true)
        {
            QList<SynTree*> res = d_st;
            if( transfer )
                d_st.clear();
            return res;
        }
        const QList<Vl::Token>& getSections() const { return d_sections; }

        static void dumpTree( SynTree*, int level = 0 );
    protected:
        void clear();

    private:
        QList<SynTree*> d_st;
        QList<Vl::Token> d_sections;
    };
}

#endif // PARSER_H
