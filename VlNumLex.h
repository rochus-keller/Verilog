#ifndef VLNUMLEX_H
#define VLNUMLEX_H

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

#include <QString>
#include <Verilog/VlToken.h>

namespace Vl
{
    class NumLex
    {
    public:
        enum Kind { Unknown, Real, Decimal, Octal, Binary, Hex };

        NumLex( const QByteArray&, int start = 0 );
        const QString& getError() const { return d_error; }
        bool parse(bool fullValidation = false);
        int getOff() const { return d_off; }
        Kind getKind() const { return (Kind)d_kind; }
        const char* getKindName() const;
        const QByteArray& getVal() const { return d_val; }
        const QByteArray& getSize() const { return d_size; }
        bool getSigned() const { return d_signed; }
        bool hasSize() const { return d_hasSize; }
        bool hasBaseFormat() const { return d_hasBaseFormat; }
        bool hasValue() const { return d_hasValue; }
        TokenType getTokenType() const;
    protected:
        char lookAhead( quint32 ) const;
        bool baseFormat();
        void skipWhiteSpace();
        bool basedValue();
        bool extendedDigit(char);
        bool unsignedNumber(bool allowHex = false);
        bool real();
        bool error( const QString& );
    private:
        QString d_error;
        const QByteArray d_str;
        QByteArray d_val;
        QByteArray d_size;
        const int d_start;
        int d_off;
        quint8 d_kind;
        bool d_signed;
        bool d_hasSize;
        bool d_hasBaseFormat;
        bool d_hasValue;
        bool d_fullValidation;
    };
}

#endif // VLNUMLEX_H
