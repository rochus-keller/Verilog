#ifndef VLTOKEN_H
#define VLTOKEN_H

/*
* Copyright 2018-2019 Rochus Keller <mailto:me@rochus-keller.ch>
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

#include <QString>
#include <Verilog/VlTokenType.h>

namespace Vl
{
    TokenType matchReservedWord( const QByteArray& );
    const char* tokenToString( quint8 ); // Pretty with punctuation chars
    QByteArray tokenToString( quint8, const QByteArray& ); // Pretty with punctuation chars and value für IDENT etc.
    const char* tokenName( quint8 ); // Just the names without punctuation chars
    bool tokenIsReservedWord( quint8 );
    bool tokenIsSvReservedWord( quint8 );
    bool tokenIsNumber(quint8);
    bool tokenIsDelimiter(quint8);
    bool tokenIsType(quint8);
    bool tokenIsOperator(quint8);
    bool tokenIsBracket(quint8);
    bool tokenIsBlockBegin(quint8);
    bool tokenIsBlockEnd(quint8);

    struct Token
    {
#ifdef _DEBUG
        union
        {
        int d_type; // TokenType
        TokenType d_tokenType;
        };
#else
        uint d_type : 16; // TokenType
#endif
        uint d_substituted : 1; // Token resultiert aus Auflösung MacroUsage
        uint d_hidden : 1; // Token ist wegen IfDef ausgeblendet und wird nur zur Dokumentation geliefert
        uint d_prePp: 1; // Token dokumentiert MacroUsage vor Auflösung durch Präprozessor
        quint32 d_lineNr;
        quint16 d_colNr, d_len;
        QByteArray d_val;
        QString d_sourcePath;
        Token(quint16 t = Tok_Invalid, quint32 line = 0, quint16 col = 0, quint16 len = 0, const QByteArray& val = QByteArray() ):
            d_type(t),d_lineNr(line),d_colNr(col),d_len(len),d_val(val),d_substituted(false),
            d_hidden(false),d_prePp(false){}
        bool isValid() const;
        bool isEof() const;
        const char* getName() const;
    };

    typedef QList<Token> TokenList;

    enum Directive {
        Cd_Invalid,
        Cd_ifdef,
        Cd_ifndef,
        Cd_else,
        Cd_elsif,
        Cd_endif,
        Cd_define,
        Cd_undef,
        Cd_resetall,
        Cd_include,
        Cd_timescale,
        Cd_begin_keywords,
        Cd_end_keywords,
        Cd_celldefine,
        Cd_endcelldefine,
        Cd_default_nettype,
        Cd_line,
        Cd_nounconnected_drive,
        Cd_unconnected_drive,
        Cd_pragma,
        Cd_Max
    };
    Directive matchDirective(const QByteArray&);
    const char* directiveToString( quint8 );

}

#endif // VLTOKEN_H
