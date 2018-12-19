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

#include "VlFileCache.h"
#include <QFile>
#include <QBuffer>
#include <QFileInfo>
using namespace Vl;

FileCache::FileCache(QObject *parent) : QObject(parent)
{

}

// TODO: brauchen wir hier canonicalPaths? Löst PpLexer::processInclude hinreichend gut auf?
#define _USE_CANONOCALS

void FileCache::addFile(const QString& path, const QByteArray& content)
{
#ifdef _USE_CANONOCALS
    const QString cpath = QFileInfo(path).canonicalFilePath();
#else
    const QString cpath = path;
#endif
    d_lock.lockForWrite();
    d_files[cpath] = content;
    d_lock.unlock();
}

void FileCache::removeFile(const QString& path)
{
#ifdef _USE_CANONOCALS
    const QString cpath = QFileInfo(path).canonicalFilePath();
#else
    const QString cpath = path;
#endif
    d_lock.lockForWrite();
    d_files.remove(cpath);
    d_lock.unlock();
}

QByteArray FileCache::getFile(const QString& path, bool* found) const
{
    QByteArray res;
#ifdef _USE_CANONOCALS
    const QString cpath = QFileInfo(path).canonicalFilePath();
#else
    const QString cpath = path;
#endif

    d_lock.lockForRead();

    if( found )
        *found = false;
    Files::const_iterator i = d_files.find(cpath);
    if( i != d_files.end() )
    {
        res = i.value();
        if( found )
            *found = true;
    }

    d_lock.unlock();

    return res;
}

QByteArray FileCache::fetchTextLineFromFile(const QString& path, int line, const QByteArray& defaultString)
{
    QIODevice* in = createFileStreamForReading( path );
    if( in == 0 )
        return defaultString;
    QByteArray str;
    for( int i = 0; i < line; i++ )
    {
        if( in->atEnd() )
        {
            delete in;
            return defaultString;
        }
        str = in->readLine();
    }
    delete in;
    str.chop(1);
    if( str.endsWith('\r') )
        str.chop(1);
    return str;
}

QIODevice*FileCache::createFileStreamForReading(const QString& path) const
{
    bool found;
    QByteArray bytes = getFile( path, &found );
    if( found )
    {
        QBuffer* buf = new QBuffer();
        buf->setData(bytes);
        buf->open(QIODevice::ReadOnly);
        return buf;
    }else if( QFileInfo(path).isReadable() )
    {
        QFile* file = new QFile( path );
        if( !file->open(QIODevice::ReadOnly) )
        {
            delete file;
            return 0;
        }else
            return file;
    }
    return 0;
}

