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

#include "VlIncludes.h"
#include <QFileInfo>
#include <QtDebug>
using namespace Vl;

Includes::Includes(QObject *parent) : QObject(parent),d_autoAdd(false)
{

}

void Includes::addDir(const QDir& dir)
{
    d_lock.lockForWrite();
    if( !d_includes.contains(dir) )
        d_includes.append( dir );
    d_lock.unlock();
}

QString Includes::findPath(const QString& fileName)
{
    QString res;
    QFileInfo info(fileName);
    if( info.isRelative() )
    {
        d_lock.lockForRead();
        foreach( const QDir& dir, d_includes )
        {
            const QString path = dir.absoluteFilePath(fileName);
            QFileInfo info(path);
            if( info.exists() )
            {
                if( d_autoAdd )
                    addDir( info.dir() );
                res = path;
            }
        }
        d_lock.unlock();
    }else if( info.exists() )
    {
        if( d_autoAdd )
            addDir( info.dir() );
        res = info.absoluteFilePath();
    }
    return res;
}

QString Includes::findPath(const QString& fileName, const QString& relativeTo)
{
    QFileInfo inc( fileName );
    if( inc.filePath().isEmpty() || inc.isRelative() )
    {
        // no or relative path
        // try in the same directory as the current soruce
        QFileInfo file = QFileInfo( relativeTo ).dir().absoluteFilePath(fileName);
        if( !file.exists() )
            return findPath(fileName);
        else
            return file.absoluteFilePath();
    }// else
    return fileName;
}

void Includes::clear()
{
    d_lock.lockForWrite();
    d_includes.clear();
    d_lock.unlock();
}

