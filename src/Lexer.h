#pragma once

#include "Token.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#define READ_RETURN(ret) { data.Read(); return ret;}

struct LexerData
{
  private:
    std::istream& stream;
    char c;
    size_t lineNr;
    size_t columnNr;

  public:
    LexerData(std::istream& stream)
      : stream{stream}, c{'\0'}, lineNr{1}, columnNr{1}
    {
      stream >> c;
    }

    char Read()
    {
      stream.get(c);
      columnNr++;
      if(c == '\n')
      {
        lineNr++;
        columnNr = 0;
      }
      return c;
    }

    char Top()
    {
      return c;
    }

    bool Empty()
    {
      return stream.eof();
    }

    size_t LineNr()
    {
      return lineNr;
    }
    size_t ColumnNr()
    {
      return columnNr;
    }
};
class Lexer
{
  public:
    static std::vector<TokenPos> Read(std::istream& source)
    {
      std::vector<TokenPos> tokens;
      LexerData data{source};
      TokenPos t;
      while(!data.Empty() && ((t = ReadToken(data)).token != Token::INVALID))
      {
        tokens.push_back(t);
        ReadWhiteSpace(data);
      }
      if(t.token == Token::INVALID)
        return {t};

      return tokens;
    }


  private:
    static void ReadWhiteSpace(LexerData& data)
    {
      while(!data.Empty() && (data.Top() == ' ' || data.Top() == '\n' || data.Top() == '\r' || data.Top() == '\t'))
        data.Read();
    }

    static TokenPos ReadToken(LexerData& data)
    {
      size_t line = data.LineNr();
      size_t column = data.ColumnNr();
      if(IsName(data.Top()))
      {
        std::string str = ReadName(data);
        Token reservedToken = Tokens::GetReservedToken(str);
        if(reservedToken == Token::INVALID)
        {
          return {Token::NAME, line, column};
        }
        else
        {
          return {reservedToken, line, column};
        }
      }
      else if(IsNumber(data.Top()))
      {
        ReadNumber(data);
        return {Token::NUMBER, line, column};
      }
      else if(IsString(data.Top()))
      {
        ReadString(data);
        return {Token::STRING, line, column};
      }
      else if(IsChar(data.Top()))
      {
        char i = ReadChar(data);
        return {Token::CHAR, line, column};
      }
      else
      {
        return {ReadSymbol(data), line, column};
      }
      if(!data.Empty())
      {
        std::cerr << "Invalid token at: " << line << ":" << column << std::endl;
      }
      return {Token::INVALID, line, column};
    }

    static bool IsLetter(char c)
    {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool IsNumber(char c)
    {
      return c >= '0' && c <= '9';
    }

    static bool IsName(char c)
    {
      return IsLetter(c) || c == '_';
    }

    static bool IsString(char c)
    {
      return c == '"';
    }

    static bool IsChar(char c)
    {
      return c == '\'';
    }

    static bool IsEscapeCharacter(char c)
    {
      return c == 'n' || c == 'r' || c == 't' || c == '\\' || c == '"' || c == '\'' || c == '0';
    }

    static bool GetEscapeCharacter(char c)
    {
      if(c == 'n') return '\n';
      if(c == 'r') return '\r';
      if(c == 't') return '\t';
      if(c == '\\') return '\\';
      if(c == '"') return '"';
      if(c == '\'') return '\'';
      if(c == '0') return '\0';
      abort();
      return '\0';
    }

    static std::string ReadString(LexerData& data)
    {
      std::stringstream ss;
      data.Read();
      char lastchar = data.Top();
      while(!data.Empty() && !(data.Top() == '"' && lastchar != '\\'))
      {
        ss << data.Top();
        lastchar = data.Top();
        data.Read();
      }
      data.Read();
      return ss.str();
    }

    static char ReadChar(LexerData& data)
    {
      char ret = '\0';
      std::stringstream ss;
      ret = data.Read();
      if(data.Top() == '\\')
      {
        data.Read();
        if(!IsEscapeCharacter(data.Top()))
        {
          std::cerr << "Invalid escape character: " << data.Top() << std::endl;
          return '\0';
        }
        ret = GetEscapeCharacter(data.Top());
        data.Read();
      }
      else
      {
        if(data.Top() == '\'')
        {
          std::cerr << "No character specified within char" << std::endl;
          return '\0';
        }
        data.Read();
      }
      if(data.Top() != '\'')
      {
        std::cerr << "More than 1 character within single quote" << std::endl;
        return '\0';
      }
      data.Read();
      return ret;
    }

    static Token ReadSymbol(LexerData& data)
    {
      if(data.Top() == '(') READ_RETURN(Token::OPEN_PARAM);
      if(data.Top() == ')') READ_RETURN(Token::CLOSE_PARAM);
      if(data.Top() == '[') READ_RETURN(Token::OPEN_SQUARE);
      if(data.Top() == ']') READ_RETURN(Token::CLOSE_SQUARE);
      if(data.Top() == '{') READ_RETURN(Token::OPEN_CURLY);
      if(data.Top() == '}') READ_RETURN(Token::CLOSE_CURLY);
      if(data.Top() == '=') READ_RETURN(ReadSymbolSuffix(data, '=', Token::ASSIGN, Token::EQUAL));
      if(data.Top() == '+') READ_RETURN(Token::ADD);
      if(data.Top() == '-') READ_RETURN(Token::SUB);
      if(data.Top() == '*') READ_RETURN(Token::MUL);
      if(data.Top() == '/') READ_RETURN(Token::DIV);
      if(data.Top() == '!') READ_RETURN(ReadSymbolSuffix(data, '=', Token::NOT, Token::NEQUAL));
      if(data.Top() == '&') READ_RETURN(ReadSymbolSuffix(data, '&', Token::BIN_AND, Token::AND));
      if(data.Top() == '|') READ_RETURN(ReadSymbolSuffix(data, '|', Token::BIN_OR, Token::OR));
      if(data.Top() == '^') READ_RETURN(Token::BIN_XOR);
      if(data.Top() == '~') READ_RETURN(Token::BIN_NOT);
      if(data.Top() == '<') READ_RETURN(ReadSymbolSuffix(data, '=', Token::LT, Token::LTE));
      if(data.Top() == '>') READ_RETURN(ReadSymbolSuffix(data, '=', Token::GT, Token::GTE));;
      if(data.Top() == '#') READ_RETURN(Token::HASH);
      if(data.Top() == '.') READ_RETURN(Token::DOT);
      if(data.Top() == ',') READ_RETURN(Token::COMMA);
      if(data.Top() == ':') READ_RETURN(Token::COLON);
      if(data.Top() == ';') READ_RETURN(Token::SEMICOLON);

      return Token::INVALID;
    }

    // Returns different Tokens depending on if the suffix exists behind char.
    static Token ReadSymbolSuffix(LexerData& data, char suffix, Token noSuffix, Token withSuffix)
    {
      if(data.Top() == suffix) READ_RETURN(withSuffix);
      return noSuffix;
    }

    static std::string ReadName(LexerData& data)
    {
      std::stringstream ss;
      while(IsLetter(data.Top()) || IsNumber(data.Top()) || data.Top() == '_')
      {
        ss << data.Top();
        data.Read();
      }

      return ss.str();
    }

    static std::string ReadNumber(LexerData& data)
    {
      std::stringstream ss;
      bool hasReadComma = false;
      while(IsNumber(data.Top()) || (data.Top() == '.' && !hasReadComma))
      {
        if(data.Top() == '.')
          hasReadComma = true;
        ss << data.Top();
        data.Read();
      }

      return ss.str();
    }
};
