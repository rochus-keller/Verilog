#ifndef VLTOKEN_H
#define VLTOKEN_H

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

#include <QString>

namespace Vl
{
    enum TokenType
    {
        Tok_Invalid,

        // operators
        Tok_Plus,           // +
        Tok_Minus,          // -
        Tok_Bang,           // !
        Tok_BangEq,         // !=
        Tok_Bang2Eq,        // !==
        Tok_Tilde,          // ~
        Tok_TildeBar,       // ~|
        Tok_TildeAmp,       // ~&
        Tok_TildeHat,       // ~^
        Tok_Hat,            // ^
        Tok_HatTilde,       // ^~
        Tok_Slash,          // /
        Tok_Percent,        // %
        Tok_Eq,             // =
        Tok_2Eq,            // ==
        Tok_3Eq,            // ===
        Tok_Amp,            // &
        Tok_2Amp,           // &&
        Tok_Bar,            // |
        Tok_2Bar,           // ||
        Tok_Star,           // *
        Tok_2Star,          // **
        Tok_Lt,             // <
        Tok_Leq,            // <=
        Tok_2Lt,            // <<
        Tok_3Lt,            // <<<
        Tok_Gt,             // >
        Tok_Geq,            // >=
        Tok_2Gt,            // >>
        Tok_3Gt,            // >>>
        Tok_Hash,           // #
        Tok_At,             // @
        Tok_Qmark,          // ?
        Tok_EqGt,           // =>
        Tok_StarGt,         // *>

        // brackets
        Tok_Lpar,           // (
        Tok_Rpar,           // )
        Tok_Lbrack,         // [
        Tok_Rbrack,         // ]
        Tok_Lbrace,         // {
        Tok_Rbrace,         // }
        Tok_Latt,           // (*
        Tok_Ratt,           // *)
        Tok_Lcmt,           // /*
        Tok_Rcmt,           // */

        // punctuation
        Tok_Comma,          // ,
        Tok_Dot,            // .
        Tok_Semi,           // ;
        Tok_Colon,          // :
        Tok_PlusColon,      // +:
        Tok_MinusColon,     // -:

        // reserved words
        Tok_always,
        Tok_and,
        Tok_assign,
        Tok_automatic,
        Tok_begin,
        Tok_buf,
        Tok_bufif0,
        Tok_bufif1,
        Tok_case,
        Tok_casex,
        Tok_casez,
        Tok_cell,
        Tok_cmos,
        Tok_config,
        Tok_deassign,
        Tok_default,
        Tok_defparam,
        Tok_design,
        Tok_disable,
        Tok_edge,
        Tok_else,
        Tok_end,
        Tok_endcase,
        Tok_endconfig,
        Tok_endfunction,
        Tok_endgenerate,
        Tok_endmodule,
        Tok_endprimitive,
        Tok_endspecify,
        Tok_endtable,
        Tok_endtask,
        Tok_event,
        Tok_for,
        Tok_force,
        Tok_forever,
        Tok_fork,
        Tok_function,
        Tok_generate,
        Tok_genvar,
        Tok_highz0,
        Tok_highz1,
        Tok_if,
        Tok_ifnone,
        Tok_incdir,
        Tok_include,
        Tok_initial,
        Tok_inout,
        Tok_input,
        Tok_instance,
        Tok_integer,
        Tok_join,
        Tok_large,
        Tok_liblist,
        Tok_library,
        Tok_localparam,
        Tok_macromodule,
        Tok_medium,
        Tok_module,
        Tok_nand,
        Tok_negedge,
        Tok_nmos,
        Tok_nor,
        Tok_noshowcancelled,
        Tok_not,
        Tok_notif0,
        Tok_notif1,
        Tok_or,
        Tok_output,
        Tok_parameter,
        Tok_pmos,
        Tok_posedge,
        Tok_primitive,
        Tok_pull0,
        Tok_pull1,
        Tok_pulldown,
        Tok_pullup,
        Tok_pulsestyle_onevent,
        Tok_pulsestyle_ondetect,
        Tok_rcmos,
        Tok_real,
        Tok_realtime,
        Tok_reg,
        Tok_release,
        Tok_repeat,
        Tok_rnmos,
        Tok_rpmos,
        Tok_rtran,
        Tok_rtranif0,
        Tok_rtranif1,
        Tok_scalared,
        Tok_showcancelled,
        Tok_signed,
        Tok_small,
        Tok_specify,
        Tok_specparam,
        Tok_strong0,
        Tok_strong1,
        Tok_supply0,
        Tok_supply1,
        Tok_table,
        Tok_task,
        Tok_time,
        Tok_tran,
        Tok_tranif0,
        Tok_tranif1,
        Tok_tri,
        Tok_tri0,
        Tok_tri1,
        Tok_triand,
        Tok_trior,
        Tok_trireg,
        Tok_unsigned,
        Tok_use,
        Tok_uwire,
        Tok_vectored,
        Tok_wait,
        Tok_wand,
        Tok_weak0,
        Tok_weak1,
        Tok_while,
        Tok_wire,
        Tok_wor,
        Tok_xnor,
        Tok_xor,

        Tok_maxKeyword,

        // reserved system names
        Tok_PathPulse,
        Tok_Setup,
        Tok_Hold,
        Tok_SetupHold,
        Tok_Recovery,
        Tok_Removal,
        Tok_Recrem,
        Tok_Skew,
        Tok_TimeSkew,
        Tok_FullSkew,
        Tok_Period,
        Tok_Width,
        Tok_NoChange,

        Tok_maxSystemName,

        // misc
        Tok_String,
        Tok_Ident,          // Normal and Escaped ID
        Tok_SysName,        // System name, starting with $
        Tok_CoDi,           // Compiler Directive
        Tok_Realnum,        // Kommazahl, ev. Exponent
        Tok_Natural,        // nur einzelne Dezimalzahl ohne Base
        Tok_SizedBased,     // vollständige Zahl aus Natural + BaseFormat + BaseValue
        Tok_BasedInt,       // BaseFormat + BaseValue
        Tok_BaseFormat,     // nur decimal_base, binary_base, octal_base oder hex_base
        Tok_BaseValue,      // nur decimal_value, binary_value, octal_value oder hex_value
        Tok_Attribute,      // substitutes Lattr and Rattr and all between
        Tok_MacroUsage,     // Redundant macro usage CoDi, followed by actual args if there, all d_prePp=true, followed by
        Tok_Comment,        // // or /**/
        Tok_LineCont,       // Line continuation in `define xxx backslash
        Tok_Eof
    };

    TokenType matchReservedWord( const QByteArray& );
    const char* tokenToString( quint8 ); // Pretty with punctuation chars
    QByteArray tokenToString( quint8, const QByteArray& ); // Pretty with punctuation chars and value für IDENT etc.
    const char* tokenName( quint8 ); // Just the names without punctuation chars
    bool tokenIsReservedWord( quint8 );
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
