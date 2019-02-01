#ifndef VLPREPROCSYMS_H
#define VLPREPROCSYMS_H

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
#include <QHash>
#include <QReadWriteLock>
#include <Verilog/VlToken.h>

namespace Vl
{
    class PpSymbols : public QObject
    {
        // class is thread-safe
    public:
        struct Define
        {
            QByteArray d_name;
            QByteArrayList d_args;
            TokenList d_toks;
            QString d_sourcePath;
            quint32 d_lineNr;
            Define():d_lineNr(0) {}
        };
        typedef QHash<QByteArray,Define> Defines;

        explicit PpSymbols(QObject *parent = 0);

        void addSymbol( const QByteArray& name, const TokenList& val,
                        const QByteArrayList& args = QByteArrayList() );
        void addSymbol( const Define& );
        void addSymbol(const QByteArray&, const QByteArray&, TokenType type );

        const Define getSymbol( const QByteArray& id );
        QByteArrayList getNames() const;

        bool contains( const QByteArray& id ) const;
        void remove( const QByteArray& id );
        void clear();
    private:
        Defines d_defs;
        mutable QReadWriteLock d_lock;
    };
}

#endif // VLPREPROCSYMS_H
