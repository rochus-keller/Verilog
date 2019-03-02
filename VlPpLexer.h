#ifndef VLPPLEXER_H
#define VLPPLEXER_H

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
#include <QVariant>
#include <QStack>
#include <Verilog/VlToken.h>

class QIODevice;

namespace Vl
{
    class Errors;
    class PpSymbols;
    class Includes;
    class FileCache;

	class PpLexer : public QObject
	{
        // this class is reentrant
	public:
        typedef QList<quint32> IfDefOutList; // Jeder Eintrag ist die Zeile der Änderung. Start bei On.
        typedef QMap<QString,IfDefOutList> IfDefOutLists;

		explicit PpLexer(QObject *parent = 0);

        void setIgnoreComments( bool b ) { d_ignoreComments = b; }
        void setPackComments( bool b ) { d_packComments = b; }
        void setIgnoreAttrs( bool b ) { d_ignoreAttrs = b; }
        void setPackAttrs( bool b ) { d_packAttributes = b; }
        void setSendMacroUsage( bool b ) { d_sendMacroUsage = b; }
        void setSupportSvExt( bool b ) { d_supportSvExt = b; }

        void setErrors(Errors* p) { d_err = p; }
        void setSyms(PpSymbols* p) { d_syms = p; }
        void setIncs(Includes* p) { d_incs = p; }
        void setCache(FileCache* p) { d_fcache = p; }

        bool setStream( QIODevice* in, const QString& sourcePath, bool reportError = false );
        bool setStream(const QString& sourcePath , bool reportError);

		Token nextToken();
        Token peekToken(quint8 lookAhead = 1);
        QList<Token> tokens( const QString& code );
        QList<Token> tokens( const QByteArray& code, const QString& path = QString() );

        // Main File
        QString getMainSource() const;
        int getMainLineNr() const;
        int getMainColNr() const;
        // Current Token
        QString getCurSource() const;
        int getCurLineNr() const;
        int getCurColNr() const;
        // Last Token
        QString getLastSource() const;
        int getLastLineNr() const;
        int getLastColNr() const;

        const IfDefOutLists& getIdols() const { return d_idols; }

    protected:
        Token nextTokenImp();
        Token nextTokenPp();
        Token processDirective(const Token& tok);
        Token processInclude();
        Token processDefine();
        Token processAttribute();
        Token processCondition(Directive);
        QList<TokenList> fetchActualArgsFromList(const Token& codi, const TokenList& text, int* startFrom = 0 );
        TokenList fetchLparToRpar();
        bool resolveAllMacroUses(const Token& codi, const QByteArray& topId, TokenList& text );
        Token processMacroUse(const Token& tok);
        void nextLine();
		void skipWhiteSpace();
		char lookAhead( quint32 ) const;
        char nextAfterSpace( quint32 ) const;
        Token token(TokenType, int len = 1, const QByteArray& val = QByteArray() );
        Token string();
		Token ident();
        Token extident();
        Token pathident();
		Token numeric();
        Token blockComment();
        Token lineComment();
        Token error( const QString& );
        Token error( const QString&, const Token& );
        void warning( const QString& );
        bool txOn() const;
        void txlog(quint32 line);
    private:
        Q_DISABLE_COPY(PpLexer)
        struct InputCtx
        {
            InputCtx():d_in(0),d_lineNr(0),d_colNr(0),d_lastOn(true){}

            QIODevice* d_in;
            quint32 d_lineNr; // current line, starting with 1
            quint16 d_colNr;  // current column (left of char), starting with 0
            QByteArray d_line;
            QString d_sourcePath;
            IfDefOutList d_idol;
            bool d_lastOn;
        };
        QStack<InputCtx> d_source;
        Token d_lastT;
        QList<Token> d_buffer;
        Errors* d_err;
        PpSymbols* d_syms;
        Includes* d_incs;
        FileCache* d_fcache;
        enum IfState { InIf, IfActive, InElse };
        QStack< QPair<quint8,bool> > d_ifState; // ifState, txOn
        IfDefOutLists d_idols;
        bool d_ignoreComments;  // don't deliver comment tokens
        bool d_packComments;    // Only deliver one Tok_Comment for /**/ instead of Tok_Lcmt and Tok_Rcmt
        bool d_ignoreHidden;    // don't deliver tokens in false compiler directive condition blocks
        bool d_packAttributes;  // Pack everything between Tok_Latt and Tok_Ratt in a Tok_Attribute
        bool d_ignoreAttrs;     // Ignore Tok_Latt and Tok_Ratt and everything between
        bool d_filePathMode;    // 'library' or 'include' detected, using pathident instead of ident up to ';'
        bool d_sendMacroUsage;  // deliver Tok_MacroUsage + actual args
        bool d_supportSvExt;    // Support SystemVerilog12 extensions
    };
}

#endif // VLPPLEXER_H
