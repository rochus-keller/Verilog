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

#include "VlNumLex.h"
#include "VlPpLexer.h" // wegen tr()
using namespace Vl;

NumLex::NumLex(const QByteArray& str, int start):
    d_str(str),d_start(start),d_off(start),d_kind(Unknown),d_signed(false),
    d_hasSize(false),d_hasBaseFormat(false),d_hasValue(false),d_fullValidation(false)
{
}

static bool inline checkOnlyDigits( const QByteArray& str )
{
    for( int i = 0; i < str.size(); i++ )
    {
        if( !::isdigit( str[i] ) && str[i] != '_' )
            return false;
    }
    return true;
}

bool NumLex::parse(bool fullValidation)
{
    d_off = 0;
    d_size.clear();
    d_val.clear();
    d_fullValidation = fullValidation;

    if( lookAhead( d_off ) == '\'' )
    {
        if( !baseFormat() )
            return false;
        skipWhiteSpace();
        return basedValue();
    }else
    {
        // NOTE: bei alleinstehenden Tok_Base_Value könnten laut Norm auch Hexdigits kommen.
        // Wir schränken hier vorerst willkürlich ein, dass als erstes in jedem Fall eine Zahl kommen muss,
        // also im Falle von Hex eine vorstehende 0.

        if( !unsignedNumber(true) )
            return false;

        char ch = lookAhead( d_off );
        if( ch == '.' || ch == 'e' || ch == 'E' )
        {
            return real();
        }

        //const int oldOff = d_off;
        skipWhiteSpace();

        ch = lookAhead( d_off );
        if( ch == '\'' )
        {
            // erste Zahl war Size
            d_size = d_val;
            d_hasSize = true;
            d_val.clear();
            if( !checkOnlyDigits( d_size) )
                return error( QLatin1String("size cannot include other than decimal digits") );
            Q_ASSERT( !d_size.isEmpty() );
            if( d_size[0] == '0' )
                return error( QLatin1String("size cannot start with a '0' digit") );
            if( !baseFormat() )
                return false;
            skipWhiteSpace();
            return basedValue();
        }else
        {
            d_kind = Decimal;

            if( d_fullValidation && !checkOnlyDigits(d_val) )
                return error( QLatin1String("decimal number cannot include other than decimal digits") );
//            ch = lookAhead( oldOff );
//            if( ch != 0 && !::isspace(ch) )
//                return error( Lexer::tr("invalid number") );
            return true;
        }
    }
}

const char*NumLex::getKindName() const
{
    switch(d_kind)
    {
    case Unknown:
        return "?";
    case Real:
        return "Real";
    case Decimal:
        return "Decimal";
    case Octal:
        return "Octal";
    case Binary:
        return "Binary";
    case Hex:
        return "Hex";
    }
    return "";
}

TokenType NumLex::getTokenType() const
{
    if( d_kind == Unknown )
        return Tok_Invalid;
    else if( d_kind == Real )
        return Tok_Realnum;
    else if( d_hasSize && d_hasBaseFormat && d_hasValue )
        return Tok_SizedBased;
    else if( d_hasBaseFormat && d_hasValue )
        return Tok_BasedInt;
    else if( d_hasBaseFormat )
        return Tok_BaseFormat;
    else if( d_hasValue )
        return Tok_BaseValue;
    else
        return Tok_Natural;
}

bool NumLex::error(const QString& str)
{
    d_error = str;
    return false;
}

char NumLex::lookAhead(quint32 off) const
{
    if( int( d_start + off ) < d_str.size() )
        return d_str[ d_start + off ];
    else
        return 0;
}

bool NumLex::baseFormat()
{
    // '[s|S]d | '[s|S]D
    // '[s|S]b | '[s|S]B
    // '[s|S]o | '[s|S]O
    // '[s|S]h | '[s|S]H
    Q_ASSERT( lookAhead(d_off) == '\'');
    d_off++;
    char ch = lookAhead(d_off);
    if( ch == 's' || ch == 'S' )
    {
        d_off++;
        d_signed = true;
    }

    switch( lookAhead(d_off) )
    {
    case 'd':
    case 'D':
        d_kind = Decimal;
        break;
    case 'b':
    case 'B':
        d_kind = Binary;
        break;
    case 'o':
    case 'O':
        d_kind = Octal;
        break;
    case 'h':
    case 'H':
        d_kind = Hex;
        break;
    default:
        return error( PpLexer::tr("invalid base format") );
    }
    d_off++;
    d_hasBaseFormat = true;
    return true;
}

void NumLex::skipWhiteSpace()
{
    while( ::isspace( lookAhead(d_off ) ) )
        d_off++;
}

static bool hasXorZdigits( const QByteArray& str )
{
    for( int i = 0; i < str.size(); i++ )
    {
        const char ch = str[i];
        if( ch == 'x' || ch == 'X' || ch == 'z' || ch == 'Z' || ch == '?' )
            return true;
    }
    return false;
}

bool NumLex::basedValue()
{
    QByteArray val;
    char ch = lookAhead( d_off );
    if( !extendedDigit(ch) )
    {
        if( d_fullValidation )
            return error( Vl::PpLexer::tr("invalid digit for given base '%1'").arg(ch) );
        else
            return true;
    }
    d_off++;
    val.append(ch);
    ch = lookAhead( d_off );
    while( true )
    {
        if( ch == '_' )
        {
            d_off++;
        }else if( extendedDigit(ch) )
        {
            d_off++;
            val.append(ch);
        }else if( d_fullValidation )
        {
            if( ch == 0 || ::isspace(ch) )
                break;
            else
                return error( Vl::PpLexer::tr("invalid digit for given base '%1'").arg(ch) );
        }else
            break;
        ch = lookAhead( d_off );
    }
    if( d_fullValidation && d_kind == Decimal && hasXorZdigits(val) &&
            !( val.size() == 1 || ( val.size() == 2 && val[1] == '_' ) ) )
        return error( QLatin1String("invalid based decimal value") );
    d_val = val;
    d_hasValue = !val.isEmpty();
    return true;
}

bool NumLex::extendedDigit(char ch)
{
    switch( ch )
    {
    case '0':
    case '1':
        return !d_fullValidation || d_kind == Octal || d_kind == Hex || d_kind == Decimal || d_kind == Binary;
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        return !d_fullValidation || d_kind == Octal || d_kind == Hex || d_kind == Decimal;
    case '8':
    case '9':
        return !d_fullValidation || d_kind == Hex || d_kind == Decimal;
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'c':
    case 'C':
    case 'd':
    case 'D':
    case 'e':
    case 'E':
    case 'f':
    case 'F':
        return !d_fullValidation || d_kind == Hex;
    case 'x':
    case 'X':
    case 'z':
    case 'Z':
    case '?':
        return !d_fullValidation || d_kind == Octal || d_kind == Hex || d_kind == Binary
                || d_kind == Decimal // aber nur x_digit { _ } oder z_digit { _ }
                ;
    default:
        return false;
    }
}

bool NumLex::unsignedNumber(bool allowHex)
{
    // Hierher kommen wir nur, wenn der Lexer ein Digit entdeckt hat
    QByteArray val;
    char ch = lookAhead( d_off );
    if( !::isdigit( ch ) )
        return error( PpLexer::tr("expecting digit") );

    d_off++;
    val.append(ch);
    ch = lookAhead( d_off );
    while( true )
    {
        if( ch == '_' )
        {
            d_off++;
        }else if( ::isdigit( ch ) || ( allowHex && ::isxdigit( ch ) ) )
        {
            d_off++;
            val.append(ch);
        }else
            break;
        ch = lookAhead( d_off );
    }
    d_val += val;
    return true;
}

bool NumLex::real()
{
    char ch = lookAhead( d_off );
    if( !checkOnlyDigits( d_val ) )
        return error( QString("number includes other than decimal digits: %1").arg(d_val.data()) );
    if( ch == '.' )
    {
        d_val += ch;
        d_kind = Real;
        d_off++;
        if( !unsignedNumber() )
            return false;
        ch = lookAhead( d_off );
        if( ch == 'e' || ch == 'E' )
        {
            d_val += 'e';
            d_off++;
            char ch = lookAhead( d_off );
            if( ch == '+' || ch == '-' )
            {
                if( ch == '-' )
                    d_val += ch;
                d_off++;
                if( !unsignedNumber() )
                    return false;
            }else if( !unsignedNumber() )
                return false;
        }
//        ch = lookAhead(d_off);
//        if( ch != 0 && !::isspace(ch) )
//            return error( Lexer::tr("invalid number") );
    }else if( ch == 'e' || ch == 'E' )
    {
        d_val += 'e';
        d_off++;
        d_kind = Real;
        ch = lookAhead( d_off );
        if( ch == '+' || ch == '-' )
        {
            if( ch == '-' )
                d_val += ch;
            d_off++;
            if( !unsignedNumber() )
                return false;
        }else if( !unsignedNumber() )
            return false;
//        ch = lookAhead(d_off);
//        if( ch != 0 && !::isspace(ch) )
//            return error( Lexer::tr("invalid number") );
    }else
        Q_ASSERT(false);
    return true;
}

