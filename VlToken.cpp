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
            for( int i = Tok_always; i < Tok_maxSystemName; i++ )
                s_lookup.insert( tokenToString(i), i );
        }
        TokenType t = (TokenType)s_lookup.value( str, Tok_Invalid );
        if( t == Tok_Invalid && str.startsWith( "PATHPULSE$" ) )
            t = Tok_PathPulse; // Fall PATHPULSE$specify_input_terminal_descriptor$specify_output_terminal_descriptor
        return t;
    }

    QByteArray tokenToString(quint8 t, const QByteArray& val)
    {
        switch( t )
        {
        case Tok_String:
        case Tok_Ident:
        case Tok_SysName:
        case Tok_CoDi:
        case Tok_Realnum:
        case Tok_Natural:
        case Tok_SizedBased:
        case Tok_BasedInt:
        case Tok_BaseFormat:
        case Tok_BaseValue:
        case Tok_Comment:
        case Tok_Attribute:
        case Tok_MacroUsage:
            return val;
        default:
            return tokenToString(t);
        }
    }
    const char* tokenToString(quint8 t)
    {
        switch(t)
        {
        case Tok_Invalid:
            return "<invalid>";
        case Tok_Plus:
            return "+";
        case Tok_Minus:
            return "-";
        case Tok_Bang:
            return "!";
        case Tok_BangEq:
            return "!=";
        case Tok_Bang2Eq:
            return "!==";
        case Tok_Tilde:
            return "~";
        case Tok_TildeBar:
            return "~|";
        case Tok_TildeAmp:
            return "~&";
        case Tok_TildeHat:
            return "~^";
        case Tok_Hat:
            return "^";
        case Tok_HatTilde:
            return "^~";
        case Tok_Slash:
            return "/";
        case Tok_Percent:
            return "%";
        case Tok_Eq:
            return "=";
        case Tok_2Eq:
            return "==";
        case Tok_3Eq:
            return "===";
        case Tok_Amp:
            return "&";
        case Tok_2Amp:
            return "&&";
        case Tok_Bar:
            return "|";
        case Tok_2Bar:
            return "||";
        case Tok_Star:
            return "*";
        case Tok_2Star:
            return "**";
        case Tok_Lt:
            return "<";
        case Tok_Leq:
            return "<=";
        case Tok_2Lt:
            return "<<";
        case Tok_3Lt:
            return "<<<";
        case Tok_Gt:
            return ">";
        case Tok_Geq:
            return ">=";
        case Tok_2Gt:
            return ">>";
        case Tok_3Gt:
            return ">>>";
        case Tok_Hash:
            return "#";
        case Tok_At:
            return "@";
        case Tok_Qmark:
            return "?";
        case Tok_EqGt:
            return "=>";
        case Tok_StarGt:
            return "*>";
        case Tok_Lpar:
            return "(";
        case Tok_Rpar:
            return ")";
        case Tok_Lbrack:
            return "[";
        case Tok_Rbrack:
            return "]";
        case Tok_Lbrace:
            return "{";
        case Tok_Rbrace:
            return "}";
        case Tok_Latt:
            return "(*";
        case Tok_Ratt:
            return "*)";
        case Tok_Lcmt:
            return "/*";
        case Tok_Rcmt:
            return "*/";
        case Tok_Comma:
            return ",";
        case Tok_Dot:
            return ".";
        case Tok_Semi:
            return ";";
        case Tok_Colon:
            return ":";
        case Tok_PlusColon:
            return "+:";
        case Tok_MinusColon:
            return "-:";
        case Tok_always:
            return "always";
        case Tok_and:
            return "and";
        case Tok_assign:
            return "assign";
        case Tok_automatic:
            return "automatic";
        case Tok_begin:
            return "begin";
        case Tok_buf:
            return "buf";
        case Tok_bufif0:
            return "bufif0";
        case Tok_bufif1:
            return "bufif1";
        case Tok_case:
            return "case";
        case Tok_casex:
            return "casex";
        case Tok_casez:
            return "casez";
        case Tok_cell:
            return "cell";
        case Tok_cmos:
            return "cmos";
        case Tok_config:
            return "config";
        case Tok_deassign:
            return "deassign";
        case Tok_default:
            return "default";
        case Tok_defparam:
            return "defparam";
        case Tok_design:
            return "design";
        case Tok_disable:
            return "disable";
        case Tok_edge:
            return "edge";
        case Tok_else:
            return "else";
        case Tok_end:
            return "end";
        case Tok_endcase:
            return "endcase";
        case Tok_endconfig:
            return "endconfig";
        case Tok_endfunction:
            return "endfunction";
        case Tok_endgenerate:
            return "endgenerate";
        case Tok_endmodule:
            return "endmodule";
        case Tok_endprimitive:
            return "endprimitive";
        case Tok_endspecify:
            return "endspecify";
        case Tok_endtable:
            return "endtable";
        case Tok_endtask:
            return "endtask";
        case Tok_event:
            return "event";
        case Tok_for:
            return "for";
        case Tok_force:
            return "force";
        case Tok_forever:
            return "forever";
        case Tok_fork:
            return "fork";
        case Tok_function:
            return "function";
        case Tok_generate:
            return "generate";
        case Tok_genvar:
            return "genvar";
        case Tok_highz0:
            return "highz0";
        case Tok_highz1:
            return "highz1";
        case Tok_if:
            return "if";
        case Tok_ifnone:
            return "ifnone";
        case Tok_incdir:
            return "incdir";
        case Tok_include:
            return "include";
        case Tok_initial:
            return "initial";
        case Tok_inout:
            return "inout";
        case Tok_input:
            return "input";
        case Tok_instance:
            return "instance";
        case Tok_integer:
            return "integer";
        case Tok_join:
            return "join";
        case Tok_large:
            return "large";
        case Tok_liblist:
            return "liblist";
        case Tok_library:
            return "library";
        case Tok_localparam:
            return "localparam";
        case Tok_macromodule:
            return "macromodule";
        case Tok_medium:
            return "medium";
        case Tok_module:
            return "module";
        case Tok_nand:
            return "nand";
        case Tok_negedge:
            return "negedge";
        case Tok_nmos:
            return "nmos";
        case Tok_nor:
            return "nor";
        case Tok_noshowcancelled:
            return "noshowcancelled";
        case Tok_not:
            return "not";
        case Tok_notif0:
            return "notif0";
        case Tok_notif1:
            return "notif1";
        case Tok_or:
            return "or";
        case Tok_output:
            return "output";
        case Tok_parameter:
            return "parameter";
        case Tok_pmos:
            return "pmos";
        case Tok_posedge:
            return "posedge";
        case Tok_primitive:
            return "primitive";
        case Tok_pull0:
            return "pull0";
        case Tok_pull1:
            return "pull1";
        case Tok_pulldown:
            return "pulldown";
        case Tok_pullup:
            return "pullup";
        case Tok_pulsestyle_onevent:
            return "pulsestyle_onevent";
        case Tok_pulsestyle_ondetect:
            return "pulsestyle_ondetect";
        case Tok_rcmos:
            return "rcmos";
        case Tok_real:
            return "real";
        case Tok_realtime:
            return "realtime";
        case Tok_reg:
            return "reg";
        case Tok_release:
            return "release";
        case Tok_repeat:
            return "repeat";
        case Tok_rnmos:
            return "rnmos";
        case Tok_rpmos:
            return "rpmos";
        case Tok_rtran:
            return "rtran";
        case Tok_rtranif0:
            return "rtranif0";
        case Tok_rtranif1:
            return "rtranif1";
        case Tok_scalared:
            return "scalared";
        case Tok_showcancelled:
            return "showcancelled";
        case Tok_signed:
            return "signed";
        case Tok_small:
            return "small";
        case Tok_specify:
            return "specify";
        case Tok_specparam:
            return "specparam";
        case Tok_strong0:
            return "strong0";
        case Tok_strong1:
            return "strong1";
        case Tok_supply0:
            return "supply0";
        case Tok_supply1:
            return "supply1";
        case Tok_table:
            return "table";
        case Tok_task:
            return "task";
        case Tok_time:
            return "time";
        case Tok_tran:
            return "tran";
        case Tok_tranif0:
            return "tranif0";
        case Tok_tranif1:
            return "tranif1";
        case Tok_tri:
            return "tri";
        case Tok_tri0:
            return "tri0";
        case Tok_tri1:
            return "tri1";
        case Tok_triand:
            return "triand";
        case Tok_trior:
            return "trior";
        case Tok_trireg:
            return "trireg";
        case Tok_unsigned:
            return "unsigned";
        case Tok_use:
            return "use";
        case Tok_uwire:
            return "uwire";
        case Tok_vectored:
            return "vectored";
        case Tok_wait:
            return "wait";
        case Tok_wand:
            return "wand";
        case Tok_weak0:
            return "weak0";
        case Tok_weak1:
            return "weak1";
        case Tok_while:
            return "while";
        case Tok_wire:
            return "wire";
        case Tok_wor:
            return "wor";
        case Tok_xnor:
            return "xnor";
        case Tok_xor:
            return "xor";
        case Tok_maxKeyword:
            return "<maxKeyword>";
       case Tok_PathPulse:
            return "PATHPULSE$";
        case Tok_Setup:
            return "$setup";
        case Tok_Hold:
            return "$hold";
        case Tok_SetupHold:
            return "$setuphold";
        case Tok_Recovery:
            return "$recovery";
        case Tok_Removal:
            return "$removal";
        case Tok_Recrem:
            return "$recrem";
        case Tok_Skew:
            return "$skew";
        case Tok_TimeSkew:
            return "$timeskew";
        case Tok_FullSkew:
            return "$fullskew";
        case Tok_Period:
            return "$period";
        case Tok_Width:
            return "$width";
        case Tok_NoChange:
            return "$nochange";
        case Tok_maxSystemName:
            return "<maxSystemName>";
        case Tok_String:
            return "STRING";
        case Tok_Ident:
            return "IDENT";
        case Tok_SysName:
            return "SYSNAME";
        case Tok_CoDi:
            return "CODI";
        case Tok_Realnum:
            return "REALNUM";
        case Tok_Natural:
            return "NATURAL";
        case Tok_SizedBased:
            return "SIZEDBASED";
        case Tok_BasedInt:
            return "BASEDINT";
        case Tok_BaseFormat:
            return "BASEFMT";
        case Tok_BaseValue:
            return "BASEVAL";
        case Tok_Eof:
            return "<eof>";
        case Tok_Comment:
            return "<comment>";
        case Tok_LineCont:
            return "<continue>";
        case Tok_Attribute:
            return "<attribute>";
        case Tok_MacroUsage:
            return "<macro usage>";
        }
        return "";
    }

    const char*tokenName(quint8 t)
    {
        switch(t)
        {
        case Tok_Plus:
            return "Tok_Plus";
        case Tok_Minus:
            return "Tok_Minus";
        case Tok_Bang:
            return "Tok_Bang";
        case Tok_BangEq:
            return "Tok_BangEq";
        case Tok_Bang2Eq:
            return "Tok_Bang2Eq";
        case Tok_Tilde:
            return "Tok_Tilde";
        case Tok_TildeBar:
            return "Tok_TildeBar";
        case Tok_TildeAmp:
            return "Tok_TildeAmp";
        case Tok_TildeHat:
            return "Tok_TildeHat";
        case Tok_Hat:
            return "Tok_Hat";
        case Tok_HatTilde:
            return "Tok_HatTilde";
        case Tok_Slash:
            return "Tok_Slash";
        case Tok_Percent:
            return "Tok_Percent";
        case Tok_Eq:
            return "Tok_Eq";
        case Tok_2Eq:
            return "Tok_2Eq";
        case Tok_3Eq:
            return "Tok_3Eq";
        case Tok_Amp:
            return "Tok_Amp";
        case Tok_2Amp:
            return "Tok_2Amp";
        case Tok_Bar:
            return "Tok_Bar";
        case Tok_2Bar:
            return "Tok_2Bar";
        case Tok_Star:
            return "Tok_Star";
        case Tok_2Star:
            return "Tok_2Star";
        case Tok_Lt:
            return "Tok_Lt";
        case Tok_Leq:
            return "Tok_Leq";
        case Tok_2Lt:
            return "Tok_2Lt";
        case Tok_3Lt:
            return "Tok_3Lt";
        case Tok_Gt:
            return "Tok_Gt";
        case Tok_Geq:
            return "Tok_Geq";
        case Tok_2Gt:
            return "Tok_2Gt";
        case Tok_3Gt:
            return "Tok_3Gt";
        case Tok_Hash:
            return "Tok_Hash";
        case Tok_At:
            return "Tok_At";
        case Tok_Qmark:
            return "Tok_Qmark";
        case Tok_EqGt:
            return "Tok_EqGt";
        case Tok_StarGt:
            return "Tok_StarGt";
        case Tok_Lpar:
            return "Tok_Lpar";
        case Tok_Rpar:
            return "Tok_Rpar";
        case Tok_Lbrack:
            return "Tok_Lbrack";
        case Tok_Rbrack:
            return "Tok_Rbrack";
        case Tok_Lbrace:
            return "Tok_Lbrace";
        case Tok_Rbrace:
            return "Tok_Rbrace";
        case Tok_Latt:
            return "Tok_Latt";
        case Tok_Ratt:
            return "Tok_Ratt";
        case Tok_Lcmt:
            return "Tok_Lcmt";
        case Tok_Rcmt:
            return "Tok_Rcmt";
        case Tok_Comma:
            return "Tok_Comma";
        case Tok_Dot:
            return "Tok_Dot";
        case Tok_Semi:
            return "Tok_Semi";
        case Tok_Colon:
            return "Tok_Colon";
        case Tok_PlusColon:
            return "Tok_PlusColon";
        case Tok_MinusColon:
            return "Tok_MinusColon";
        case Tok_always:
            return "Tok_always";
        case Tok_and:
            return "Tok_and";
        case Tok_assign:
            return "Tok_assign";
        case Tok_automatic:
            return "Tok_automatic";
        case Tok_begin:
            return "Tok_begin";
        case Tok_buf:
            return "Tok_buf";
        case Tok_bufif0:
            return "Tok_bufif0";
        case Tok_bufif1:
            return "Tok_bufif1";
        case Tok_case:
            return "Tok_case";
        case Tok_casex:
            return "Tok_casex";
        case Tok_casez:
            return "Tok_casez";
        case Tok_cell:
            return "Tok_cell";
        case Tok_cmos:
            return "Tok_cmos";
        case Tok_config:
            return "Tok_config";
        case Tok_deassign:
            return "Tok_deassign";
        case Tok_default:
            return "Tok_default";
        case Tok_defparam:
            return "Tok_defparam";
        case Tok_design:
            return "Tok_design";
        case Tok_disable:
            return "Tok_disable";
        case Tok_edge:
            return "Tok_edge";
        case Tok_else:
            return "Tok_else";
        case Tok_end:
            return "Tok_end";
        case Tok_endcase:
            return "Tok_endcase";
        case Tok_endconfig:
            return "Tok_endconfig";
        case Tok_endfunction:
            return "Tok_endfunction";
        case Tok_endgenerate:
            return "Tok_endgenerate";
        case Tok_endmodule:
            return "Tok_endmodule";
        case Tok_endprimitive:
            return "Tok_endprimitive";
        case Tok_endspecify:
            return "Tok_endspecify";
        case Tok_endtable:
            return "Tok_endtable";
        case Tok_endtask:
            return "Tok_endtask";
        case Tok_event:
            return "Tok_event";
        case Tok_for:
            return "Tok_for";
        case Tok_force:
            return "Tok_force";
        case Tok_forever:
            return "Tok_forever";
        case Tok_fork:
            return "Tok_fork";
        case Tok_function:
            return "Tok_function";
        case Tok_generate:
            return "Tok_generate";
        case Tok_genvar:
            return "Tok_genvar";
        case Tok_highz0:
            return "Tok_highz0";
        case Tok_highz1:
            return "Tok_highz1";
        case Tok_if:
            return "Tok_if";
        case Tok_ifnone:
            return "Tok_ifnone";
        case Tok_incdir:
            return "Tok_incdir";
        case Tok_include:
            return "Tok_include";
        case Tok_initial:
            return "Tok_initial";
        case Tok_inout:
            return "Tok_inout";
        case Tok_input:
            return "Tok_input";
        case Tok_instance:
            return "Tok_instance";
        case Tok_integer:
            return "Tok_integer";
        case Tok_join:
            return "Tok_join";
        case Tok_large:
            return "Tok_large";
        case Tok_liblist:
            return "Tok_liblist";
        case Tok_library:
            return "Tok_library";
        case Tok_localparam:
            return "Tok_localparam";
        case Tok_macromodule:
            return "Tok_macromodule";
        case Tok_medium:
            return "Tok_medium";
        case Tok_module:
            return "Tok_module";
        case Tok_nand:
            return "Tok_nand";
        case Tok_negedge:
            return "Tok_negedge";
        case Tok_nmos:
            return "Tok_nmos";
        case Tok_nor:
            return "Tok_nor";
        case Tok_noshowcancelled:
            return "Tok_noshowcancelled";
        case Tok_not:
            return "Tok_not";
        case Tok_notif0:
            return "Tok_notif0";
        case Tok_notif1:
            return "Tok_notif1";
        case Tok_or:
            return "Tok_or";
        case Tok_output:
            return "Tok_output";
        case Tok_parameter:
            return "Tok_parameter";
        case Tok_pmos:
            return "Tok_pmos";
        case Tok_posedge:
            return "Tok_posedge";
        case Tok_primitive:
            return "Tok_primitive";
        case Tok_pull0:
            return "Tok_pull0";
        case Tok_pull1:
            return "Tok_pull1";
        case Tok_pulldown:
            return "Tok_pulldown";
        case Tok_pullup:
            return "Tok_pullup";
        case Tok_pulsestyle_onevent:
            return "Tok_pulsestyle_onevent";
        case Tok_pulsestyle_ondetect:
            return "Tok_pulsestyle_ondetect";
        case Tok_rcmos:
            return "Tok_rcmos";
        case Tok_real:
            return "Tok_real";
        case Tok_realtime:
            return "Tok_realtime";
        case Tok_reg:
            return "Tok_reg";
        case Tok_release:
            return "Tok_release";
        case Tok_repeat:
            return "Tok_repeat";
        case Tok_rnmos:
            return "Tok_rnmos";
        case Tok_rpmos:
            return "Tok_rpmos";
        case Tok_rtran:
            return "Tok_rtran";
        case Tok_rtranif0:
            return "Tok_rtranif0";
        case Tok_rtranif1:
            return "Tok_rtranif1";
        case Tok_scalared:
            return "Tok_scalared";
        case Tok_showcancelled:
            return "Tok_showcancelled";
        case Tok_signed:
            return "Tok_signed";
        case Tok_small:
            return "Tok_small";
        case Tok_specify:
            return "Tok_specify";
        case Tok_specparam:
            return "Tok_specparam";
        case Tok_strong0:
            return "Tok_strong0";
        case Tok_strong1:
            return "Tok_strong1";
        case Tok_supply0:
            return "Tok_supply0";
        case Tok_supply1:
            return "Tok_supply1";
        case Tok_table:
            return "Tok_table";
        case Tok_task:
            return "Tok_task";
        case Tok_time:
            return "Tok_time";
        case Tok_tran:
            return "Tok_tran";
        case Tok_tranif0:
            return "Tok_tranif0";
        case Tok_tranif1:
            return "Tok_tranif1";
        case Tok_tri:
            return "Tok_tri";
        case Tok_tri0:
            return "Tok_tri0";
        case Tok_tri1:
            return "Tok_tri1";
        case Tok_triand:
            return "Tok_triand";
        case Tok_trior:
            return "Tok_trior";
        case Tok_trireg:
            return "Tok_trireg";
        case Tok_unsigned:
            return "Tok_unsigned";
        case Tok_use:
            return "Tok_use";
        case Tok_uwire:
            return "Tok_uwire";
        case Tok_vectored:
            return "Tok_vectored";
        case Tok_wait:
            return "Tok_wait";
        case Tok_wand:
            return "Tok_wand";
        case Tok_weak0:
            return "Tok_weak0";
        case Tok_weak1:
            return "Tok_weak1";
        case Tok_while:
            return "Tok_while";
        case Tok_wire:
            return "Tok_wire";
        case Tok_wor:
            return "Tok_wor";
        case Tok_xnor:
            return "Tok_xnor";
        case Tok_xor:
            return "Tok_xor";
        case Tok_maxKeyword:
            return "Tok_maxKeyword";
        case Tok_PathPulse:
            return "Tok_PathPulse";
        case Tok_Setup:
            return "Tok_Setup";
        case Tok_Hold:
            return "Tok_Hold";
        case Tok_SetupHold:
            return "Tok_SetupHold";
        case Tok_Recovery:
            return "Tok_Recovery";
        case Tok_Removal:
            return "Tok_Removal";
        case Tok_Recrem:
            return "Tok_Recrem";
        case Tok_Skew:
            return "Tok_Skew";
        case Tok_TimeSkew:
            return "Tok_TimeSkew";
        case Tok_FullSkew:
            return "Tok_FullSkew";
        case Tok_Period:
            return "Tok_Period";
        case Tok_Width:
            return "Tok_Width";
        case Tok_NoChange:
            return "Tok_NoChange";
        case Tok_maxSystemName:
            return "Tok_maxSystemName";
        case Tok_String:
            return "Tok_String";
        case Tok_Ident:
            return "Tok_Ident";
        case Tok_SysName:
            return "Tok_SysName";
        case Tok_CoDi:
            return "Tok_CoDi";
        case Tok_Realnum:
            return "Tok_Realnum";
        case Tok_Natural:
            return "Tok_Natural";
        case Tok_SizedBased:
            return "Tok_SizedBased";
        case Tok_BasedInt:
            return "Tok_BasedInt";
        case Tok_BaseFormat:
            return "Tok_BaseFormat";
        case Tok_BaseValue:
            return "Tok_BaseValue";
        case Tok_Attribute:
            return "Tok_Attribute";
        case Tok_MacroUsage:
            return "Tok_MacroUsage";
       case Tok_Eof:
            return "Tok_Eof";
        case Tok_Comment:
            return "Tok_Comment";
        case Tok_LineCont:
            return "Tok_LineCont";
        }
        return "?";
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
        return t >= Tok_always && t < Tok_maxSystemName;
    }

    bool tokenIsNumber(quint8 t)
    {
        return t >= Tok_Realnum && t <= Tok_BaseValue;
    }

    bool tokenIsDelimiter(quint8 t)
    {
        return ( t >= Tok_Plus && t <= Tok_MinusColon ) || t == Tok_LineCont;
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
        return ( t >= Tok_Plus && t <= Tok_StarGt ) || ( t >= Tok_Comma && t <= Tok_MinusColon );
    }

    bool tokenIsBracket(quint8 t)
    {
        return t >= Tok_Lpar && t <= Tok_Ratt;
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
