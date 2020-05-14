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
  TOKEN(PLUS) \
  TOKEN(MINUS) \
  TOKEN(MULTIPLY) \
  TOKEN(DIVIDE) \
  TOKEN(EXCLAMATION) \
  TOKEN(AND) \
  TOKEN(OR) \
  TOKEN(XOR) \
  TOKEN(NOT) \
  TOKEN(LESS) \
  TOKEN(GREATER) \
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
  TOKEN(RETURN) \
  TOKEN(FUNC) \
  TOKEN(IN) \
  TOKEN(VOID) \

enum class Token
{
#define TOKEN(x) x,
  LIST_TOKENS
#undef TOKEN
};

namespace Tokens
{

#define TOKEN(x) {Token::x, #x},
  std::map<Token, std::string> tokenName{
    LIST_TOKENS
  };
#undef TOKEN

  std::map<std::string_view, Token> validTokens{
    {"int", Token::INT},
    {"float", Token::FLOAT},
    {"string", Token::STRING_K},
    {"char", Token::CHAR_K},
    {"if", Token::IF},
    {"else", Token::ELSE},
    {"elif", Token::ELSE_IF},
    {"return", Token::RETURN},
    {"func", Token::FUNC},
    {"in", Token::IN},
    {"void", Token::IN},
  };
}
