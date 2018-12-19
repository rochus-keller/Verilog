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

#include "VlPreprocessor.h"
#include "VlPpSymbols.h"
#include "VlToken.h"
#include "VlErrors.h"
#include <QtDebug>
using namespace Vl;

static QHash<QByteArray,Preprocessor::Directive> s_predefs;

Preprocessor::Preprocessor(QObject *parent) :
    QIODevice(parent),d_in(0),d_lnr(0),d_syms(0),d_err(0)
{

}

Preprocessor::~Preprocessor()
{
}

void Preprocessor::setSource(QIODevice* in, PpSymbols* syms, Errors* err)
{
    d_in = in;
    d_err = err;
    if( syms )
    {
        if( d_syms != 0 && d_syms != syms )
        {
            if( d_syms->parent() == this )
                d_syms->deleteLater();
        }
        d_syms = syms;
    }
    if( d_in != 0 && d_syms == 0 )
    {
        d_syms = new PpSymbols( this );
    }
    close();
    d_out.clear();
    d_lnr = 0;
    open(ReadOnly);
}

Preprocessor::Directive Preprocessor::getDirective(const QByteArray& str)
{
    if( s_predefs.isEmpty() )
    {
        s_predefs.insert( "begin_keywords", Cd_begin_keywords );
        s_predefs.insert( "celldefine", Cd_celldefine );
        s_predefs.insert( "default_nettype", Cd_default_nettype );
        s_predefs.insert( "define", Cd_define );
        s_predefs.insert( "else", Cd_else );
        s_predefs.insert( "elsif", Cd_elsif );
        s_predefs.insert( "end_keywords", Cd_end_keywords );
        s_predefs.insert( "endcelldefine", Cd_endcelldefine );
        s_predefs.insert( "endif", Cd_endif );
        s_predefs.insert( "ifdef", Cd_ifdef );
        s_predefs.insert( "ifndef", Cd_ifndef );
        s_predefs.insert( "include", Cd_include );
        s_predefs.insert( "line", Cd_line );
        s_predefs.insert( "nounconnected_drive", Cd_nounconnected_drive );
        s_predefs.insert( "pragma", Cd_pragma );
        s_predefs.insert( "resetall", Cd_resetall );
        s_predefs.insert( "timescale", Cd_timescale );
        s_predefs.insert( "unconnected_drive", Cd_unconnected_drive );
        s_predefs.insert( "undef", Cd_undef );
    }
    return s_predefs.value( str, Cd_invalid );
}

bool Preprocessor::atEnd() const
{
    return ( d_in != 0 && d_in->atEnd() ) && bytesAvailable() == 0;
}

qint64 Preprocessor::bytesAvailable() const
{
    return d_out.size() + QIODevice::bytesAvailable();
}

QByteArray Preprocessor::simpIdent( const QByteArray& str, int& pos )
{
    if( pos < 0 )
        return QByteArray();
    const int start = pos;
    // pos steht auf dem ersten Zeichen des Ident
    for( ; pos < str.size(); pos++ )
    {
        const char ch = str[pos];

        if( !::isalnum(ch) && ch != '$' && ch != '_'  )
            return str.mid( start, pos - start );
    }
    // Ident reicht bis Ende str
    return str.mid(start);
}

int Preprocessor::endsWithBackslash(const QByteArray& str, int from )
{
    if( str.indexOf( "//", from ) != -1 )
        return 0; // Im Falle eines Zeilenkommentars wird der abschliessende Backslash ignoriert
    int res = 0;
    for( int i = str.size() - 1; i >= 0; i-- )
    {
        if( ::isspace(str[i]) )
            res++;
        else if( str[i] == '\\' )
            return res + 1;
        else
            return 0;
    }
    return res;
}

QByteArrayList Preprocessor::arguments(const QByteArray& str, int& pos)
{
    if( pos >= str.size() )
        return QByteArrayList();
    if( str[pos] != '(' )
        return QByteArrayList();
    pos++;
    skipWhite(str,pos);
    QByteArrayList res;
    if( pos < str.size() && str[pos] == ')' )
        return res;
    while( pos < str.size() )
    {
        skipWhite(str,pos);
        res.append( simpIdent( str, pos ) );
        skipWhite(str,pos);
        if( str[pos] == ',' )
            pos++;
        else if( str[pos] == ')' )
        {
            pos++;
            return res;
        }else
            break;
    }
    error( "error in argument syntax", d_lnr );
    return res;
}

static inline char la( const QByteArray& line, int pos )
{
    if( pos + 1 < line.size() )
        return line[pos+1];
    else
        return 0;
}

QByteArrayList Preprocessor::actualArgs(QByteArray& line, int& pos, bool fetchLines )
{
    if( pos >= line.size() )
        return QByteArrayList();

    if( line[pos] != '(' )
    {
        error( "actual arguments expected", d_lnr );
        return QByteArrayList();
    }
    pos++;
    QByteArrayList res;
    QByteArray curArg;
    while( true )
    {
        int start = pos;
        bool inExpr = true;
        bool inString = false;
        int parCount = 0;
        for( ; pos < line.size(); pos++ )
        {
            const char ch = line[pos];
            if( inExpr && ch == '(' )
                parCount++;
            else if( inExpr && ( ch == ')' || ch == ',' ) )
            {
                if( parCount == 0 )
                {
                    // Klammern sind ausgeglichen, wir haben ein Argument fertig gelesen
                    curArg += line.mid(start, pos - start ) + ' ';
                    res.append( curArg.simplified() );
                    curArg.clear();
                    if( ch == ')' )
                    {
                        pos++;
                        return res;
                    }else
                        start = pos + 1;
                }else if( ch == ')' )
                    parCount--;
            }else if( ch == '"' )
            {
                inExpr = !inExpr;
                inString = inExpr;
            }else if( ch == '\\' )
            {
                if( !inExpr && la(line,pos) == '"' )
                    pos++;
            }else if( inExpr && ch == '/' )
            {
                if( la(line,pos) == '/' )
                {
                    // Line comment found; ignore to end of line, ignore comment
                    break;
                }else if( la(line,pos) == '*' )
                    // Multiline comment found; just go over it to end of comment
                    inExpr = !inExpr;
            }else if( !inExpr && !inString && ch == '*' && la(line,pos) == '/' )
                // end of comment found
                inExpr = !inExpr;
        }
        // line was finished before ',' or ')'
        curArg += line.mid(start, pos - start ) + ' ';
        if( d_in->atEnd() || !fetchLines )
            return res;
        line = readLine();
        d_lnr++;
        emitNl();
        pos = 0;
    }
    Q_ASSERT( false );
    return res;
}

void Preprocessor::skipWhite(const QByteArray& str, int& pos)
{
    while( pos < str.size() && ::isspace(str[pos]) )
        pos++;
}

void Preprocessor::emitSubstitute(const QByteArray& str, int pos, int len)
{
    if( txOn() )
        emitOriginal( str, pos, len );
}

void Preprocessor::emitOriginal(const QByteArray& str, int pos, int len)
{
    //const char* str2 = str.constData() + pos;
    if( !txOn() )
    {
        d_out += "//";
        //d_out.enqueue('/');
        //d_out.enqueue('/');
    }
    if( len < 0 )
        len = str.size();
    else
        len = qMin( str.size(), len );
    // qDebug() << "emit" << str << pos << len << str.mid(pos,len);
//    for( int i = pos; i < len; i++ )
//        d_out.enqueue( str[i] );
    d_out += str.mid(pos,len);
}

void Preprocessor::emitNl()
{
    // d_out.enqueue( '\n' );
    d_out += '\n';
}

bool Preprocessor::error(const QString& msg, int lnr)
{
    if( d_err )
        d_err->error(Errors::Preprocessor, QString(), lnr, 0, msg ); // TODO
    else
        qCritical() << "ERROR Preprocessor line" << lnr << msg;
    return false;
}

bool Preprocessor::warning(const QString& msg, int lnr)
{
    if( d_err )
        d_err->warning(Errors::Preprocessor, QString(), lnr, 0, msg ); // TODO
    else
        qWarning() << "WARNING Preprocessor line" << lnr << msg;
}

QByteArray Preprocessor::readLine()
{
    return d_in->readLine();
}

bool Preprocessor::txOn() const
{
    return d_ifState.isEmpty() || d_ifState.top().second;
}

Preprocessor::Hit Preprocessor::indexOf(const QByteArray& str, int from, int& pos)
{
    pos = from;
    while( pos < str.size() )
    {
        if( str[pos] == '`' )
            return CoDi;
        if( str[pos] == '/' && pos+1 < str.size() )
        {
            if( str[pos+1] == '*' )
                return MultilineCmt;
            else if( str[pos+1] == '/' )
                return LineCmt;
        }
        pos++;
    }
    pos = -1;
    return Nothing;
}

QByteArray Preprocessor::extIdent( const QByteArray& str, int& pos )
{
    if( pos < 0 )
        return QByteArray();
    const int start = pos;
    // pos steht auf backslash
    pos++;
    for( ; pos < str.size(); pos++ )
    {
        const char ch = str[pos];

        if( ::isspace(ch)  )
            return str.mid(start + 1, pos - start + 1 );
    }
    return str.mid(start + 1 );
}

QByteArray Preprocessor::ident( const QByteArray& str, int& pos )
{
    if( pos < 0 || str.isEmpty() )
        return QByteArray();
    if( str[0] == '\\' )
        return extIdent(str,pos);
    else
        return simpIdent(str,pos);
}

void Preprocessor::processLine()
{
    Q_ASSERT( !d_in->atEnd() );
    QByteArray line = readLine();
    d_lnr++;

    int pos = 0;
    while( processDirective(line, pos) )
        ;
    if( pos >= 0 )
        emitOriginal( line, pos );

}

bool Preprocessor::processDirective( QByteArray& line, int& pos )
{
    int hitPos = -1;
    Hit hit = indexOf( line, pos, hitPos );
    switch( hit )
    {
    case Nothing:
    case LineCmt:
        // Zeile enthält keine weitere Compiler Directive
        return false;
    case CoDi:
        emitOriginal( line, pos, hitPos - pos );
        pos = hitPos;
        break;
    case MultilineCmt:
        emitOriginal( line, pos, hitPos - pos ); // schreibe letzte Position bis vor /*
        pos = hitPos;
        hitPos = line.indexOf("*/", pos ); // Comment end ist noch auf selber Zeile
        if( hitPos != -1 )
        {
            emitOriginal( line, pos, hitPos - pos + 2 );
            pos = hitPos + 2;
            return true;
        }else
        {
            emitOriginal( line, pos );
            pos = line.size();
        }
        while( !d_in->atEnd() )
        {
            // Eat multiline comment
            line = readLine();
            d_lnr++;
            hitPos = line.indexOf("*/");
            if( hitPos != -1 )
            {
                pos = hitPos + 2;
                emitOriginal( line, 0, pos );
                return true;
            }else
                emitOriginal( line );
        }
        // hier kommen wir nur hin wenn in->atEnd!
        return false;
    }

    const QByteArray id = ident(line,++pos);
    const Directive dir = getDirective(id);

    if( dir == Cd_define )
    {
        processDefineDecl( line, pos );
        pos = -1;
        return false;
    }else if( dir == Cd_undef )
    {
        const QByteArray id2 = ident(line,++pos);
        Q_ASSERT( d_syms );
        if( !d_syms->contains(id2) )
            warning( tr("unknown macro '%1'").arg(id2.data()),d_lnr);
        else
            d_syms->remove(id2);
        return true;
    }else if( dir == Cd_ifdef )
    {
        const QByteArray id2 = ident(line,++pos);
        const bool tx = txOn() && d_syms->contains(id2);
        d_ifState.push( qMakePair(quint8(InIf),tx) );
        return true;
    }else if( dir == Cd_ifndef )
    {
        const QByteArray id2 = ident(line,++pos);
        const bool tx = txOn() && !d_syms->contains(id2);
        d_ifState.push( qMakePair(quint8(InIf),tx) );
        return true;
    }else if( dir == Cd_elsif )
    {
        if( d_ifState.isEmpty() || d_ifState.top().first != InIf )
            error("`elsif not expected",d_lnr);
        else
        {
            const QByteArray id2 = ident(line,++pos);
            const bool tx = ( d_ifState.size() == 1 || d_ifState[d_ifState.size()-2].second ) &&
                    d_syms->contains(id2);
            d_ifState.top().second = tx;
        }
        return true;
    }else if( dir == Cd_else )
    {
        if( d_ifState.isEmpty() || d_ifState.top().first != InIf )
            error("`else not expected",d_lnr);
        else
        {
            const bool tx = ( d_ifState.size() == 1 || d_ifState[d_ifState.size()-2].second ) &&
                    !d_ifState.top().second;
            d_ifState.top().second = tx;
        }
    }else if( dir == Cd_endif )
    {
        if( d_ifState.isEmpty() )
            error("`endif not expected",d_lnr);
        else
        {
            d_ifState.pop();
        }
    }else if( dir == Cd_include )
    {
        processInclude( line, pos );
        return true;
    }else
    {
        if( dir != Cd_invalid )
        {
            warning( tr("compiler directive not implemented '%1'").arg(id.data()),d_lnr );
            // ignore rest of line up to nl
            pos = line.size() - 1;
        }else
            processDefineUse( id, line, pos );
        return true;
    }
}

void Preprocessor::processDefineDecl(QByteArray line, int pos)
{
    const int lnr = d_lnr;
    int off;
    while( ( off = endsWithBackslash(line, pos ) ) )
    {
        if( d_in->atEnd() )
        {
            error("define with line continuation at end of file",lnr);
            return;
        }
        line.chop(off);
        line += '\n';
        line += readLine();
        emitNl(); // Lexer erhält nur eine leere Zeile
        d_lnr++;
    }
    emitNl();
    // qDebug() << "Define lines:" << line;
    skipWhite(line,pos);

    PpSymbols::Define def;
    def.d_name = ident(line,pos);
    // defines use all remaining text on the line up to // or EOL; \EOL includes the next line
    def.d_args = arguments(line,pos);
    const int pos2 = line.indexOf( "//", pos );
    if( pos2 == -1 )
        def.d_val = line.mid(pos).trimmed();
    else
        def.d_val = line.mid(pos, pos2 - pos).trimmed();

    if( getDirective( def.d_name ) != Tok_Invalid )
        error("Text macro names may not be the same as compiler directive keywords", lnr );
    foreach( const QByteArray& a, def.d_args )
    {
        if( a.isEmpty() )
            error( tr("invalid argument list in define '%1'").arg(def.d_name.data()), lnr );
    }
    Q_ASSERT( d_syms );
    d_syms->addSymbol( def );
}

void Preprocessor::processDefineUse(const QByteArray& id, QByteArray& line, int& pos )
{
    PpSymbols::Define def;
    Q_ASSERT( d_syms );
    if( !d_syms->contains(id) )
    {
        warning( tr("unknown define '%1'").arg(id.data()),d_lnr);
        return;
    }else
        def = d_syms->getSymbol(id);
    QByteArray val = def.d_val;
    if( !def.d_args.isEmpty() )
    {
        skipWhite( line, pos );
        const QByteArrayList args = actualArgs( line, pos, true );
        //qDebug() << "actualArgs" << args;
        if( def.d_args.size() != args.size() )
        {
            error( tr("wrong number of actual arguments in define '%1'").arg(id.data()),d_lnr);
            return;
        }
        for( int i = 0; i < args.size(); i++ )
            val.replace( def.d_args[i], args[i] );
        // TODO: aufgelöstes Makro kann weitere `x enthalten; diese können einerseits gleich heissen wie ein
        // argument, und müssen anderseits ebenfalls aufgelöst werden

        // NOTE: wenn das Makro weitere Macro Uses enthält mit dem Namen eines Parameters, werden diese durch
        // den Wert des Paramenters ersetzt!
    }
    recursiveDefineResolution( id, val );

    emitSubstitute( val.simplified() );
}

void Preprocessor::recursiveDefineResolution(const QByteArray& topId, QByteArray& line)
{
    int count = 0;
    int pos = -1;
    while( ( pos = line.indexOf( '`' ) ) != -1 )
    {
        const int start = pos;
        const QByteArray id = ident(line,++pos);
        const Directive dir = getDirective(id);
        if( dir != Cd_invalid )
        {
            error( "macro text cannot contain other compiler directives than text macros", d_lnr );
            return;
        }
        if( topId == id )
        {
            error( "macro expands directly or indirectly to text containing another usage of itself", d_lnr );
            return;
        }
        PpSymbols::Define def;
        Q_ASSERT( d_syms );
        if( !d_syms->contains(id) )
        {
            error( tr("unknown define '%1'").arg(id.data()),d_lnr);
            return;
        }else
            def = d_syms->getSymbol(id);
        QByteArray val = def.d_val;
        if( !def.d_args.isEmpty() )
        {
            skipWhite( line, pos );
            const QByteArrayList args = actualArgs( line, pos, false );
            //qDebug() << "actualArgs" << args;
            if( def.d_args.size() != args.size() )
            {
                error( tr("wrong number of actual arguments in define '%1'").arg(id.data()),d_lnr);
                return;
            }
            for( int i = 0; i < args.size(); i++ )
                val.replace( def.d_args[i], args[i] );
            // aufgelöstes Makro kann weitere `x enthalten; diese können einerseits gleich heissen wie ein
            // argument, und müssen anderseits ebenfalls aufgelöst werden

            // NOTE: wenn das Makro weitere Macro Uses enthält mit dem Namen eines Parameters, werden diese durch
            // den Wert des Paramenters ersetzt!
        }
        if( count > 1 )
            ; // qDebug() << "Preprocessor::recursiveDefineResolution iteration" << count;
        line.replace( start, pos - start, val );
        count++;
    }
}

void Preprocessor::processInclude(const QByteArray& line, int& pos)
{
    skipWhite( line, pos );
    if( pos >= line.size() || line[pos] != '"' )
    {
        error( "expecting filename in include directive", d_lnr );
        return;
    }
    pos++;
    const int pos2 = line.indexOf('"', pos );
    if( pos2 == -1 )
    {
        error( "invalid filename in include directive", d_lnr );
        return;
    }else
    {
        const QByteArray path = line.mid(pos, pos2-pos );
        pos = pos2 + 1;
        if( path.isEmpty() )
        {
            error( "empty filename in incude directive", d_lnr );
            return;
        }else
            emit sigInclude( path );
    }
}

qint64 Preprocessor::readData(char* data, qint64 maxSize)
{
    if( d_in == 0 || !d_in->isReadable() || d_in->atEnd() )
        return -1;

    while( d_out.size() < maxSize && !d_in->atEnd() )
        processLine();

    const int len = qMin( int(maxSize), d_out.size() );
    ::memcpy( data, d_out.data(), len ); // viel schneller als d_out.dequeue();
    if( len == d_out.size() )
        d_out.clear();
    else
        d_out = d_out.mid(len);
//    for( int i = 0; i < len; i++ )
//        data[i] = d_out.dequeue();

    //qDebug() << "readData" << QByteArray( data, len );
    return len;
}

qint64 Preprocessor::writeData(const char* data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

