#/*
#* Copyright 2018 Rochus Keller <mailto:me@rochus-keller.ch>
#*
#* This file is part of the Verilog parser library.
#*
#* The following is the license that applies to this copy of the
#* library. For a license to use the library under conditions
#* other than those described here, please email to me@rochus-keller.ch.
#*
#* GNU General Public License Usage
#* This file may be used under the terms of the GNU General Public
#* License (GPL) versions 2.0 or 3.0 as published by the Free Software
#* Foundation and appearing in the file LICENSE.GPL included in
#* the packaging of this file. Please review the following information
#* to ensure GNU General Public Licensing requirements will be met:
#* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
#* http://www.gnu.org/copyleft/gpl.html.
#*/

QT       += core
QT       -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Verilog
TEMPLATE = app

INCLUDEPATH +=  ..

SOURCES += main.cpp \
    VlFrontend.cpp

include( Verilog.pri )

CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
}

#CONFIG += c++11

#include( ../../Libraries/antlr4/antlr4.pri )

HEADERS += \
    VlFrontend.h


