// This file was automatically generated by EbnfStudio; don't modify it!
#include "VlTokenType.h"

namespace Vl {
	const char* tokenTypeString( int r ) {
		switch(r) {
			case Tok_Invalid: return "<invalid>";
			case Tok_Bang: return "!";
			case Tok_BangEq: return "!=";
			case Tok_Bang2Eq: return "!==";
			case Tok_Hash: return "#";
			case Tok_Percent: return "%";
			case Tok_Amp: return "&";
			case Tok_2Amp: return "&&";
			case Tok_3Amp: return "&&&";
			case Tok_Lpar: return "(";
			case Tok_Latt: return "(*";
			case Tok_Rpar: return ")";
			case Tok_Star: return "*";
			case Tok_Ratt: return "*)";
			case Tok_2Star: return "**";
			case Tok_Rcmt: return "*/";
			case Tok_StarGt: return "*>";
			case Tok_Plus: return "+";
			case Tok_PlusColon: return "+:";
			case Tok_Comma: return ",";
			case Tok_Minus: return "-";
			case Tok_MinusColon: return "-:";
			case Tok_MinusGt: return "->";
			case Tok_Dot: return ".";
			case Tok_Slash: return "/";
			case Tok_Lcmt: return "/*";
			case Tok_Colon: return ":";
			case Tok_Semi: return ";";
			case Tok_Lt: return "<";
			case Tok_2Lt: return "<<";
			case Tok_3Lt: return "<<<";
			case Tok_Leq: return "<=";
			case Tok_Eq: return "=";
			case Tok_2Eq: return "==";
			case Tok_3Eq: return "===";
			case Tok_EqGt: return "=>";
			case Tok_Gt: return ">";
			case Tok_Geq: return ">=";
			case Tok_2Gt: return ">>";
			case Tok_3Gt: return ">>>";
			case Tok_Qmark: return "?";
			case Tok_At: return "@";
			case Tok_Lbrack: return "[";
			case Tok_Rbrack: return "]";
			case Tok_Hat: return "^";
			case Tok_HatTilde: return "^~";
			case Tok_Lbrace: return "{";
			case Tok_Bar: return "|";
			case Tok_2Bar: return "||";
			case Tok_Rbrace: return "}";
			case Tok_Tilde: return "~";
			case Tok_TildeAmp: return "~&";
			case Tok_TildeHat: return "~^";
			case Tok_TildeBar: return "~|";
			case Tok_dlr_fullskew: return "$fullskew";
			case Tok_dlr_hold: return "$hold";
			case Tok_dlr_nochange: return "$nochange";
			case Tok_dlr_period: return "$period";
			case Tok_dlr_recovery: return "$recovery";
			case Tok_dlr_recrem: return "$recrem";
			case Tok_dlr_removal: return "$removal";
			case Tok_dlr_setup: return "$setup";
			case Tok_dlr_setuphold: return "$setuphold";
			case Tok_dlr_skew: return "$skew";
			case Tok_dlr_timeskew: return "$timeskew";
			case Tok_dlr_width: return "$width";
			case Tok_PATHPULSE_dlr: return "PATHPULSE$";
			case Tok_always: return "always";
			case Tok_and: return "and";
			case Tok_assign: return "assign";
			case Tok_automatic: return "automatic";
			case Tok_begin: return "begin";
			case Tok_buf: return "buf";
			case Tok_bufif0: return "bufif0";
			case Tok_bufif1: return "bufif1";
			case Tok_case: return "case";
			case Tok_casex: return "casex";
			case Tok_casez: return "casez";
			case Tok_cell: return "cell";
			case Tok_cmos: return "cmos";
			case Tok_config: return "config";
			case Tok_deassign: return "deassign";
			case Tok_default: return "default";
			case Tok_defparam: return "defparam";
			case Tok_design: return "design";
			case Tok_disable: return "disable";
			case Tok_edge: return "edge";
			case Tok_else: return "else";
			case Tok_end: return "end";
			case Tok_endcase: return "endcase";
			case Tok_endconfig: return "endconfig";
			case Tok_endfunction: return "endfunction";
			case Tok_endgenerate: return "endgenerate";
			case Tok_endmodule: return "endmodule";
			case Tok_endprimitive: return "endprimitive";
			case Tok_endspecify: return "endspecify";
			case Tok_endtable: return "endtable";
			case Tok_endtask: return "endtask";
			case Tok_event: return "event";
			case Tok_for: return "for";
			case Tok_force: return "force";
			case Tok_forever: return "forever";
			case Tok_fork: return "fork";
			case Tok_function: return "function";
			case Tok_generate: return "generate";
			case Tok_genvar: return "genvar";
			case Tok_highz0: return "highz0";
			case Tok_highz1: return "highz1";
			case Tok_if: return "if";
			case Tok_ifnone: return "ifnone";
			case Tok_incdir: return "incdir";
			case Tok_include: return "include";
			case Tok_initial: return "initial";
			case Tok_inout: return "inout";
			case Tok_input: return "input";
			case Tok_instance: return "instance";
			case Tok_integer: return "integer";
			case Tok_join: return "join";
			case Tok_large: return "large";
			case Tok_liblist: return "liblist";
			case Tok_library: return "library";
			case Tok_localparam: return "localparam";
			case Tok_macromodule: return "macromodule";
			case Tok_medium: return "medium";
			case Tok_module: return "module";
			case Tok_nand: return "nand";
			case Tok_negedge: return "negedge";
			case Tok_nmos: return "nmos";
			case Tok_nor: return "nor";
			case Tok_noshowcancelled: return "noshowcancelled";
			case Tok_not: return "not";
			case Tok_notif0: return "notif0";
			case Tok_notif1: return "notif1";
			case Tok_or: return "or";
			case Tok_output: return "output";
			case Tok_parameter: return "parameter";
			case Tok_pmos: return "pmos";
			case Tok_posedge: return "posedge";
			case Tok_primitive: return "primitive";
			case Tok_pull0: return "pull0";
			case Tok_pull1: return "pull1";
			case Tok_pulldown: return "pulldown";
			case Tok_pullup: return "pullup";
			case Tok_pulsestyle_ondetect: return "pulsestyle_ondetect";
			case Tok_pulsestyle_onevent: return "pulsestyle_onevent";
			case Tok_rcmos: return "rcmos";
			case Tok_real: return "real";
			case Tok_realtime: return "realtime";
			case Tok_reg: return "reg";
			case Tok_release: return "release";
			case Tok_repeat: return "repeat";
			case Tok_rnmos: return "rnmos";
			case Tok_rpmos: return "rpmos";
			case Tok_rtran: return "rtran";
			case Tok_rtranif0: return "rtranif0";
			case Tok_rtranif1: return "rtranif1";
			case Tok_scalared: return "scalared";
			case Tok_showcancelled: return "showcancelled";
			case Tok_signed: return "signed";
			case Tok_small: return "small";
			case Tok_specify: return "specify";
			case Tok_specparam: return "specparam";
			case Tok_strong0: return "strong0";
			case Tok_strong1: return "strong1";
			case Tok_supply0: return "supply0";
			case Tok_supply1: return "supply1";
			case Tok_table: return "table";
			case Tok_task: return "task";
			case Tok_time: return "time";
			case Tok_tran: return "tran";
			case Tok_tranif0: return "tranif0";
			case Tok_tranif1: return "tranif1";
			case Tok_tri: return "tri";
			case Tok_tri0: return "tri0";
			case Tok_tri1: return "tri1";
			case Tok_triand: return "triand";
			case Tok_trior: return "trior";
			case Tok_trireg: return "trireg";
			case Tok_use: return "use";
			case Tok_uwire: return "uwire";
			case Tok_vectored: return "vectored";
			case Tok_wait: return "wait";
			case Tok_wand: return "wand";
			case Tok_weak0: return "weak0";
			case Tok_weak1: return "weak1";
			case Tok_while: return "while";
			case Tok_wire: return "wire";
			case Tok_wor: return "wor";
			case Tok_xnor: return "xnor";
			case Tok_xor: return "xor";
			case Tok_Attribute: return "Attribute";
			case Tok_Comment: return "Comment";
			case Tok_MacroUsage: return "MacroUsage";
			case Tok_Section: return "Section";
			case Tok_SectionEnd: return "SectionEnd";
			case Tok_CoDi: return "CoDi";
			case Tok_LineCont: return "LineCont";
			case Tok_Realnum: return "Realnum";
			case Tok_Natural: return "Natural";
			case Tok_SizedBased: return "SizedBased";
			case Tok_BasedInt: return "BasedInt";
			case Tok_BaseFormat: return "BaseFormat";
			case Tok_BaseValue: return "BaseValue";
			case Tok_SysName: return "SysName";
			case Tok_Ident: return "Ident";
			case Tok_Str: return "Str";
			case Tok_Eof: return "<eof>";
			default: return "";
		}
	}
	const char* tokenTypeName( int r ) {
		switch(r) {
			case Tok_Invalid: return "Tok_Invalid";
			case Tok_Bang: return "Tok_Bang";
			case Tok_BangEq: return "Tok_BangEq";
			case Tok_Bang2Eq: return "Tok_Bang2Eq";
			case Tok_Hash: return "Tok_Hash";
			case Tok_Percent: return "Tok_Percent";
			case Tok_Amp: return "Tok_Amp";
			case Tok_2Amp: return "Tok_2Amp";
			case Tok_3Amp: return "Tok_3Amp";
			case Tok_Lpar: return "Tok_Lpar";
			case Tok_Latt: return "Tok_Latt";
			case Tok_Rpar: return "Tok_Rpar";
			case Tok_Star: return "Tok_Star";
			case Tok_Ratt: return "Tok_Ratt";
			case Tok_2Star: return "Tok_2Star";
			case Tok_Rcmt: return "Tok_Rcmt";
			case Tok_StarGt: return "Tok_StarGt";
			case Tok_Plus: return "Tok_Plus";
			case Tok_PlusColon: return "Tok_PlusColon";
			case Tok_Comma: return "Tok_Comma";
			case Tok_Minus: return "Tok_Minus";
			case Tok_MinusColon: return "Tok_MinusColon";
			case Tok_MinusGt: return "Tok_MinusGt";
			case Tok_Dot: return "Tok_Dot";
			case Tok_Slash: return "Tok_Slash";
			case Tok_Lcmt: return "Tok_Lcmt";
			case Tok_Colon: return "Tok_Colon";
			case Tok_Semi: return "Tok_Semi";
			case Tok_Lt: return "Tok_Lt";
			case Tok_2Lt: return "Tok_2Lt";
			case Tok_3Lt: return "Tok_3Lt";
			case Tok_Leq: return "Tok_Leq";
			case Tok_Eq: return "Tok_Eq";
			case Tok_2Eq: return "Tok_2Eq";
			case Tok_3Eq: return "Tok_3Eq";
			case Tok_EqGt: return "Tok_EqGt";
			case Tok_Gt: return "Tok_Gt";
			case Tok_Geq: return "Tok_Geq";
			case Tok_2Gt: return "Tok_2Gt";
			case Tok_3Gt: return "Tok_3Gt";
			case Tok_Qmark: return "Tok_Qmark";
			case Tok_At: return "Tok_At";
			case Tok_Lbrack: return "Tok_Lbrack";
			case Tok_Rbrack: return "Tok_Rbrack";
			case Tok_Hat: return "Tok_Hat";
			case Tok_HatTilde: return "Tok_HatTilde";
			case Tok_Lbrace: return "Tok_Lbrace";
			case Tok_Bar: return "Tok_Bar";
			case Tok_2Bar: return "Tok_2Bar";
			case Tok_Rbrace: return "Tok_Rbrace";
			case Tok_Tilde: return "Tok_Tilde";
			case Tok_TildeAmp: return "Tok_TildeAmp";
			case Tok_TildeHat: return "Tok_TildeHat";
			case Tok_TildeBar: return "Tok_TildeBar";
			case Tok_dlr_fullskew: return "Tok_dlr_fullskew";
			case Tok_dlr_hold: return "Tok_dlr_hold";
			case Tok_dlr_nochange: return "Tok_dlr_nochange";
			case Tok_dlr_period: return "Tok_dlr_period";
			case Tok_dlr_recovery: return "Tok_dlr_recovery";
			case Tok_dlr_recrem: return "Tok_dlr_recrem";
			case Tok_dlr_removal: return "Tok_dlr_removal";
			case Tok_dlr_setup: return "Tok_dlr_setup";
			case Tok_dlr_setuphold: return "Tok_dlr_setuphold";
			case Tok_dlr_skew: return "Tok_dlr_skew";
			case Tok_dlr_timeskew: return "Tok_dlr_timeskew";
			case Tok_dlr_width: return "Tok_dlr_width";
			case Tok_PATHPULSE_dlr: return "Tok_PATHPULSE_dlr";
			case Tok_always: return "Tok_always";
			case Tok_and: return "Tok_and";
			case Tok_assign: return "Tok_assign";
			case Tok_automatic: return "Tok_automatic";
			case Tok_begin: return "Tok_begin";
			case Tok_buf: return "Tok_buf";
			case Tok_bufif0: return "Tok_bufif0";
			case Tok_bufif1: return "Tok_bufif1";
			case Tok_case: return "Tok_case";
			case Tok_casex: return "Tok_casex";
			case Tok_casez: return "Tok_casez";
			case Tok_cell: return "Tok_cell";
			case Tok_cmos: return "Tok_cmos";
			case Tok_config: return "Tok_config";
			case Tok_deassign: return "Tok_deassign";
			case Tok_default: return "Tok_default";
			case Tok_defparam: return "Tok_defparam";
			case Tok_design: return "Tok_design";
			case Tok_disable: return "Tok_disable";
			case Tok_edge: return "Tok_edge";
			case Tok_else: return "Tok_else";
			case Tok_end: return "Tok_end";
			case Tok_endcase: return "Tok_endcase";
			case Tok_endconfig: return "Tok_endconfig";
			case Tok_endfunction: return "Tok_endfunction";
			case Tok_endgenerate: return "Tok_endgenerate";
			case Tok_endmodule: return "Tok_endmodule";
			case Tok_endprimitive: return "Tok_endprimitive";
			case Tok_endspecify: return "Tok_endspecify";
			case Tok_endtable: return "Tok_endtable";
			case Tok_endtask: return "Tok_endtask";
			case Tok_event: return "Tok_event";
			case Tok_for: return "Tok_for";
			case Tok_force: return "Tok_force";
			case Tok_forever: return "Tok_forever";
			case Tok_fork: return "Tok_fork";
			case Tok_function: return "Tok_function";
			case Tok_generate: return "Tok_generate";
			case Tok_genvar: return "Tok_genvar";
			case Tok_highz0: return "Tok_highz0";
			case Tok_highz1: return "Tok_highz1";
			case Tok_if: return "Tok_if";
			case Tok_ifnone: return "Tok_ifnone";
			case Tok_incdir: return "Tok_incdir";
			case Tok_include: return "Tok_include";
			case Tok_initial: return "Tok_initial";
			case Tok_inout: return "Tok_inout";
			case Tok_input: return "Tok_input";
			case Tok_instance: return "Tok_instance";
			case Tok_integer: return "Tok_integer";
			case Tok_join: return "Tok_join";
			case Tok_large: return "Tok_large";
			case Tok_liblist: return "Tok_liblist";
			case Tok_library: return "Tok_library";
			case Tok_localparam: return "Tok_localparam";
			case Tok_macromodule: return "Tok_macromodule";
			case Tok_medium: return "Tok_medium";
			case Tok_module: return "Tok_module";
			case Tok_nand: return "Tok_nand";
			case Tok_negedge: return "Tok_negedge";
			case Tok_nmos: return "Tok_nmos";
			case Tok_nor: return "Tok_nor";
			case Tok_noshowcancelled: return "Tok_noshowcancelled";
			case Tok_not: return "Tok_not";
			case Tok_notif0: return "Tok_notif0";
			case Tok_notif1: return "Tok_notif1";
			case Tok_or: return "Tok_or";
			case Tok_output: return "Tok_output";
			case Tok_parameter: return "Tok_parameter";
			case Tok_pmos: return "Tok_pmos";
			case Tok_posedge: return "Tok_posedge";
			case Tok_primitive: return "Tok_primitive";
			case Tok_pull0: return "Tok_pull0";
			case Tok_pull1: return "Tok_pull1";
			case Tok_pulldown: return "Tok_pulldown";
			case Tok_pullup: return "Tok_pullup";
			case Tok_pulsestyle_ondetect: return "Tok_pulsestyle_ondetect";
			case Tok_pulsestyle_onevent: return "Tok_pulsestyle_onevent";
			case Tok_rcmos: return "Tok_rcmos";
			case Tok_real: return "Tok_real";
			case Tok_realtime: return "Tok_realtime";
			case Tok_reg: return "Tok_reg";
			case Tok_release: return "Tok_release";
			case Tok_repeat: return "Tok_repeat";
			case Tok_rnmos: return "Tok_rnmos";
			case Tok_rpmos: return "Tok_rpmos";
			case Tok_rtran: return "Tok_rtran";
			case Tok_rtranif0: return "Tok_rtranif0";
			case Tok_rtranif1: return "Tok_rtranif1";
			case Tok_scalared: return "Tok_scalared";
			case Tok_showcancelled: return "Tok_showcancelled";
			case Tok_signed: return "Tok_signed";
			case Tok_small: return "Tok_small";
			case Tok_specify: return "Tok_specify";
			case Tok_specparam: return "Tok_specparam";
			case Tok_strong0: return "Tok_strong0";
			case Tok_strong1: return "Tok_strong1";
			case Tok_supply0: return "Tok_supply0";
			case Tok_supply1: return "Tok_supply1";
			case Tok_table: return "Tok_table";
			case Tok_task: return "Tok_task";
			case Tok_time: return "Tok_time";
			case Tok_tran: return "Tok_tran";
			case Tok_tranif0: return "Tok_tranif0";
			case Tok_tranif1: return "Tok_tranif1";
			case Tok_tri: return "Tok_tri";
			case Tok_tri0: return "Tok_tri0";
			case Tok_tri1: return "Tok_tri1";
			case Tok_triand: return "Tok_triand";
			case Tok_trior: return "Tok_trior";
			case Tok_trireg: return "Tok_trireg";
			case Tok_use: return "Tok_use";
			case Tok_uwire: return "Tok_uwire";
			case Tok_vectored: return "Tok_vectored";
			case Tok_wait: return "Tok_wait";
			case Tok_wand: return "Tok_wand";
			case Tok_weak0: return "Tok_weak0";
			case Tok_weak1: return "Tok_weak1";
			case Tok_while: return "Tok_while";
			case Tok_wire: return "Tok_wire";
			case Tok_wor: return "Tok_wor";
			case Tok_xnor: return "Tok_xnor";
			case Tok_xor: return "Tok_xor";
			case Tok_Attribute: return "Tok_Attribute";
			case Tok_Comment: return "Tok_Comment";
			case Tok_MacroUsage: return "Tok_MacroUsage";
			case Tok_Section: return "Tok_Section";
			case Tok_SectionEnd: return "Tok_SectionEnd";
			case Tok_CoDi: return "Tok_CoDi";
			case Tok_LineCont: return "Tok_LineCont";
			case Tok_Realnum: return "Tok_Realnum";
			case Tok_Natural: return "Tok_Natural";
			case Tok_SizedBased: return "Tok_SizedBased";
			case Tok_BasedInt: return "Tok_BasedInt";
			case Tok_BaseFormat: return "Tok_BaseFormat";
			case Tok_BaseValue: return "Tok_BaseValue";
			case Tok_SysName: return "Tok_SysName";
			case Tok_Ident: return "Tok_Ident";
			case Tok_Str: return "Tok_Str";
			case Tok_Eof: return "Tok_Eof";
			default: return "";
		}
	}
	bool tokenTypeIsLiteral( int r ) {
		return r > TT_Literals && r < TT_Keywords;
	}
	bool tokenTypeIsKeyword( int r ) {
		return r > TT_Keywords && r < TT_Specials;
	}
	bool tokenTypeIsSpecial( int r ) {
		return r > TT_Specials && r < TT_Max;
	}
}
