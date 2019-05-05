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

#include <QApplication>
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#include <QThread>
#include "VlFrontend.h"
#include "VlCrossRefModel.h"
#include "VlFileCache.h"
#include "VlPpLexer.h"
#include "VlErrors.h"
#include "VlParser.h"
#include "VlPpSymbols.h"
#include <QPlainTextEdit>

static QStringList collectFiles( const QDir& dir )
{
    QStringList res;
    QStringList files = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );

    foreach( const QString& f, files )
        res += collectFiles( QDir( dir.absoluteFilePath(f) ) );

    files = dir.entryList( QStringList() << QString("*.v")
                                           << QString("*.vl"),
                                           QDir::Files, QDir::Name );
    foreach( const QString& f, files )
    {
        res.append( dir.absoluteFilePath(f) );
    }
    return res;
}

static bool readFile( const QString& path, bool resOnly )
{

//#define LexerTest
#ifdef LexerTest
    qDebug() << "***** reading" << path;
    QFile in( path );
    if( !in.open(QIODevice::ReadOnly) )
    {
        qDebug() << "**** cannot open file" << path;
        return false;
    }
    Vl::PpLexer lex;
    Vl::PpSymbols s;
    lex.setSyms(&s);
    lex.setStream( &in, path );

    Vl::Token t = lex.nextToken();
    while( true )
    {
        if( !resOnly )
            qDebug() << t.getName() << t.d_lineNr << t.d_colNr << QString::fromLatin1(t.d_val);
        if( t.isEof() )
        {
            qDebug() << "OK";
            return true;
        }
        if( !t.isValid() )
        {
            qDebug() << "FAILED" << t.d_lineNr << t.d_colNr << QString::fromLatin1(t.d_val);
            return true;
        }
        t = lex.nextToken();
    }
    in.close();
    return true;
#else

    // mit PP braucht das gesamte Testverzeichnis (2k Verilog files) 1918 ms, ohne nur 718 ms

#define UseFrontend
#ifdef UseFrontend
    Vl::Frontend ff;
    const bool res = ff.process(path);
    return res;
#else
    Vl::Errors e;
    e.setShowWarnings(false);
    Vl::PpLexer lex;
    Vl::PpSymbols s;
    lex.setIgnoreAttrs(false);
    lex.setPackAttrs(false);
    lex.setSendMacroUsage(true);
    lex.setErrors(&e);
    lex.setSyms(&s);
    lex.setStream( path, true );
    Vl::Parser p;
    return p.parseFile( &lex, &e );
#endif
#endif
}

static void dumpErrors( Vl::Errors* errs )
{
    if( errs->getErrCount() == 0 )
    {
        qDebug() << "***** no errors";
        return;
    } // else

    Vl::Errors::EntriesByFile lbf = errs->getErrors();
    Vl::Errors::EntriesByFile::const_iterator i;
    for( i = lbf.begin(); i != lbf.end(); ++i )
    {
        foreach( const Vl::Errors::Entry& e, i.value() )
        {
            qDebug() << QFileInfo(i.key()).fileName() << e.d_line << e.d_col << e.d_msg;
        }
    }
    qDebug() << "*****" << errs->getErrCount() << "errors and" << errs->getWrnCount() << "warnings";
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString dirOrFilePath;
    bool isProject = false;
    const QStringList args = QApplication::arguments();
    for( int i = 1; i < args.size(); i++ ) // arg 0 enthaelt Anwendungspfad
    {
        if( args[i].startsWith( "-p" ) )
            isProject = true;
        else if( !args[ i ].startsWith( '-' ) )
        {
            dirOrFilePath = args[ i ];
        }else
        {
            qDebug() << "invalid command line parameter" << args[i];
            return -1;
        }
    }
    if( dirOrFilePath.isEmpty() )
    {
        qCritical() << "expecting a directory or file path";
        return -1;
    }

    QFileInfo info(dirOrFilePath);

    QElapsedTimer t;
    t.start();
    int count = 0, succ = 0;
    if( info.isDir() )
    {
        const QStringList files = collectFiles( info.absoluteFilePath() );
        count = files.size();
        if( isProject )
        {
            qDebug() << "*****" << "parsing" << files.size() << "files";
//#define _USE_CODEMODEL
#ifdef _USE_CODEMODEL
            Vl::CodeModel m;
            m.readProject(files);
#else
            Vl::FileCache c;
            Vl::CrossRefModel m(0,&c);
            m.updateFiles(files, true);
            dumpErrors(m.getErrs());
#endif
        }else
            foreach( const QString& f, files )
            {
                if( readFile(f, true) )
                    succ++;
            }
    }else
    {
        count = 1;
        if( isProject )
        {
#ifdef _USE_CODEMODEL
            Vl::CodeModel m;
            m.readProject( QStringList() << info.absoluteFilePath() );
#else
            Vl::CrossRefModel m;
            m.updateFiles( QStringList() << info.absoluteFilePath(),true );
            dumpErrors(m.getErrs());
#endif
        }else
            if( readFile( info.absoluteFilePath(), false) )
                succ++;
    }
    if( !isProject )
        qDebug() << "Elapsed time [ms]:" << t.elapsed() << "Success:" << succ << "/" << count;

    return 0 ; // a.exec(); // TEST
}
