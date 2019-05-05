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

#include "VlPpLexer.h"
#include "VlToken.h"
#include "VlErrors.h"
#include "VlNumLex.h"
#include "VlPpSymbols.h"
#include "VlIncludes.h"
#include "VlFileCache.h"
#include <QIODevice>
#include <QtDebug>
#include <QBuffer>
#include <QFileInfo>
#include <QDir>
using namespace Vl;

PpLexer::PpLexer(QObject *parent) :
    QObject(parent), d_lastT(Tok_Invalid), d_err(0), d_syms(0), d_ignoreComments(true),
    d_ignoreAttrs(true), d_ignoreHidden(true), d_packAttributes(true), d_packComments(true),
    d_filePathMode(false), d_incs(0),d_fcache(0),d_sendMacroUsage(false),d_supportSvExt(false)
{
}

bool PpLexer::setStream(QIODevice* in, const QString& sourcePath, bool reportError)
{
    if( in == 0 )
        return setStream( sourcePath, reportError );
    else
    {
        InputCtx ctx;
        ctx.d_in = in;
        ctx.d_sourcePath = sourcePath;
        d_source.push(ctx);
        if( d_fcache != 0 && d_source.size() == 1 )
        {
            d_supportSvExt = d_fcache->supportSvExt(sourcePath);
        }
        return true;
    }
}

bool PpLexer::setStream(const QString& sourcePath, bool reportError )
{
    foreach( const InputCtx& ctx, d_source )
    {
        if( ctx.d_sourcePath == sourcePath )
        {
            if( d_err )
                d_err->error(Errors::Lexer, sourcePath, 0, 0,
                         tr("recursive include of file %1").arg(sourcePath) );
            return false;
        }
    }

    QIODevice* in = 0;

    if( d_fcache )
    {
        bool found;
        QByteArray content = d_fcache->getFile(sourcePath, &found );
        if( found )
        {
            QBuffer* buf = new QBuffer(this);
            buf->setData( content );
            buf->open(QIODevice::ReadOnly);
            in = buf;
        }
    }

    if( in == 0 )
    {
        QFile* file = new QFile(sourcePath, this);
        if( !file->open(QIODevice::ReadOnly) )
        {
            if( d_err && reportError )
            {
                if( d_source.isEmpty() )
                    d_err->error(Errors::Lexer, sourcePath, 0, 0,
                                 tr("cannot open file from path %1").arg(sourcePath) );
                else
                    d_err->error( Errors::Preprocessor, getLastSource(), getLastLineNr(), getLastColNr(),
                                           tr("cannot open include file '%1'").arg(sourcePath) );
            }
            delete file;
            return false;
        }
        in = file;
    }
    // else
    setStream( in, sourcePath );
    return true;
}

Token PpLexer::nextToken()
{
    Token t;
    if( !d_buffer.isEmpty() )
    {
        t = d_buffer.first();
        d_buffer.pop_front();
    }else
        t = nextTokenPp();
    return t;
}

Token PpLexer::peekToken(quint8 lookAhead)
{
    Q_ASSERT( lookAhead > 0 );
    while( d_buffer.size() < lookAhead )
        d_buffer.push_back( nextTokenPp() );
    return d_buffer[ lookAhead - 1 ];
}

QList<Token> PpLexer::tokens(const QString& code)
{
    return tokens( code.toLatin1() );
}

QList<Token> PpLexer::tokens(const QByteArray& code, const QString& path)
{
    QBuffer in;
    in.setData( code );
    in.open(QIODevice::ReadOnly);
    setStream( &in, path );

    QList<Token> res;
    Token t = nextToken();
    while( t.isValid() )
    {
        res << t;
        t = nextToken();
    }
    return res;
}

QString PpLexer::getMainSource() const
{
    if( d_source.isEmpty() )
        return QString();
    else
        return d_source.first().d_sourcePath;
}

int PpLexer::getMainLineNr() const
{
    if( d_source.isEmpty() )
        return -1;
    else
        return d_source.first().d_lineNr;
}

int PpLexer::getMainColNr() const
{
    if( d_source.isEmpty() )
        return -1;
    else
        return d_source.first().d_colNr;
}

QString PpLexer::getCurSource() const
{
    if( d_source.isEmpty() )
        return QString();
    else
        return d_source.top().d_sourcePath;
}

int PpLexer::getCurLineNr() const
{
    if( d_source.isEmpty() )
        return -1;
    else
        return d_source.top().d_lineNr;
}

int PpLexer::getCurColNr() const
{
    if( d_source.isEmpty() )
        return -1;
    else
        return d_source.top().d_colNr;
}

QString PpLexer::getLastSource() const
{
    return d_lastT.d_sourcePath;
}

int PpLexer::getLastLineNr() const
{
    return d_lastT.d_lineNr;
}

int PpLexer::getLastColNr() const
{
    return d_lastT.d_colNr;
}

Token PpLexer::nextTokenImp()
{
    if( d_source.isEmpty() )
        return d_lastT; // Token(Tok_Eof);

	skipWhiteSpace();
    while( d_source.top().d_colNr >= d_source.top().d_line.size() )
	{
        if( d_source.top().d_in->atEnd() )
        {
            Token t = token( Tok_Eof, 0 );
            if( d_source.top().d_in->parent() == this )
                d_source.top().d_in->deleteLater();
            d_idols[d_source.top().d_sourcePath] = d_source.top().d_idol;
            d_source.pop();
            return t;
        }
		nextLine();
		skipWhiteSpace();
	}

    Q_ASSERT( d_source.top().d_colNr < d_source.top().d_line.size() );
    while( d_source.top().d_colNr < d_source.top().d_line.size() )
	{
        const char ch = d_source.top().d_line[d_source.top().d_colNr];

        if( ch == '"' )
            return string();

        if( ch == '\\')
        {
            if( lookAhead(1) == 0 )
                return token( Tok_LineCont );
            else
                return extident();
        }

        if( ::isdigit(ch) || ch == '\'' )
            return numeric();

        if( QChar::fromLatin1( ch ).isLetter() || ch == '_' || ch == '$' || ch == '`' )
        {
#ifdef VL_SV12
            if( d_supportSvExt && ch == '$' && ( ::isspace(lookAhead(1)) || lookAhead(1) == 0 ) )
                return token(Tok_Dlr);
#endif
            if( d_filePathMode )
                return pathident();
            // Identifier oder Reserved Word
            return ident();
        }

        // else
        int pos = d_source.top().d_colNr;
        TokenType tt = tokenTypeFromString(d_source.top().d_line,&pos);

#ifdef VL_SV12
        if( tt == Tok_ColonSlash && lookAhead(2) == '/' )
        {
            tt = Tok_Colon;
            pos--;
        }
#endif

        if( d_filePathMode )
        {
            if( tt == Tok_Star || tt == Tok_2Slash || tt == Tok_Dot || tt == Tok_Qmark )
                return pathident();
            if( tt == Tok_Semi )
                d_filePathMode = false;
        }

        if( tt == Tok_2Slash )
            return lineComment();
        if( tt == Tok_Lcmt )
            return blockComment();

        if( tt == Tok_Invalid || pos == d_source.top().d_colNr )
            // Error
            return token( Tok_Invalid, 1, tr("unexpected character '%1' %2").arg(QChar::fromLatin1(ch)).arg(int(ch)).toUtf8() );

        // else
        const int len = pos - d_source.top().d_colNr;
        return token( tt, len, d_source.top().d_line.mid( d_source.top().d_colNr,len) );
    }
	Q_ASSERT( false );
    return Tok_Invalid;
}

static inline bool isCondition(Directive d)
{
    return d >= Cd_ifdef && d <= Cd_endif;
}

Token PpLexer::nextTokenPp()
{
    Token t = nextTokenImp();
    while( true )
    {
        try
        {
            if( t.isEof() && !d_source.isEmpty() )
                t = nextTokenImp();

            const bool on = txOn();
            t.d_hidden = !on;

            if( on || !d_ignoreHidden )
            {
                if( t.d_type == Tok_Invalid )
                {
                    if( d_err == 0 )
                        return t;
                    else
                        // Already reported; TODO: gilt noch nicht für alle Fehler, z.B. ungültige Zeichen!
                        t = nextTokenImp();
                }else if( t.d_type == Tok_Comment && d_ignoreComments )
                {
                    t = nextTokenImp();
                }else if( t.d_type == Tok_Latt && ( d_packAttributes || d_ignoreAttrs ) )
                {
                    t = processAttribute();
                    if( d_ignoreAttrs )
                        t = nextTokenImp();
                }else if( d_syms && t.d_type == Tok_CoDi )
                {
                    t = processDirective( t );
                }else
                {
                    return t;
                }
            }else
            {
                Directive d;
                if( t.d_type == Tok_CoDi && isCondition( ( d = matchDirective(t.d_val) ) ) )
                {
                    t = processCondition(d);
                }else
                    t = nextTokenImp();
            }
        }catch( const Token& e )
        {
            t = e;
        }
    }
}

Token PpLexer::processDirective(const Token& tok )
{
    const Directive d = matchDirective(tok.d_val);
    if( d == Cd_include )
        return processInclude();
    else if( d == Cd_define )
        return processDefine();
    else if( d == Cd_undef )
    {
        Token t = nextTokenImp();
        if( t.d_type != Tok_Ident )
            return error("expecting identifier after `undef");
        if( d_syms && !d_syms->contains(t.d_val) )
            warning( tr("unknown macro '%1'").arg(t.d_val.data()));
        else if( d_syms )
            d_syms->remove(t.d_val);
        return nextTokenImp();
    }else if( d >= Cd_ifdef && d <= Cd_endif )
        return processCondition(d);
    else
    {
        if( d != Cd_Invalid )
        {
            if( d_err )
                d_err->warning( Errors::Preprocessor, d_lastT.d_sourcePath,
                                d_lastT.d_lineNr, d_lastT.d_colNr,
                                tr("compiler directive not implemented '%1'").arg(tok.d_val.data())  );
            // ignore rest of line up to nl
            d_source.top().d_colNr = d_source.top().d_line.size();
            return nextTokenImp();
        }else
            return processMacroUse( tok );
    }
    Q_ASSERT( false );
    return Token();
}

Token PpLexer::processInclude()
{
    Token t = nextTokenImp();
    if( t.d_type != Tok_Str )
        return error( "expecting filename string after include directive" );
    const QString path = QString::fromLatin1(t.d_val);
    QFileInfo inc( path );
    if( inc.filePath().isEmpty() || inc.isRelative() )
    {
        // no or relative path
        // try in the same directory as the current soruce
        if( !setStream( QFileInfo( d_source.top().d_sourcePath ).dir().absoluteFilePath(path), false ) && d_incs != 0 )
            setStream( d_incs->findPath(path), true );
    }else
        // absolute path
        setStream( path, true );
    return nextTokenImp();
}

static Token next( const QList<Token>& toks, int& i )
{
    if( i < toks.size() )
        return toks[i++];
    else
        return Token(Tok_Eof);
}

Token PpLexer::processDefine()
{
    Token t = nextTokenImp();
    const bool reserved = tokenIsReservedWord(t.d_type);
    if( t.d_type != Tok_Ident && !reserved )
        return error("expecting identifier after `define");
    if( reserved )
        t.d_type = Tok_Ident; // we use the token here as normal ident
    const bool hasArgs = d_source.top().d_colNr < d_source.top().d_line.size() &&
            d_source.top().d_line[d_source.top().d_colNr] == '(';
            // zwischen Ident und Klammer darf kein Space sein!
    const int topLnr = d_source.top().d_lineNr;
    const QString source = d_source.top().d_sourcePath;
    QByteArray line = QByteArray( d_source.top().d_colNr, ' ') + // to sync colNr with orig
            d_source.top().d_line.mid(d_source.top().d_colNr);
    d_source.top().d_colNr = d_source.top().d_line.size();
    while( line.endsWith( '\\' ) )
    {
        if( d_source.top().d_in->atEnd() )
            return error("define with line continuation at end of file");
        nextLine();
        line.chop(1);
        line += '\n';
        line += d_source.top().d_line;
        d_source.top().d_colNr = d_source.top().d_line.size();
    }

    PpSymbols::Define def;
    def.d_name = t.d_val;
    def.d_lineNr = topLnr;
    def.d_sourcePath = source;

    if( matchDirective( def.d_name ) != Cd_Invalid )
        return error("text macro names may not be the same as compiler directive keywords" );

    PpLexer lex;
    lex.setErrors( d_err );
    lex.setCache(d_fcache);
    QList<Token> toks = lex.tokens( line, source );

    int i = 0;
    t = next(toks, i);
    if( hasArgs && t.d_type == Tok_Lpar )
    {
        t = next(toks, i);
        // Read Arguments
        while( t.isValid() && t.d_type != Tok_Rpar )
        {
            if( t.d_type == Tok_Ident )
                def.d_args.append( t.d_val );
            else if( t.d_type != Tok_Comma && t.d_type != Tok_Rpar )
                return error( tr("invalid token in define argument list of '%1': %2").
                              arg(def.d_name.data()).arg(t.getName()) );
            t = next(toks, i);
        }
        if( t.d_type != Tok_Rpar )
            return error( tr("invalid argument list in macro definition '%1'").arg(def.d_name.data()) );
        t = next(toks, i);
    }

    // Read Macro Text
    while( t.isValid() && t.d_type != Tok_Comment )
    {
        t.d_lineNr += topLnr - 1;
        def.d_toks.append(t);
        t = next(toks, i);
        if( t.d_type == Tok_Invalid )
            return error( tr("invalid token in macro text '%1'").arg(def.d_name.data()) );
    }
    if( d_syms )
        d_syms->addSymbol( def );

    return nextTokenImp();
}

Token PpLexer::processAttribute()
{
    int pos = d_source.top().d_line.indexOf( "*)", d_source.top().d_colNr );
    if( pos != -1 )
    {
        const int len = pos - d_source.top().d_colNr;
        return token( Tok_Attribute, len + 2,
                      d_source.top().d_line.mid( d_source.top().d_colNr, len ) );
    }else
    {
        const int lnr = d_source.top().d_lineNr;
        const int cnr = d_source.top().d_colNr;
        QByteArray attr = d_source.top().d_line.mid( d_source.top().d_colNr );
        do
        {
            if( d_source.top().d_in->atEnd() )
                return error("non-terminated attribute");
            nextLine();
            int pos = d_source.top().d_line.indexOf( "*)" );
            if( pos != -1 )
            {
                attr += " " + d_source.top().d_line.left( pos );
                Token t( Tok_Attribute, lnr, cnr + 1, attr.size(), attr );
                t.d_sourcePath = d_source.top().d_sourcePath;
                d_lastT = t;
                d_source.top().d_colNr = pos + 2;
                return t;
            }else
                attr += " " + d_source.top().d_line;
        }while(true);
    }
    Q_ASSERT( false );
    return Token();
}

Token PpLexer::processCondition(Directive d)
{
    if( d == Cd_ifdef )
    {
        Token t = nextTokenImp();
        if( t.d_type != Tok_Ident )
            return error("expecting identifier after `ifdef");
        const bool tx = ( d_ifState.isEmpty() || d_ifState.top().second ) &&
                d_syms != 0 && d_syms->contains(t.d_val);
        d_ifState.push( qMakePair(quint8(tx ? IfActive : InIf),tx) );
//        if( d_passDirectives )
//            return t;
//        else
        txlog(t.d_lineNr + 1 );
            return nextTokenImp();
    }else if( d == Cd_ifndef )
    {
        Token t = nextTokenImp();
        if( t.d_type != Tok_Ident )
            return error("expecting identifier after `ifndef");
        const bool tx = ( d_ifState.isEmpty() || d_ifState.top().second ) &&
                d_syms != 0 && !d_syms->contains(t.d_val);
        d_ifState.push( qMakePair(quint8(tx ? IfActive : InIf),tx) );
//        if( d_passDirectives )
//            return t;
//        else
        txlog(t.d_lineNr + 1 );
            return nextTokenImp();
    }else if( d == Cd_elsif )
    {
        if( d_ifState.isEmpty() || ( d_ifState.top().first != InIf && d_ifState.top().first != IfActive ) )
            return error("`elsif not expected");
        else
        {
            Token t = nextTokenImp();
            if( t.d_type != Tok_Ident )
                return error("expecting identifier after `elsif");
            const bool tx = ( d_ifState.size() == 1 || d_ifState[d_ifState.size()-2].second ) &&
                    d_ifState.top().first != IfActive && d_syms != 0 && d_syms->contains(t.d_val);
            d_ifState.top().second = tx;
            if( tx )
                d_ifState.top().first = IfActive;
//            if( d_passDirectives )
//                return t;
//            else
            txlog(t.d_lineNr + ( tx ? 1 : -1 ) );
                return nextTokenImp();
        }
    }else if( d == Cd_else )
    {
        if( d_ifState.isEmpty() || ( d_ifState.top().first != InIf && d_ifState.top().first != IfActive ) )
            return error("`else not expected");
        else
        {
            const bool tx = ( d_ifState.size() == 1 || d_ifState[d_ifState.size()-2].second ) &&
                    d_ifState.top().first != IfActive;
            d_ifState.top().first = InElse;
            d_ifState.top().second = tx;
            txlog( d_lastT.d_lineNr + ( tx ? -1 : 1 ) );
        }
        return nextTokenImp();
    }else if( d == Cd_endif )
    {
        if( d_ifState.isEmpty() )
            return error("`endif not expected");
        else
        {
            d_ifState.pop();
        }
        txlog( d_lastT.d_lineNr - 1 );
        return nextTokenImp();
    }
    return Token();
}

static void replaceFormalByActualArg( TokenList& inout, const QByteArray& formalArg, const TokenList& actualArg )
{
    TokenList out;
    for( int i = 0; i < inout.size(); i++ )
    {
        if( inout[i].d_type == Tok_Ident && inout[i].d_val == formalArg )
            out << actualArg;
        else
            out << inout[i];
    }
    inout = out;
}

static QDebug& operator<<(QDebug& dbg, const Token& t)
{
    dbg << "Tok(" << t.getName() << " " << t.d_val << ")";
    return dbg;
}

QList<TokenList> PpLexer::fetchActualArgsFromList(const Token& codi,const TokenList& toks, int* startFrom )
{
    QList<TokenList> result;
    int off = 0;
    if( startFrom )
        off = *startFrom;
    if( off >= toks.size() || toks[off].d_type != Tok_Lpar )
    {
        throw error( "actual arguments expected", codi );
        return result;
    }
    off++;
    TokenList actualArg;
    int nestingLevel = 0;
    bool rparFound = false;
    while( off < toks.size() )
    {
        if( toks[off].d_type == Tok_Lpar )
            nestingLevel++;
        else if( ( toks[off].d_type == Tok_Rpar || toks[off].d_type == Tok_Comma ) )
        {
            if( nestingLevel == 0 )
            {
                // Klammern sind ausgeglichen, wir haben ein Argument fertig gelesen
                result.append(actualArg);
                actualArg.clear();
                if( toks[off].d_type == Tok_Rpar )
                {
                    rparFound = true;
                    break;
                }
            }else if( toks[off].d_type == Tok_Rpar )
                nestingLevel--;
        }else
            actualArg.append( toks[off] );
        off++;
    }
    if( !rparFound )
    {
        throw error( "nonterminated macro actual argument list", codi );
        result.clear();
    }
    if( startFrom )
        *startFrom = off;
    return result;
}

TokenList PpLexer::fetchLparToRpar()
{
    TokenList result;
    Token t = nextTokenImp();
    result.append(t);
    if( t.d_type != Tok_Lpar )
        return result;

    t = nextTokenImp();
    int nestingLevel = 0;
    while( t.isValid() )
    {
        result.append(t);
        if( t.d_type == Tok_Lpar )
            nestingLevel++;
        else if( t.d_type == Tok_Rpar )
        {
            if( nestingLevel == 0 )
            {
                if( t.d_type == Tok_Rpar )
                    // Klammern sind ausgeglichen, Funktion erfüllt
                    break;
            }else if( t.d_type == Tok_Rpar )
                nestingLevel--;
        }
        t = nextTokenImp();
    }
    return result;
}

bool PpLexer::resolveAllMacroUses(const Token& codi,const QByteArray& topMacroId, TokenList& macroText)
{
    while( true )
    {
        TokenList result;
        bool foundCodi = false;
        for( int i = 0; i < macroText.size(); i++ )
        {
            if( macroText[i].d_type == Tok_CoDi )
            {
                foundCodi = true;
                const QByteArray macroId = macroText[i].d_val;
                if( matchDirective(macroId) != Cd_Invalid )
                {
                    throw error( "macro text cannot contain other compiler directives than text macros", codi );
                    return false;
                }
                if( topMacroId == macroId )
                {
                    throw error( "macro expands directly or indirectly to text containing another usage of itself", codi );
                    return false;
                }
                PpSymbols::Define macroDef;
                if( d_syms == 0 || !d_syms->contains(macroId) )
                {
                    throw error( tr("unknown text macro '%1'").arg(macroId.data()), codi );
                    return false;
                }else
                    macroDef = d_syms->getSymbol(macroId);

                TokenList makroText = macroDef.d_toks;
                const QByteArrayList& formalArgs = macroDef.d_args;

                if( !formalArgs.isEmpty() )
                {
                    ++i;
                    const QList<TokenList> actualArgs = fetchActualArgsFromList( codi, macroText, &i );
                    if( formalArgs.size() != actualArgs.size() )
                    {
                        throw error( tr("wrong number of actual arguments in define '%1'" ).
                                     arg(macroId.data()), codi );
                        return false;
                    }
                    for( int j = 0; j < actualArgs.size(); j++ )
                        replaceFormalByActualArg( makroText, formalArgs[j], actualArgs[j] );
                }

                result << makroText;
            }else
                result << macroText[i];
        }
        macroText = result;
        if( !foundCodi )
            return true;
    }
    return true;
}

Token PpLexer::processMacroUse(const Token& curTok)
{
    const QByteArray makroId = curTok.d_val;
    PpSymbols::Define makroDef;
    if( d_syms == 0 || !d_syms->contains(makroId) )
    {
        return error( tr("unknown text macro '%1'").arg(makroId.data()), curTok );
        //return nextTokenImp();
    }else
        makroDef = d_syms->getSymbol(makroId);

    TokenList makroText = makroDef.d_toks;
    const QByteArrayList& formalArgs = makroDef.d_args;

    TokenList rawActualArgs;
    if( !formalArgs.isEmpty() )
    {
        rawActualArgs = fetchLparToRpar();
        const QList<TokenList> actualArgs = fetchActualArgsFromList( curTok, rawActualArgs );

        if( formalArgs.size() != actualArgs.size() )
            return error( tr("wrong number of actual arguments in macro '%1'").arg(makroId.data()), curTok );

        for( int i = 0; i < actualArgs.size(); i++ )
            replaceFormalByActualArg( makroText, formalArgs[i], actualArgs[i] );
    }
    resolveAllMacroUses( curTok, makroId, makroText );
    for( int i = 0; i < makroText.size(); i++ )
    {
        makroText[i].d_substituted = true; // check: don't substitute in every case; provide option
        makroText[i].d_lineNr = curTok.d_lineNr;
        makroText[i].d_colNr = curTok.d_colNr;
        makroText[i].d_sourcePath =curTok.d_sourcePath;
        makroText[i].d_len = curTok.d_len /*+ ( rawActualArgs.isEmpty() ? 0 :
                          rawActualArgs.last().d_colNr - rawActualArgs.first().d_colNr +
                                                                        rawActualArgs.last().d_len )*/;
    }
    const Token nextTafterM = nextTokenImp();
    makroText.append(nextTafterM);

    if( d_sendMacroUsage )
    {
        // return macro usage token immediately followed by raw actual args and substituted actual args per buffer
        Token mu = curTok;
        mu.d_type = Tok_MacroUsage;
        mu.d_prePp = true;
        for( int i = 0; i < rawActualArgs.size(); i++ )
            rawActualArgs[i].d_prePp = true;
        d_buffer << rawActualArgs;
        d_buffer << makroText;
        return mu;
    }else
    {
        // return the first token immediately and the remaining ones via buffer
        const Token nextTtoSend = makroText.first();
        makroText = makroText.mid(1);
        d_buffer << makroText;
        return nextTtoSend;
    }
}

void PpLexer::nextLine()
{
    d_source.top().d_colNr = 0;
    d_source.top().d_lineNr++;
    d_source.top().d_line = d_source.top().d_in->readLine();

    // see https://de.wikipedia.org/wiki/Zeilenumbruch
    if( d_source.top().d_line.endsWith("\r\n") )
        d_source.top().d_line.chop(2);
    else if( d_source.top().d_line.endsWith('\n') || d_source.top().d_line.endsWith('\r') || d_source.top().d_line.endsWith('\025') )
        d_source.top().d_line.chop(1);
}

void PpLexer::skipWhiteSpace()
{
    while( d_source.top().d_colNr < d_source.top().d_line.size() &&
           ::isspace( d_source.top().d_line[d_source.top().d_colNr] ) )
        d_source.top().d_colNr++;
}

char PpLexer::lookAhead(quint32 off) const
{
    if( int( d_source.top().d_colNr + off ) < d_source.top().d_line.size() )
	{
        return d_source.top().d_line[ d_source.top().d_colNr + off ];
	}else
        return 0;
}

char PpLexer::nextAfterSpace(quint32 off) const
{
    while( true )
    {
        const char ch = lookAhead(off++ );
        if( ch == 0 )
            return 0;
        if( !::isspace(ch) )
            return ch;
    }
    return 0;
}

Token PpLexer::token(TokenType tt, int len, const QByteArray& val)
{
    Q_ASSERT( !d_source.isEmpty() );
    Token t( tt, d_source.top().d_lineNr, d_source.top().d_colNr + 1, len, val );
    t.d_sourcePath = d_source.top().d_sourcePath;
    d_lastT = t;
    d_source.top().d_colNr += len;
    return t;
}

static inline bool isOctal( char ch )
{
    return ::isdigit(ch) && ch >= '0' && ch <= '7';
}

Token PpLexer::string()
{
	int off = 1;
    QByteArray res;
    res.reserve( d_source.top().d_line.size() );
	while(true)
	{
        const char ch = lookAhead(off);
        if( ch == '"' )
            break; // End of String
        else if( ch == '\\' )
		{
            const char la = lookAhead(off + 1);
            switch( la )
            {
            case 'n':
                res.append( '\n' );
                off += 2;
                break;
            case 't':
                res.append( '\t' );
                off += 2;
                break;
            case '"':
            case '\\':
                res.append( la );
                off += 2;
                break;
            default:
                if( isOctal(la ) )
                {
                    int n = 1;
                    if( isOctal(lookAhead(off+2)) )
                        n++;
                    if( isOctal(lookAhead(off+3)) )
                        n++;
                    const QByteArray octalStr = d_source.top().d_line.mid( d_source.top().d_colNr + off + 1, n );
                    off += n + 1;
                    const int octal = octalStr.toInt( 0, 8 );
                    if( octal > 0xff )
                        return token( Tok_Invalid, off, tr("invalid octal character in escape string").toUtf8() );
                    res.append( char(octal) );
                }else
                    return token( Tok_Invalid, off, tr("invalid escape string").toUtf8() );
                break;
            }
        }else if( QChar::fromLatin1(ch).isPrint() )
        {
			off++;
            res.append(ch);
        }else if( ch == '\t' )
        {
            off++;
            res.append(ch);
        }
        else if( ch == 0 )
            return token( Tok_Invalid, d_source.top().d_line.size() - d_source.top().d_colNr,
                          tr("non terminated string").toUtf8() );
	}

    return token( Tok_Str, off + 1, res );
}

Token PpLexer::ident()
{
    // lookAhead(0) isLetter or '$' or '_' or '`'
    int off = 1;
	while( true )
	{
        const char c = lookAhead(off);
        if( !QChar(c).isLetterOrNumber() && c != '_' && c != '$' ) // QChar wegen möglichen Umlauten
			break;
		else
			off++;
	}
    const QByteArray str = d_source.top().d_line.mid(d_source.top().d_colNr, off );
    Q_ASSERT( !str.isEmpty() );
    TokenType tt = Tok_Invalid;
    if( str[0] != '`' )
        tt = matchReservedWord( str );
    if( !d_supportSvExt && tokenIsSvReservedWord(tt) )
        tt = Tok_Invalid;
    if( tt != Tok_Invalid )
    {
        if( tt == Tok_library || tt == Tok_include )
            d_filePathMode = true;
        if( Tok_PATHPULSE_dlr )
            return token( tt, off, str );
        else
            return token( tt, off );
    }
    else if( str[0] == '$' )
        return token( Tok_SysName, off, str.mid(1) );
    else if( str[0] == '`' )
        return token( Tok_CoDi, off, str.mid(1) );
    else
        return token( Tok_Ident, off, str );
}

Token PpLexer::extident()
{
    // Hier wurde bereits geprüft, dass lookAhead(0) == \ gilt
    int off = 1;
    while( true )
    {
        const char c = lookAhead(off);
        if( ::isspace( c ) || c == 0 )
            break;
        else
            off++;
    }
    const QByteArray str = d_source.top().d_line.mid(d_source.top().d_colNr + 1, off - 1 );
    return token( Tok_Ident, off, str );
}

Token PpLexer::pathident()
{
    int off = 0;
    while( true )
    {
        const char c = lookAhead(off);
        if( c == '/' && lookAhead(off+1) == '/' )
        {
            // double slash still means line comment, isn't it?
            off--;
            break;
        }
        if( ::isspace(c) || c == ',' || c == ';' || c == 0 )
            break;
        else
            off++;
    }
    const QByteArray str = d_source.top().d_line.mid(d_source.top().d_colNr, off );
    return token( Tok_Ident, off, str );
}

Token PpLexer::numeric()
{
    NumLex np( d_source.top().d_line, d_source.top().d_colNr );
	if( !np.parse() )
	{
        return token( Tok_Invalid, np.getOff(), np.getError().toUtf8() );
    }else
    {
        // qDebug() << "### Parsed" << np.getKindName() << np.getSize() << np.getVal();
        return token( np.getTokenType(), np.getOff(), d_source.top().d_line.mid( d_source.top().d_colNr, np.getOff() ) );
    }
}

Token PpLexer::blockComment()
{
    // hier gilt lookAhead(0)==/ und lookAhead(1)==*
    const int startLine = d_source.top().d_lineNr;
    const int startCol = d_source.top().d_colNr;

    d_source.top().d_colNr += 2;
    int pos = d_source.top().d_line.indexOf( "*/", d_source.top().d_colNr );
    QByteArray str;
    while( pos == -1 && !d_source.top().d_in->atEnd() )
    {
        if( !str.isEmpty() )
            str += '\n';
        str += d_source.top().d_line.mid( d_source.top().d_colNr );
        nextLine();
        pos = d_source.top().d_line.indexOf( "*/" );
    }
    if( d_packComments && pos == -1 && d_source.top().d_in->atEnd() )
    {
        d_source.top().d_colNr = d_source.top().d_line.size();
        return Token( Tok_Invalid, startLine, startCol + 1, str.size(), tr("non-terminated block comment").toLatin1() );
    }
    if( !str.isEmpty() )
        str += '\n';
    str += d_source.top().d_line.mid( d_source.top().d_colNr, pos - d_source.top().d_colNr );
    Token t( ( d_packComments ? Tok_Comment : Tok_Lcmt ), startLine, startCol + 1, str.size() + 2, str );
    t.d_sourcePath = d_source.top().d_sourcePath;
    d_lastT = t;
    if( d_packComments )
    {
        t.d_len += 2;
        d_source.top().d_colNr = pos + 2; // konsumiere */
    }else
        d_source.top().d_colNr = pos; // lasse */ dem nächsten call von nextToken
    return t;
}

static inline bool isOpenPar( char c )
{
    switch( c )
    {
    case '{':
    case '(':
    case '[':
    case '>':
        return true;
    default:
        return false;
    }
}

static inline bool isClosePar( char c )
{
    switch( c )
    {
    case '}':
    case ')':
    case ']':
    case '<':
        return true;
    default:
        return false;
    }
}

Token PpLexer::lineComment()
{
    const int lenToEat = d_source.top().d_line.size() - d_source.top().d_colNr;
    TokenType tt = Tok_Invalid;
    int off = 0;
    const char la2 = lookAhead(2);
    if( isOpenPar(la2) )
    {
        tt = Tok_Section;
        off = 3;
        while( lookAhead(off) == la2 )
            off++;
    }else if( isClosePar(la2) )
    {
        tt = Tok_SectionEnd;
        off = 3;
        while( lookAhead(off) == la2 )
            off++;
    }else
    {
        tt = Tok_Comment;
        off = 2;
    }
    return token( tt, lenToEat, d_source.top().d_line.mid( d_source.top().d_colNr + off ).trimmed() );
}

Token PpLexer::error(const QString& msg)
{
    return error( msg, d_lastT );
}

Token PpLexer::error(const QString& msg, const Token& e)
{
    if( d_err )
        d_err->error(Errors::Lexer, e.d_sourcePath, e.d_lineNr, e.d_colNr, msg );
    Token t = e;
    t.d_type = Tok_Invalid;
    t.d_val = msg.toUtf8();
    return t;
}

void PpLexer::warning(const QString& msg)
{
    if( d_err )
        d_err->warning(Errors::Lexer, d_lastT.d_sourcePath, d_lastT.d_lineNr, d_lastT.d_colNr, msg );
}

bool PpLexer::txOn() const
{
    return d_syms == 0 || d_ifState.isEmpty() || d_ifState.top().second;
}

void PpLexer::txlog(quint32 line)
{
    if( d_source.isEmpty() )
        return;
    const bool on = txOn();
    if( d_source.top().d_lastOn != on )
    {
        d_source.top().d_lastOn = on;
        d_source.top().d_idol.append(line);
    }
}

