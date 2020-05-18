#pragma once

#include <map>

#define LIST_TOKENS \
  TOKEN(INVALID) \
  TOKEN(NUMBER) \
  TOKEN(STRING) \
  TOKEN(CHAR) \
  TOKEN(OPEN_PARAM) \
  TOKEN(CLOSE_PARAM) \
  TOKEN(OPEN_SQUARE) \
  TOKEN(CLOSE_SQUARE) \
  TOKEN(OPEN_CURLY) \
  TOKEN(CLOSE_CURLY) \
  TOKEN(EQUAL) \
  TOKEN(NEQUAL) \
  TOKEN(ASSIGN) \
  TOKEN(ADD) \
  TOKEN(SUB) \
  TOKEN(MUL) \
  TOKEN(DIV) \
  TOKEN(EXCLAMATION) \
  TOKEN(AND) \
  TOKEN(OR) \
  TOKEN(NOT) \
  TOKEN(BIN_AND) \
  TOKEN(BIN_OR) \
  TOKEN(BIN_XOR) \
  TOKEN(BIN_NOT) \
  TOKEN(LT) \
  TOKEN(GT) \
  TOKEN(LTE) \
  TOKEN(GTE) \
  TOKEN(HASH) \
  TOKEN(DOT) \
  TOKEN(COMMA) \
  TOKEN(COLON) \
  TOKEN(SEMICOLON) \
  TOKEN(INT) \
  TOKEN(FLOAT) \
  TOKEN(STRING_K) \
  TOKEN(CHAR_K) \
  TOKEN(NAME) \
  TOKEN(IF) \
  TOKEN(ELSE) \
  TOKEN(ELSE_IF) \
  TOKEN(FOR) \
  TOKEN(WHILE) \
  TOKEN(RETURN) \
  TOKEN(IN) \
  TOKEN(VOID) \

enum class Token
{
#define TOKEN(x) x,
  LIST_TOKENS
#undef TOKEN
};

class Tokens
{

  private:

    static std::map<std::string_view, Token> reservedTokens;
    static std::map<Token, std::string> tokenName;


  public:
    static std::string GetName(Token token)
    {
      return tokenName.find(token)->second;
    }

    static Token GetReservedToken(const std::string& str)
    {
      auto it = reservedTokens.find(str);
      if(it == reservedTokens.end())
        return Token::INVALID;
      return it->second;
    }
};

std::map<std::string_view, Token> Tokens::reservedTokens{
  {"int", Token::INT},
  {"float", Token::FLOAT},
  {"string", Token::STRING_K},
  {"char", Token::CHAR_K},
  {"if", Token::IF},
  {"for", Token::FOR},
  {"while", Token::WHILE},
  {"else", Token::ELSE},
  {"elif", Token::ELSE_IF},
  {"return", Token::RETURN},
  {"in", Token::IN},
  {"void", Token::VOID},
};

#define TOKEN(x) {Token::x, #x},
    std::map<Token, std::string> Tokens::tokenName{
      LIST_TOKENS
    };
#undef TOKEN
