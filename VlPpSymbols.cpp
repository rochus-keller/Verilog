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

#include "VlPpSymbols.h"
#include <QtDebug>
using namespace Vl;

PpSymbols::PpSymbols(QObject *parent) : QObject(parent)
{

}

void PpSymbols::addSymbol(const QByteArray& name, const TokenList& val, const QByteArrayList& args)
{
    Define d;
    d.d_name = name;
    d.d_toks = val;
    d.d_args = args;
    addSymbol(d);
}

static QDebug& operator<<(QDebug& dbg, const Vl::Token& t)
{
    dbg << "Tok(" << t.getName() << " " << t.d_val << ")";
    return dbg;
}

void PpSymbols::addSymbol(const PpSymbols::Define& def)
{
    d_lock.lockForWrite();
    //qDebug() << "Symbol added" << def.d_name << def.d_args << def.d_toks;
    d_defs[def.d_name] = def;
    d_lock.unlock();
}

void PpSymbols::addSymbol(const QByteArray& name, const QByteArray& val, TokenType type)
{
    Define d;
    d.d_name = name;
    d.d_toks << Token(type, 0,0,0, val );
    addSymbol(d);
}

const PpSymbols::Define PpSymbols::getSymbol(const QByteArray& id)
{
    d_lock.lockForRead();
    const Define d = d_defs.value(id);
    d_lock.unlock();
    return d;
}

bool PpSymbols::contains(const QByteArray& id) const
{
    d_lock.lockForRead();
    const bool res = d_defs.contains(id);
    d_lock.unlock();
    return res;
}

void PpSymbols::remove(const QByteArray& id)
{
    d_lock.lockForWrite();
    d_defs.remove(id);
    d_lock.unlock();
}

void PpSymbols::clear()
{
    d_lock.lockForWrite();
    d_defs.clear();
    d_lock.unlock();
}

