//
// token.hh - Token class
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
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief Token class used by the Tokenizer
/// @author Daniel Vogelbacher
/// @since 0.1


#ifndef INFORMAVE_ARGON_TOKEN_HH
#define INFORMAVE_ARGON_TOKEN_HH

#include "argon/fwd.hh"


ARGON_NAMESPACE_BEGIN

#define ARGON_DEFAULT_SOURCE_NAME "<input>"


//--------------------------------------------------------------------------
/// Token source information
///
/// @since 0.0.1
/// @brief Provides information about token source
class SourceInfo
{
public:
    SourceInfo(String file = "",
               std::streamsize chpos = 0, size_t len = 0, size_t line = 0);

    /// @brief Update/expand the current info
    void expand(const SourceInfo &info);

    inline std::streamsize offset(void) const
    {
        return this->m_offset;
    }

    inline size_t length(void) const
    {
        return this->m_len;
    }

    inline size_t linenum(void) const
    {
        return this->m_line;
    }

    inline String sourceName(void) const
    {
        return this->m_file;
    }

protected:
    String            m_file;
    std::streamsize   m_offset;
    size_t            m_len;
    size_t            m_line;

};


/// for debugging
inline std::ostream& operator<<(std::ostream &o, const SourceInfo &info)
{
    o << "SRCINFO: " << info.sourceName() << ":" << info.linenum() << " " << info.offset() << ":" << info.length();
    return o;
}



//--------------------------------------------------------------------------
/// Token class
///
/// @since 0.0.1
/// @brief Token class
class Token
{
public:
    /// @brief Constructor for empty Tokens
    Token(void);

    /// @brief Creates a new token with the given ID
    Token(int tid, SourceInfo si = SourceInfo());

    /// @brief Copy constructor
    Token(const Token& t);

    /// @brief Creates a EOF token
    static Token eof(void);

    /// @brief Set the raw data
    void setData(const String &str);

    /// @brief Get the raw data
    String data(void) const;

    /// @brief Get the token ID used by the parser
    int id(void) const;

    /// @brief Get a string representation for the token (used for debugging)
    std::string str(void) const;

    /// @brief returns the source information object
    const SourceInfo& getSourceInfo(void) const;

    /// @brief compare two tokens
    bool operator !=(const Token &tok) const;

protected:
    int         m_id;
    String      m_data;
    SourceInfo  m_info;
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
