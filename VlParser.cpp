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

#include "VlParser.h"
#include "VlPpLexer.h"
#include "VlToken.h"
#include "VlErrors.h"
#include <QFileInfo>
#include <QtDebug>
#include "CocoParser.h"

extern int yyparse (void);

namespace Vl
{
static PpLexer* s_lex = 0;

Parser::Parser(QObject* p):QObject(p)
{
}

Parser::~Parser()
{
    clear();
}

bool Parser::parseFile(PpLexer* in, Errors* err)
{
    if( in == 0 )
        return false;
    s_lex = in;

#define _USE_COCO

#ifdef _USE_BISON
    return yyparse() == 0;
#endif
#ifdef _USE_PACKCC
    int ret;
    pcc_context_t *ctx = pcc_create(NULL);
    while (pcc_parse(ctx, &ret));
    pcc_destroy(ctx);
#endif
#ifdef _USE_COCO
    clear();
    Q_ASSERT( err );
    Coco::Parser p(in,err);
    int errs = err->getErrCount();
    p.Parse();
    errs = err->getErrCount() - errs;
    //dumpTree( &p.d_root );
    if( errs == 0 )
    {
        d_st = p.d_root.d_children;
        p.d_root.d_children.clear();
        return true;
    }else
        return false;
#endif
}

QList<SynTree*> Parser::getResult(bool transfer)
{
    QList<SynTree*> res = d_st;
    if( transfer )
        d_st.clear();
    return res;
}

void Parser::dumpTree(SynTree* node, int level)
{
    QByteArray str;
    if( node->d_tok.d_type == Tok_Invalid )
        level--;
    else if( node->d_tok.d_type < SynTree::R_First )
    {
        if( tokenIsReservedWord( node->d_tok.d_type ) )
            str = node->d_tok.d_val.toUpper();
        else if( node->d_tok.d_type >= Tok_String )
            str = QByteArray("\"") + node->d_tok.d_val + QByteArray("\"");
        else
            str = QByteArray("\"") + node->d_tok.getName() + QByteArray("\"");

    }else
        str = SynTree::rToStr( node->d_tok.d_type );
    str += QByteArray("\t\t\t\t\t") + QFileInfo(node->d_tok.d_sourcePath).fileName() +
            ":" + QByteArray::number(node->d_tok.d_lineNr);
    qDebug() << QByteArray(level*3, ' ').data() << str.data();
    foreach( SynTree* sub, node->d_children )
        dumpTree( sub, level + 1 );
}

void Parser::clear()
{
    foreach( SynTree* st, d_st )
        delete st;
}

}

extern "C"
{
    int lexerGetNextChar(void*)
    {
        // https://github.com/enechaev/packcc
        static QByteArray buf;
        while( true )
        {
            if( !buf.isEmpty() )
            {
                const char ch = buf[0];
                buf = buf.mid(1);
                return ch;
            }
            Vl::Token t = Vl::s_lex->nextToken();
            if( t.isEof() )
                return 0;
            else if( !t.isValid() )
            {
                qCritical() << t.d_val;
                return 0;
            }else if( t.d_type != Vl::Tok_Comment )
            {
                qDebug() << t.getName();
                buf = QByteArray::number( t.d_type ); //  + " ";
            }else
            {
                qDebug() << "Ignoring" << t.getName();
                t = Vl::s_lex->nextToken();
            }
        }
        /* ***** parsing "/home/me/Entwicklung/Modules/VerilogEbnf/Testdaten/Traffic_eng312_proj3.v"
        Ignoring <comment>
        module
        ID
        (
        input
        [
        NUM
        :
        NUM
        Dann passiert nichts mehr
        */
    }
}

void yyerror(const char* str)
{
    qCritical() << str;
}

int yylex()
{

    Vl::Token t = Vl::s_lex->nextToken();
    while( true )
    {
        if( t.isEof() )
            return 0;
        else if( !t.isValid() )
        {
            qCritical() << t.d_val;
            return 0;
        }else if( t.d_type != Vl::Tok_Comment )
        {
            qDebug() << t.getName();
            return t.d_type;
        }else
        {
            qDebug() << "Ignoring" << t.getName();
            t = Vl::s_lex->nextToken();
        }
    }
    /*
        ***** parsing "/home/me/Entwicklung/Modules/VerilogEbnf/Testdaten/Traffic_eng312_proj3.v"
        Ignoring <comment>
        Ignoring <comment>
        module
        ID
        (
        input
        [
        NUM
        :
        memory exhausted
    */
}
