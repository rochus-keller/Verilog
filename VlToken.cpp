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

#include "VlToken.h"
#include <QHash>
using namespace Vl;


static inline char at( const QByteArray& str, int off )
{
    if( off >= 0 && off < str.size() )
        return ::tolower( str[off] );
    else
        return 0;
}

static inline bool check( const QByteArray& str, int off, const char* match )
{
    // qDebug() << str << off << match;
    int i = 0;
    for( i = off; i < str.length(); i++ )
    {
        if( QChar::fromLatin1(str[i]).toLower().unicode() != match[i - off] )
            return false;
    }
    if( match[i - off] != 0 )
        return false;
    // Macht Kopie: return str.mid( off ).toLower().toAscii() == match;
    return true;
}


namespace Vl
{
    TokenType matchReservedWord(const QByteArray& str)
    {
        static QHash<QByteArray,quint8> s_lookup;
        // TODO: nur temporäre Lösung
        if( s_lookup.isEmpty() )
        {
            for( int i = TT_Keywords + 1; i < TT_Specials; i++ )
                s_lookup.insert( tokenToString(i), i );
        }
        TokenType t = (TokenType)s_lookup.value( str, Tok_Invalid );
        if( t == Tok_Invalid && str.startsWith( "PATHPULSE$" ) )
            t = Tok_PATHPULSE_dlr; // Fall PATHPULSE$specify_input_terminal_descriptor$specify_output_terminal_descriptor
        return t;
    }

    QByteArray tokenToString(quint8 t, const QByteArray& val)
    {
        switch( t )
        {
        case Tok_string:
        case Tok_identifier:
        case Tok_system_name:
        case Tok_CoDi:
        case Tok_real_number:
        case Tok_natural_number:
        case Tok_sizedbased_number:
        case Tok_based_number:
        case Tok_base_format:
        case Tok_base_value:
        case Tok_Comment:
        case Tok_Attribute:
        case Tok_MacroUsage:
        case Tok_Section:
        case Tok_SectionEnd:
            return val;
        default:
            return tokenToString(t);
        }
    }
    const char* tokenToString(quint8 t)
    {
        return tokenTypeString(t);
    }

    const char*tokenName(quint8 t)
    {
        return tokenTypeName(t);
    }

    bool Token::isValid() const
    {
        return d_type != Tok_Eof && d_type != Tok_Invalid;
    }

    bool Token::isEof() const
    {
        return d_type == Tok_Eof;
    }

    const char* Token::getName() const
    {
        return tokenToString( d_type );
    }

    Directive matchDirective(const QByteArray& str)
    {
        static QHash<QByteArray,quint8> s_lookup;
        // TODO: nur temporäre Lösung
        if( s_lookup.isEmpty() )
        {
            for( int i = Cd_Invalid + 1; i <= Cd_Max; i++ )
                s_lookup.insert( directiveToString(i), i );
        }
        return (Directive)s_lookup.value( str, Cd_Invalid );
    }

    const char*directiveToString(quint8 d)
    {
        switch(d)
        {
        case Cd_begin_keywords:
            return "begin_keywords";
        case Cd_celldefine:
            return "celldefine";
        case Cd_default_nettype:
            return "default_nettype";
        case Cd_define:
            return "define";
        case Cd_else:
            return "else";
        case Cd_elsif:
            return "elsif";
        case Cd_end_keywords:
            return "end_keywords";
        case Cd_endcelldefine:
            return "endcelldefine";
        case Cd_endif:
            return "endif";
        case Cd_ifdef:
            return "ifdef";
        case Cd_ifndef:
            return "ifndef";
        case Cd_include:
            return "include";
        case Cd_line:
            return "line";
        case Cd_nounconnected_drive:
            return "nounconnected_drive";
        case Cd_pragma:
            return "pragma";
        case Cd_resetall:
            return "resetall";
        case Cd_timescale:
            return "timescale";
        case Cd_unconnected_drive:
            return "unconnected_drive";
        case Cd_undef:
            return "undef";
        default:
            return "<unknown_directive>";
        }
    }

    bool tokenIsReservedWord(quint8 t)
    {
        return tokenTypeIsKeyword(t);
    }

    bool tokenIsNumber(quint8 t)
    {
        switch( t )
        {
        case Tok_real_number:
        case Tok_natural_number:
        case Tok_sizedbased_number:
        case Tok_based_number:
        case Tok_base_format:
        case Tok_base_value:
            return true;
        }
        return false;
    }

    bool tokenIsDelimiter(quint8 t)
    {
        return tokenTypeIsLiteral(t) || t == Tok_LineCont;
    }

    bool tokenIsType(quint8 t)
    {
        switch(t)
        {
        // type support
        case Tok_signed:
            return true;
        // net types
        case Tok_vectored:
        case Tok_scalared:
        case Tok_trireg:
        case Tok_supply0:
        case Tok_supply1:
        case Tok_tri:
        case Tok_triand:
        case Tok_trior:
        case Tok_tri0:
        case Tok_tri1:
        case Tok_uwire:
        case Tok_wire:
        case Tok_wand:
        case Tok_wor:
            return true;
        // variable declaration
        case Tok_integer:
        case Tok_real:
        case Tok_realtime:
        case Tok_reg:
        case Tok_time:
            return true;
        default:
            return false;
        }
    }

    bool tokenIsOperator(quint8 t)
    {
        return tokenTypeIsLiteral(t) && !tokenIsBracket(t);
    }

    bool tokenIsBracket(quint8 t)
    {
        switch(t)
        {
        case Tok_Lpar:
        case Tok_Rpar:
        case Tok_Lbrack:
        case Tok_Rbrack:
        case Tok_Lbrace:
        case Tok_Rbrace:
        case Tok_Latt:
        case Tok_Ratt:
            return true;
        }
        return false;
    }

    bool tokenIsBlockEnd(quint8 t)
    {
        switch(t)
        {
        case Tok_end:
        case Tok_endcase:
        case Tok_endconfig:
        case Tok_endfunction:
        case Tok_endgenerate:
        case Tok_endmodule:
        case Tok_endprimitive:
        case Tok_endspecify:
        case Tok_endtable:
        case Tok_endtask:
        case Tok_join:
            return true;
        default:
            return false;
        }
    }

    bool tokenIsBlockBegin(quint8 t)
    {
        switch(t)
        {
        case Tok_begin:
        case Tok_case:
        case Tok_casez:
        case Tok_casex:
        case Tok_config:
        case Tok_function:
        case Tok_generate:
        case Tok_module:
        case Tok_macromodule:
        case Tok_primitive:
        case Tok_specify:
        case Tok_table:
        case Tok_task:
        case Tok_fork:
            return true;
        default:
            return false;
        }
    }
}
