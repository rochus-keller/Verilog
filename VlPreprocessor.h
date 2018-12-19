#ifndef VLPREPROC_H
#define VLPREPROC_H

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

#include <QIODevice>
#include <QQueue>
#include <QHash>
#include <QStack>

namespace Vl
{
    class PpSymbols;
    class Errors;

    class Preprocessor : public QIODevice
    {
        Q_OBJECT
    public:
        enum Directive {
            Cd_invalid,
            Cd_begin_keywords,
            Cd_celldefine,
            Cd_default_nettype,
            Cd_define,
            Cd_else,
            Cd_elsif,
            Cd_end_keywords,
            Cd_endcelldefine,
            Cd_endif,
            Cd_ifdef,
            Cd_ifndef,
            Cd_include,
            Cd_line,
            Cd_nounconnected_drive,
            Cd_pragma,
            Cd_resetall,
            Cd_timescale,
            Cd_unconnected_drive,
            Cd_undef
        };

        explicit Preprocessor(QObject *parent = 0);
        ~Preprocessor();

        void setSource( QIODevice*, PpSymbols* = 0, Errors* = 0 );
        PpSymbols* getSyms() const { return d_syms; }
        static Directive getDirective( const QByteArray& );
        int getLnr() const { return d_lnr; }

        // public overrides
        bool isSequential() const { return true; }
        bool atEnd() const;
        qint64 bytesAvailable() const;

    signals:
        void sigInclude( const QString& );

    protected:
        void processLine();
        bool processDirective(QByteArray& , int&);
        void processDefineDecl( QByteArray, int pos );
        void processDefineUse(const QByteArray& id, QByteArray& line, int& pos);
        void recursiveDefineResolution( const QByteArray& topId, QByteArray& line );
        void processInclude( const QByteArray& line, int& pos );
        static QByteArray ident( const QByteArray& str, int& pos );
        static QByteArray extIdent( const QByteArray& str, int& pos );
        static QByteArray simpIdent( const QByteArray& str, int& pos );
        static int endsWithBackslash(const QByteArray& , int from = 0);
        QByteArrayList arguments( const QByteArray&, int& pos );
        QByteArrayList actualArgs(QByteArray&, int& pos , bool fetchLines);
        static void skipWhite( const QByteArray&, int& pos );
        void emitSubstitute( const QByteArray& str, int pos = 0, int len = -1 );
        void emitOriginal( const QByteArray& str, int pos = 0, int len = -1 );
        void emitNl();
        bool error( const QString& msg, int lnr );
        bool warning( const QString& msg, int lnr );
        QByteArray readLine();
        bool inline txOn() const;

        enum Hit { Nothing, CoDi, LineCmt, MultilineCmt };
        static Hit indexOf( const QByteArray&, int from, int& pos );

        // protected overrides
        qint64 readData(char * data, qint64 maxSize);
        qint64 writeData(const char * data, qint64 maxSize);

    private:
        enum IfState { NotInIf, InIf, InIfdef, InElse };
        QStack< QPair<quint8,bool> > d_ifState; // ifState, txOn
        QIODevice* d_in;
        PpSymbols* d_syms;
        int d_lnr;
        // QQueue<char> d_out;
        QByteArray d_out;
        Errors* d_err;
    };
}

#endif // VLPREPROC_H
