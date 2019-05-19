#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

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

#include <QString>
#include <QStringList>
#include <QMap>

namespace Vl
{
    class CrossRefModel;

    class ProjectConfig
    {
    public:
        ProjectConfig();

        bool loadFromFile( const QString& path );
        const QStringList& getSrcFiles() const { return d_srcFiles; }
        const QStringList& getLibFiles() const { return d_libFiles; }
        const QStringList& getOtherFiles() const { return d_otherFiles; }
        QStringList getConfig( const QString& key ) const { return d_config.value(key); }
        QStringList getIncDirs() const { return d_incDirs; }
        QString getTopMod() const;
        QString getDefines() const;
        const QString& getPath() const { return d_path; }

        void setup( CrossRefModel*, bool synchronous = false );
    protected:
        static void findFilesInDirs(const QStringList& dirs, const QStringList& filter, QSet<QString>& files);
        static void findFilesInDir(const QString& dir, const QStringList& filter, QStringList& files , bool recursive);
    private:
        QString d_path;
        QStringList d_srcFiles, d_libFiles, d_incDirs, d_otherFiles;
        QMap<QString, QStringList> d_config;
    };
}

#endif // PROJECTCONFIG_H
