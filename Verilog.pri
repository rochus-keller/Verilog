#/*
#* Copyright 2018-2019 Rochus Keller <mailto:me@rochus-keller.ch>
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

INCLUDEPATH +=  ..

SOURCES += \
    ../Verilog/VlPpSymbols.cpp \
    ../Verilog/VlErrors.cpp \
    ../Verilog/VlNumLex.cpp \
    ../Verilog/VlPpLexer.cpp \
    ../Verilog/VlToken.cpp \
    ../Verilog/VlParser.cpp \
    ../Verilog/VlSynTree.cpp \
    ../Verilog/VlIncludes.cpp \
    ../Verilog/VlFileCache.cpp \
    ../Verilog/VlCrossRefModel.cpp \
    ../Verilog/VlProjectFile.cpp \
    ../Verilog/VlProjectConfig.cpp \
    ../Verilog/VlTokenType.cpp

HEADERS  += \
    ../Verilog/VlPpSymbols.h \
    ../Verilog/VlErrors.h \
    ../Verilog/VlNumLex.h \
    ../Verilog/VlPpLexer.h \
    ../Verilog/VlToken.h \
    ../Verilog/VlParser.h \
    ../Verilog/VlSynTree.h \
    ../Verilog/VlIncludes.h \
    ../Verilog/VlFileCache.h \
    ../Verilog/VlCrossRefModel.h \
    ../Verilog/VlProjectFile.h \
    ../Verilog/VlProjectConfig.h\
    ../Verilog/VlTokenType.h

