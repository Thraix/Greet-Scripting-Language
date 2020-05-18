#pragma once

#include "Token.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#define READ_RETURN(ret) { source >> c; return ret;}

class Lexer
{
  public:
    static std::vector<Token> Read(std::istream& source)
    {
      std::vector<Token> tokens;
      char c;
      source >> std::noskipws;
      source >> c;
      Token t;
      while(!source.eof() && ((t = ReadToken(source, c)) != Token::INVALID))
      {
        tokens.push_back(t);
        ReadWhiteSpace(source, c);
      }
      if(t == Token::INVALID)
        return {Token::INVALID};

      return tokens;
    }


  private:
    static void ReadWhiteSpace(std::istream& source, char& c)
    {
      while(!source.eof() && (c == ' ' || c == '\n' || c == '\r' || c == '\t'))
        source >> c;
    }

    static Token ReadToken(std::istream& source, char& c)
    {
      if(IsName(c))
      {
        std::string str = ReadName(source, c);
        Token reservedToken = Tokens::GetReservedToken(str);
        if(reservedToken == Token::INVALID)
        {
          return Token::NAME;
        }
        else
        {
          return reservedToken;
        }
      }
      else if(IsNumber(c))
      {
        ReadNumber(source, c);
        return Token::NUMBER;
      }
      else if(IsString(c))
      {
        ReadString(source, c);
        return Token::STRING;
      }
      else if(IsChar(c))
      {
        char i = ReadChar(source, c);
        return Token::CHAR;
      }
      else
      {
        return ReadSymbol(source, c);
      }
      if(!source.eof())
      {
        std::string s;
        std::getline(source, s);
        std::cerr << "Invalid token at: " << s << std::endl;
      }
      return Token::INVALID;
    }

    static bool IsLetter(char& c)
    {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool IsNumber(char& c)
    {
      return c >= '0' && c <= '9';
    }

    static bool IsName(char& c)
    {
      return IsLetter(c) || c == '_';
    }

    static bool IsString(char& c)
    {
      return c == '"';
    }

    static bool IsChar(char& c)
    {
      return c == '\'';
    }

    static bool IsEscapeCharacter(char& c)
    {
      return c == 'n' || c == 'r' || c == 't' || c == '\\' || c == '"' || c == '\'' || c == '0';
    }

    static bool GetEscapeCharacter(char& c)
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

    static std::string ReadString(std::istream& source, char& c)
    {
      std::stringstream ss;
      source >> c;
      char lastchar = c;
      while(!source.eof() && !(c == '"' && lastchar != '\\'))
      {
        ss << c;
        lastchar = c;
        source >> c;
      }
      source >> c;
      return ss.str();
    }

    static char ReadChar(std::istream& source, char& c)
    {
      char ret = '\0';
      std::stringstream ss;
      source >> c;
      ret = c;
      if(c == '\\')
      {
        source >> c;
        if(!IsEscapeCharacter(c))
        {
          std::cerr << "Invalid escape character: " << c << std::endl;
          return '\0';
        }
        ret = GetEscapeCharacter(c);
        source >> c;
      }
      else
      {
        if(c == '\'')
        {
          std::cerr << "No character specified within char" << std::endl;
          return '\0';
        }
        source >> c;
      }
      if(c != '\'')
      {
        std::cerr << "More than 1 character within single quote" << std::endl;
        return '\0';
      }
      source >> c;
      return ret;
    }

    static Token ReadSymbol(std::istream& source, char& c)
    {
      if(c == '(') READ_RETURN(Token::OPEN_PARAM);
      if(c == ')') READ_RETURN(Token::CLOSE_PARAM);
      if(c == '[') READ_RETURN(Token::OPEN_SQUARE);
      if(c == ']') READ_RETURN(Token::CLOSE_SQUARE);
      if(c == '{') READ_RETURN(Token::OPEN_CURLY);
      if(c == '}') READ_RETURN(Token::CLOSE_CURLY);
      if(c == '=') READ_RETURN(ReadSymbolSuffix(source, c, '=', Token::ASSIGN, Token::EQUAL));
      if(c == '+') READ_RETURN(Token::ADD);
      if(c == '-') READ_RETURN(Token::SUB);
      if(c == '*') READ_RETURN(Token::MUL);
      if(c == '/') READ_RETURN(Token::DIV);
      if(c == '!') READ_RETURN(ReadSymbolSuffix(source, c, '=', Token::NOT, Token::NEQUAL));
      if(c == '&') READ_RETURN(ReadSymbolSuffix(source, c, '&', Token::BIN_AND, Token::AND));
      if(c == '|') READ_RETURN(ReadSymbolSuffix(source, c, '|', Token::BIN_OR, Token::OR));
      if(c == '^') READ_RETURN(Token::BIN_XOR);
      if(c == '~') READ_RETURN(Token::BIN_NOT);
      if(c == '<') READ_RETURN(ReadSymbolSuffix(source, c, '=', Token::LT, Token::LTE));
      if(c == '>') READ_RETURN(ReadSymbolSuffix(source, c, '=', Token::GT, Token::GTE));;
      if(c == '#') READ_RETURN(Token::HASH);
      if(c == '.') READ_RETURN(Token::DOT);
      if(c == ',') READ_RETURN(Token::COMMA);
      if(c == ':') READ_RETURN(Token::COLON);
      if(c == ';') READ_RETURN(Token::SEMICOLON);

      std::cerr << "Invalid symbol: \"" << c << "\"" << std::endl;
      return Token::INVALID;
    }

    // Returns different Tokens depending on if the suffix exists behind char.
    static Token ReadSymbolSuffix(std::istream& source, char& c, char suffix, Token noSuffix, Token withSuffix)
    {
      if(c == suffix) READ_RETURN(withSuffix);
      return noSuffix;
    }

    static std::string ReadName(std::istream& source, char& c)
    {
      std::stringstream ss;
      while(IsLetter(c) || IsNumber(c) || c == '_')
      {
        ss << c;
        source >> c;
      }

      return ss.str();
    }

    static std::string ReadNumber(std::istream& source, char& c)
    {
      std::stringstream ss;
      bool hasReadComma = false;
      while(IsNumber(c) || (c == '.' && !hasReadComma))
      {
        if(c == '.')
          hasReadComma = true;
        ss << c;
        source >> c;
      }

      return ss.str();
    }
};
