//
// tokenizer.hh - Read a stream and produce tokens
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief Tokenizer
/// @author Daniel Vogelbacher
/// @since 0.1

#ifndef INFORMAVE_ARGON_TOKENIZER_HH
#define INFORMAVE_ARGON_TOKENIZER_HH


#include "argon/token.hh"

#include <iterator>
#include <vector>
#include <cctype>
#include <locale>
#include <cassert>
#include <algorithm>

#include <iostream>

ARGON_NAMESPACE_BEGIN




template <typename CharT, typename TraitsT, size_t N>
std::basic_string<CharT, TraitsT>
inline _str(const char (&src)[N])
{
    return std::basic_string<CharT, TraitsT>(src, src+N-1);
}



template <typename CharT, typename TraitsT>
std::basic_string<CharT, TraitsT>
inline upper(const std::basic_string<CharT, TraitsT> &str)
{
    std::basic_string<CharT, TraitsT> out;
    out.resize(str.length());
    std::locale loc;
    const std::ctype<char>& ct = std::use_facet<std::ctype<char> >(loc);
    std::transform(str.begin(), str.end(), out.begin(), std::bind1st(std::mem_fun(&std::ctype<char>::toupper), &ct));
    return out;
}




//--------------------------------------------------------------------------
/// Tokenizer
///
/// @since 0.0.1
/// @brief Tokenizer
template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class Tokenizer
{
public:
    typedef std::istreambuf_iterator<CharT, TraitsT>             streambuf_iterator;
    typedef std::map< std::basic_string<CharT, TraitsT>, int >   map_type;
    typedef CharT                                                char_type;
    typedef TraitsT                                              traits_type;


    /// Create new Tokenizer from input stream iterator
    Tokenizer(streambuf_iterator in)
        : m_in(in),
          m_line(1),
          m_charpos(0),
          m_srcname("<input>")
    {
        /// We need to consume the first character for initial state.
        consume();


        /// Add your keywords here
        this->m_keywords[ _str<CharT, TraitsT>("CONNECTION") ] = ARGON_TOK_CONNECTION;
        this->m_keywords[ _str<CharT, TraitsT>("TYPE")       ] = ARGON_TOK_TYPE;
        this->m_keywords[ _str<CharT, TraitsT>("DBCSTR")     ] = ARGON_TOK_DBCSTR;
        this->m_keywords[ _str<CharT, TraitsT>("PROGRAM.")   ] = ARGON_TOK_PROGRAM;
        this->m_keywords[ _str<CharT, TraitsT>("TASK")       ] = ARGON_TOK_TASK;
        this->m_keywords[ _str<CharT, TraitsT>("AS")         ] = ARGON_TOK_AS;
        this->m_keywords[ _str<CharT, TraitsT>("BEGIN")      ] = ARGON_TOK_BEGIN;
        this->m_keywords[ _str<CharT, TraitsT>("END")        ] = ARGON_TOK_END;

        this->m_keywords[ _str<CharT, TraitsT>("DECLARE")     ] = ARGON_TOK_DECLARE;
        this->m_keywords[ _str<CharT, TraitsT>("TABLE")       ] = ARGON_TOK_TABLE;
        this->m_keywords[ _str<CharT, TraitsT>("VIEW")        ] = ARGON_TOK_VIEW;
        this->m_keywords[ _str<CharT, TraitsT>("PROCEDURE")   ] = ARGON_TOK_PROCEDURE;
        this->m_keywords[ _str<CharT, TraitsT>("SQL")         ] = ARGON_TOK_SQL;

        this->m_keywords[ _str<CharT, TraitsT>("LOG")         ] = ARGON_TOK_LOG;
        this->m_keywords[ _str<CharT, TraitsT>("EXEC")        ] = ARGON_TOK_EXEC;

        /// Additional map with names
        this->m_templates[ _str<CharT, TraitsT>("VOID")         ] = ARGON_TOK_TEMPLATE;
        this->m_templates[ _str<CharT, TraitsT>("FETCH")        ] = ARGON_TOK_TEMPLATE;
        this->m_templates[ _str<CharT, TraitsT>("STORE")        ] = ARGON_TOK_TEMPLATE;
        this->m_templates[ _str<CharT, TraitsT>("TRANSFER")     ] = ARGON_TOK_TEMPLATE;
    }



    /// Consume next character
    /// Returns false on EOF, otherwise true
    bool consume(void)
    {
        if(this->m_in != streambuf_iterator())
        {
            this->m_charpos++;
            this->m_char = *this->m_in++;
            return true;
        }
        this->m_char = traits_type::eof();
        return false;
    }



    /// Consume next character and return it
    inline char_type getnc(void)
    {
        consume();
        return this->m_char;
    }



    /// Skip line breaks and returns the next character
    /// or EOF.
    char_type skiplb(void)
    {
        /// Handle multiple linebreaks
        while(this->m_char == 0x0A || this->m_char == 0x0D)
        {
            char_type prev = this->m_char;
            this->m_line++;
            if(! consume())
                return traits_type::eof();
             
            /// Skip 2-Byte linebreaks on Windows and MAC
            if((m_char == 0x0A && prev == 0x0D) || (m_char == 0x0D && prev == 0x0A))
                if(! consume())
                    return traits_type::eof();
        }
        return this->m_char;
    }


    
    /// Read input and return the next Token
    Token next(void)
    {
        char_type c;

        skiplb();

        // skip whitespace and linebreaks
        for(c = this->m_char;
            (traits_type::to_int_type(c) != traits_type::eof() && (c == ' ' || c == '\t'));
            consume(), c = skiplb()); 

            
        /// Start position of current char
        std::streamsize start = this->m_charpos;
        size_t line = this->m_line;
        size_t len = 0;
                  
        SourceInfo si(m_srcname, start, line, len);

        if(traits_type::to_int_type(c) == traits_type::eof())
            return Token(0);
            
        switch(c)
        {
        case '"':
            return this->readLiteral(start, len, line);
        case ';':
            consume();
            return Token(ARGON_TOK_SEP, si);
        case '(':
            consume();
            return Token(ARGON_TOK_LP, si);
        case ')':
            consume();
            return Token(ARGON_TOK_RP, si);
        case '[':
            consume();
            return Token(ARGON_TOK_LB, si);
        case ']':
            consume();
            return Token(ARGON_TOK_RB, si);
        case ',':
            consume();
            return Token(ARGON_TOK_COMMA, si);
        case '=':
            consume();
            return Token(ARGON_TOK_ASSIGNOP, si);

        case '<':
            consume();
            assert(m_char == '-');
            consume();
            return Token(ARGON_TOK_ASSIGNOP, si);


        case '$':
            return this->readColumn(start, len, line);


        case '/':
            consume();
            if(m_char == '/' || m_char == '*')
            {
                this->skipComment();
                return this->next();
            }
            else
                assert(!"/ err token");
            //return ARGON_TOK_DIV;

            // case '%':
            //     return Token(ARGON_TOK_TERM, start, len, line);
        default:
            return this->readMulti(start, len, line);
        };

    }



    /// Returns true if EOF is reached
    bool eof(void) const
    {
        return !(this->m_in != streambuf_iterator());
    }



    /// Skip comment
    ///
    /// @todo
    /// Not skip, create a new token which can skipped by the parser
    void skipComment(void)
    {
        char_type c;

        // check second char
        assert(m_char == '/' || m_char == '*');

        if(m_char == '/')
        {
              
            for(c = m_char;
                traits_type::to_int_type(c) != traits_type::eof();
                c = getnc())
            {
                if(! isprint(c))
                    break;
            };
        }
        else if(m_char == '*')
        {
            char_type back;
            for(c = m_char, back = m_char;
                traits_type::to_int_type(c) != traits_type::eof();
                c = getnc())
            {
                if(back == '*' && c == '/')
                {
                    consume(); // skip
                    break;
                }
                else
                {
                    // skip linebreaks and return next "valid" character
                    back = skiplb();
                    //back = c;
                }
            };                
        }
    }

    Token readColumn(std::streamsize start, size_t len, size_t line)
    {
        std::vector<char_type> v;
            
        char_type c;
        bool par = false;

        c = getnc();
        if(c == '(')
            par = true;
                
            
        for(;
            traits_type::to_int_type(c) != traits_type::eof();
            c = getnc())
        {
            if((par && c != ')') || isalnum(c))
                v.push_back(c);
            else
                break;
        };
        if(par && c == ')')
            consume(); // skip )
        std::basic_string<char_type> s(v.begin(), v.end());
        Token tok(ARGON_TOK_COLUMN, SourceInfo(m_srcname, start, len, line));
        tok.setData(s);
        return tok;
    }



    /// Read literal
    ///
    /// @todo
    /// Support for escaping
    Token readLiteral(std::streamsize start, size_t len, size_t line)
    {
        std::vector<char_type> v;
        char_type c;

        for(c = getnc();
            traits_type::to_int_type(c) != traits_type::eof();
            c = getnc())
        {
            if(c != '"')
                v.push_back(c);
            else
                break;
        };
        if(c == '"')
            consume(); // skip "
        std::basic_string<char_type> s(v.begin(), v.end());
        Token tok(ARGON_TOK_LITERAL, SourceInfo(m_srcname, start, len, line));
        tok.setData(String(s));
        return tok;
    }


    Token readMulti(std::streamsize start, size_t len, size_t line)
    {
        std::vector<char_type> v;
        v.push_back(this->m_char);
            
        char_type c;

        for(c = getnc();
            traits_type::to_int_type(c) != traits_type::eof();
            c = getnc())
        {
            if(::isalnum(c) || c == '.')
                v.push_back(c);
            else
                break;
        };
        std::basic_string<char_type> s(v.begin(), v.end());

        std::basic_string<char_type> up(upper(s));

        {
            typename map_type::iterator i = this->m_keywords.find(up);
            if(i != this->m_keywords.end())
            {
                Token tok(i->second, SourceInfo(m_srcname, start, len, line));
                tok.setData(up);
                return tok;
            }
        }

        {
            typename map_type::iterator i = this->m_templates.find(up);
            if(i != this->m_templates.end())
            {
                Token tok(i->second, SourceInfo(m_srcname, start, len, line));
                tok.setData(up);
                return tok;
            }
        }

        if(isdigit(s[0]))
        {
            Token tok(ARGON_TOK_NUMBER, SourceInfo(m_srcname, start, len, line));
            /// @bug add number value
            return tok;
        }
            
        Token tok(ARGON_TOK_ID, SourceInfo(m_srcname, start, len, line));
        String x(s);
        tok.setData(x);
        return tok;
    }


    void setSourceName(const String &str)
    {
        this->m_srcname = str;
    }

protected:
    streambuf_iterator  m_in;
    char_type           m_char;
    map_type            m_keywords;
    map_type            m_templates;
    size_t              m_line;
    std::streamsize     m_charpos;
    String      m_srcname;
};



ARGON_NAMESPACE_END

#endif


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
