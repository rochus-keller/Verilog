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

#include "VlCrossRefModel.h"
#include "VlProjectConfig.h"
#include "VlProjectFile.h"
#include "VlIncludes.h"
#include "VlFileCache.h"
#include <QFileInfo>
#include <QDir>
#include <QtDebug>
using namespace Vl;

ProjectConfig::ProjectConfig()
{

}

bool ProjectConfig::loadFromFile(const QString& path)
{
    d_srcFiles.clear();
    d_libFiles.clear();
    d_incDirs.clear();
    d_config.clear();

    d_config["SRCEXT"] << ".v"; // Preset
    d_config["LIBEXT"] << ".v";
    d_config["SVEXT"] << ".sv";

    d_path = path;

    Vl::ProjectFile p( d_config );
    if( !p.read(path) )
        return false;

    d_config = p.variables();

    const QString oldCur = QDir::currentPath();
    QDir::setCurrent(QFileInfo(path).path());

    const QStringList incDirs = d_config.value("INCDIRS");
    foreach( const QString& d, incDirs )
    {
        QFileInfo info(d);
        QString path;
        if( info.isRelative() )
            path = QDir::cleanPath( QDir::current().absoluteFilePath(d) );
        else
            path = info.canonicalPath();
        if( !d_incDirs.contains(path) )
            d_incDirs.append(path);
    }

    QStringList filter = d_config["LIBEXT"];
    for( int i = 0; i < filter.size(); i++ )
        filter[i] = "*" + filter[i];

    QSet<QString> files;

    QStringList libFiles = d_config.value("LIBFILES");
    QStringList libDirs = d_config.value("LIBDIRS");

    findFilesInDirs( libDirs, filter, files );
    QDir lastDir = QDir::current();
    foreach( const QString& f, libFiles )
    {
        QFileInfo info(f);
        if( info.isDir() )
        {
            if( info.isRelative() )
                lastDir = QDir::current().absoluteFilePath(f);
            else
                lastDir = info.absoluteDir();
        }else
        {
            if( info.isRelative() )
                files.insert( lastDir.absoluteFilePath(f) );
            else
                files.insert(f);
        }
    }
    libFiles = files.toList();
    libFiles.sort();

    d_libFiles = libFiles;

    filter = d_config["SRCEXT"];
    for( int i = 0; i < filter.size(); i++ )
        filter[i] = "*" + filter[i];

    files.clear();
    QStringList srcFiles = d_config.value("SRCFILES");
    QStringList srcDirs = d_config.value("SRCDIRS");
    if( srcFiles.isEmpty() && ( srcDirs.isEmpty() || srcDirs.first().startsWith('-') ) )
        srcDirs.prepend(".*");

    findFilesInDirs( srcDirs, filter, files );
    lastDir = QDir::current();
    foreach( const QString& f, srcFiles )
    {
        QFileInfo info(f);
        if( info.isDir() )
        {
            if( info.isRelative() )
                lastDir = QDir::current().absoluteFilePath(f);
            else
                lastDir = info.absoluteDir();
        }else
        {
            if( info.isRelative() )
                files.insert( lastDir.absoluteFilePath(f) );
            else
                files.insert(f);
        }
    }
    srcFiles = files.toList();
    srcFiles.sort();

    d_srcFiles = srcFiles;

    QDir::setCurrent(oldCur);
    return true;
}

QString ProjectConfig::getTopMod() const
{
    QStringList topmods = d_config.value("TOPMOD");
    if( !topmods.isEmpty() )
        return topmods.first();
    else
        return QString();
}

QString ProjectConfig::getDefines() const
{
    QStringList defs = d_config.value("DEFINES");
    defs.sort();
    for( int i = 0; i < defs.size(); i++ )
    {
        defs[i] = "`define " + defs[i];
    }
    return defs.join('\n');
}

void ProjectConfig::setup(CrossRefModel* mdl, bool synchronous)
{
    Q_ASSERT( mdl != 0 );
    mdl->parseString( getDefines() , d_path );
    foreach( const QString& inc, d_incDirs )
        mdl->getIncs()->addDir( QDir( inc ) );
    if( mdl->getFcache() )
    {
        mdl->getFcache()->setSvSuffix(d_config["SVEXT"]);
        mdl->getFcache()->setSupportSvExt(d_config["CONFIG"].contains("UseSvExtension") );
    }
    mdl->updateFiles( d_srcFiles + d_libFiles, synchronous );
}

static void filterFiles( QStringList& in, QSet<QString>& out, QSet<QString>& filter )
{
    if( filter.isEmpty() )
    {
        // Trivialfall
        foreach( const QString& s, in )
            out.insert(s);
    }else
    {
        foreach( const QString& s, in )
        {
            if( !filter.contains( QFileInfo(s).fileName() ) )
                out.insert(s);
        }
    }
    in.clear();
    filter.clear();
}

void ProjectConfig::findFilesInDirs(const QStringList& dirs, const QStringList& filter, QSet<QString>& files)
{
    int i = 0;
    QStringList files2;
    QSet<QString> filter2;
    while( i < dirs.size() )
    {
        QString dir = dirs[i];
        if( dir.startsWith('-') )
        {
            // Element ist eine auszulassende Datei
            filter2.insert( dir.mid(1).trimmed() );
        }else
        {
            if( !files2.isEmpty() )
                filterFiles( files2, files, filter2 );
            // Element ist ein zu durchsuchendes Verzeichnis
            bool recursive = false;
            if( dir.endsWith( '*' ) )
            {
                recursive = true;
                dir.chop(1);
            }
            findFilesInDir(dir,filter,files2,recursive);
        }
        i++;
    }
    if( !files2.isEmpty() )
        filterFiles( files2, files, filter2 );
}

void ProjectConfig::findFilesInDir(const QString& dirPath, const QStringList& filter, QStringList& out, bool recursive )
{
    QStringList files;

    QDir dir(dirPath);

    if( recursive )
    {
        files = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );

        foreach( const QString& f, files )
        {
            findFilesInDir( dir.absoluteFilePath(f), filter, out, recursive );
        }
    }

    if( !filter.isEmpty() )
        files = dir.entryList( filter, QDir::Files );
    else
        files.clear();
    for( int i = 0; i < files.size(); i++ )
    {
        out.append( dir.absoluteFilePath(files[i]) );
    }
}

