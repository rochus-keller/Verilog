#ifndef VLINCLUDES_H
#define VLINCLUDES_H

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
#include <QDir>
#include <QReadWriteLock>

namespace Vl
{
    class Includes : public QObject
    {
        Q_OBJECT
        // class is thread-safe
    public:
        explicit Includes(QObject *parent = 0);

        void addDir( const QDir& h );
        QString findPath( const QString& fileName );
        QString findPath( const QString& fileName, const QString& relativeTo );
        void setAutoAdd(bool on) { d_autoAdd = on; }
        bool getAutoAdd() const { return d_autoAdd; }
        void clear();

    private:
        QList<QDir> d_includes;
        QReadWriteLock d_lock;
        bool d_autoAdd;
    };
}

#endif // VLINCLUDES_H
