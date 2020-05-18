#pragma once

#include "Token.h"

#include <vector>
#include <iostream>

#define VALID_TOKEN(token) if(!data.Read(token)) { PrintError(data, data.next, token); return false; };
#define VALID_PRODUCTION(production) \
  if(!production) \
{ \
  if(data.printFail) \
  std::cerr << "Invalid product: " << #production << " at " << Tokens::GetName(data.next) << " linenr: " << __LINE__ << std::endl;\
  return false; \
}

struct ParseData
{
  const std::vector<Token>& tokens;
  size_t pos;
  Token next;
  bool printFail = true;
  ParseData(const std::vector<Token>& tokens)
    : tokens{tokens}, pos{0}, next{tokens[0]}
  {}

  bool Read(Token token)
  {
    if(next != token)
      return false;
    if(pos >= tokens.size())
    {
      std::cerr << "No more tokens to be read" << std::endl;
      return false;
    }
    next = tokens[++pos];
    return true;
  }

  void Backtrack(size_t pos)
  {
    this->pos = pos;
    next = tokens[pos];
  }
};

class Parser
{
  public:
    static void Parse(const std::vector<Token>& tokens)
    {
      ParseData data{tokens};
      if(!Function(data))
      {
        std::cerr << "Failed to parse file" << std::endl;
      }
    }

  private:
    static bool Function(ParseData& data)
    {
      if(!FunctionType(data))
      {
        PrintError(data, "primitive");
        return false;
      }
      VALID_TOKEN(Token::NAME);
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(FunctionParams(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_TOKEN(Token::OPEN_CURLY);
      VALID_PRODUCTION(Statements(data));
      VALID_TOKEN(Token::CLOSE_CURLY);
      return true;
    }

    // Ss -> S
    //    -> S Ss
    static bool Statements(ParseData& data)
    {
      size_t pos = data.pos;
      VALID_PRODUCTION(Statement(data));
      if(data.pos != pos)
      {
        return Statements(data);
      }
      return true;
    }

    // S -> IF
    //   -> for(EA;E;EA) { Ss }
    //   -> while(E) { Ss }
    static bool Statement(ParseData& data)
    {
      if(data.Read(Token::IF))
      {
        return StatementIf(data);
      }
      else if(data.Read(Token::FOR))
      {
        VALID_TOKEN(Token::OPEN_PARAM);
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::SEMICOLON);
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::SEMICOLON);
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        VALID_TOKEN(Token::OPEN_CURLY);
        VALID_PRODUCTION(Statements(data));
        VALID_TOKEN(Token::CLOSE_CURLY);
        return true;
      }
      else if(data.Read(Token::WHILE))
      {
        VALID_TOKEN(Token::OPEN_PARAM);
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        VALID_TOKEN(Token::OPEN_CURLY);
        VALID_PRODUCTION(Statements(data));
        VALID_TOKEN(Token::CLOSE_CURLY);
        return true;
      }
      else if(data.Read(Token::RETURN))
      {
        data.printFail = false;
        Expression(data);
        data.printFail = true;
        VALID_TOKEN(Token::SEMICOLON);
      }
      else if(data.Read(Token::SEMICOLON))
      {
        return true;
      }
      else
      {
        data.printFail = false;
        int pos = data.pos;
        if(Expression(data))
        {
          data.printFail = true;
          VALID_TOKEN(Token::SEMICOLON);
          return true;
        }
        data.printFail = true;

        // Failed to parse if the pos changed but expression failed
        if(pos != data.pos)
        {
          return false;
        }
      }
      return true;
    }

    // IF -> if(E) { Ss }
    //    -> if(E) { Ss } ELSE_IF
    //    -> if(E) { Ss } ELSE
    static bool StatementIf(ParseData& data)
    {
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(Expression(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_TOKEN(Token::OPEN_CURLY);
      VALID_PRODUCTION(Statements(data));
      VALID_TOKEN(Token::CLOSE_CURLY);
      if(data.Read(Token::ELSE_IF))
        return StatementElseIf(data);
      else if(data.Read(Token::ELSE))
        return StatementElse(data);
      return true;
    }

    // IF -> elif(E) { Ss }
    //    -> elif(E) { Ss } ELSE_IF
    //    -> elif(E) { Ss } ELSE
    static bool StatementElseIf(ParseData& data)
    {
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(Expression(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_TOKEN(Token::OPEN_CURLY);
      VALID_PRODUCTION(Statements(data));
      VALID_TOKEN(Token::CLOSE_CURLY);

      if(data.Read(Token::ELSE_IF))
        return StatementElseIf(data);
      else if(data.Read(Token::ELSE))
        return StatementElse(data);
      return true;
    }

    // ELSE -> { Ss }
    static bool StatementElse(ParseData& data)
    {
      VALID_TOKEN(Token::OPEN_CURLY);
      VALID_PRODUCTION(Statements(data));
      VALID_TOKEN(Token::CLOSE_CURLY);
      return true;
    }

    // E -> LVALD = EL
    //   -> EL
    static bool Expression(ParseData& data)
    {
      int pos = data.pos;
      if(LValueDefine(data))
      {
        if(data.Read(Token::ASSIGN))
        {
          return ExpressionLogical(data);
        }
        else
        {
          // Go back since there was no assignment
          data.Backtrack(pos);
        }
      }
      return ExpressionLogical(data);
    }

    // EL -> EC && EL
    //    -> EC || EL
    static bool ExpressionLogical(ParseData& data)
    {
      VALID_PRODUCTION(ExpressionCompare(data));
      if(data.Read(Token::AND))
        return ExpressionLogical(data);
      else if(data.Read(Token::OR))
        return ExpressionLogical(data);
      return true;
    }

    // EC -> EAS
    //    -> EAS == EC
    //    -> EAS != EC
    //    -> EAS >= EC
    //    -> EAS <= EC
    //    -> EAS > EC
    //    -> EAS < EC
    static bool ExpressionCompare(ParseData& data)
    {
      VALID_PRODUCTION(ExpressionAddSub(data));
      if(data.Read(Token::EQUAL))
      {
        return ExpressionCompare(data);
      }
      else if(data.Read(Token::NEQUAL))
      {
        return ExpressionCompare(data);
      }
      else if(data.Read(Token::GTE))
      {
        return ExpressionCompare(data);
      }
      else if(data.Read(Token::LTE))
      {
        return ExpressionCompare(data);
      }
      else if(data.Read(Token::GT))
      {
        return ExpressionCompare(data);
      }
      else if(data.Read(Token::LT))
      {
        return ExpressionCompare(data);
      }
      return true;
    }

    // EAS -> EMD
    //     -> EMD + EAS
    //     -> EMD - EAS
    static bool ExpressionAddSub(ParseData& data)
    {
      VALID_PRODUCTION(ExpressionMulDiv(data));
      if(data.Read(Token::ADD))
      {
        return ExpressionAddSub(data);
      }
      else if(data.Read(Token::SUB))
      {
        return ExpressionAddSub(data);
      }
      return true;
    }

    // EMD -> EV
    //     -> EV * EMD
    //     -> EV / EMD
    static bool ExpressionMulDiv(ParseData& data)
    {
      VALID_PRODUCTION(ExpressionValue(data));
      if(data.Read(Token::MUL))
      {
        return ExpressionMulDiv(data);
      }
      else if(data.Read(Token::DIV))
      {
        return ExpressionMulDiv(data);
      }
      return true;
    }

    // EV -> !EV
    //    -> name
    //    -> number
    //    -> string
    //    -> char
    //    -> (E)
    static bool ExpressionValue(ParseData& data)
    {
      if(data.Read(Token::NOT))
      {
        return ExpressionValue(data);
      }
      else if(data.Read(Token::NUMBER))
      {
        return true;
      }
      else if(data.Read(Token::STRING))
      {
        return true;
      }
      else if(data.Read(Token::CHAR))
      {
        return true;
      }
      else if(data.Read(Token::OPEN_PARAM))
      {
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        return true;
      }
      return LValue(data);
    }

    // LVALD -> LVAL
    //       -> PRIM LVAL
    static bool LValueDefine(ParseData& data)
    {
      Primitive(data);
      return LValue(data);
    }
    // LVAL -> name[E]
    //      -> name
    static bool LValue(ParseData& data)
    {
      if(data.Read(Token::NAME))
      {
        if(data.Read(Token::OPEN_SQUARE))
        {
          VALID_PRODUCTION(Expression(data));
          VALID_TOKEN(Token::CLOSE_SQUARE);
        }
        return true;
      }
      return false;
    }

    // FTYPE -> PRIM
    //       -> void
    static bool FunctionType(ParseData& data)
    {
      if(!Primitive(data))
      {
        if(data.Read(Token::VOID))
        {
          return true;
        }
        return false;
      }
      return true;
    }

    // FPARAM -> PRIM name
    //        -> PRIM name, FPARAM
    static bool FunctionParams(ParseData& data)
    {
      if(data.next == Token::CLOSE_PARAM)
        return true;

      VALID_PRODUCTION(Primitive(data));
      VALID_TOKEN(Token::NAME);
      if(data.Read(Token::COMMA))
        return FunctionParams(data);
      return true;
    }

    // PRIM -> int
    //      -> float
    //      -> char
    //      -> string
    static bool Primitive(ParseData& data)
    {
      if(data.Read(Token::INT))
      {
        return true;
      }
      else if(data.Read(Token::FLOAT))
      {
        return true;
      }
      else if(data.Read(Token::CHAR_K))
      {
        return true;
      }
      else if(data.Read(Token::STRING_K))
      {
        return true;
      }
      return false;
    }

    static void PrintError(ParseData& data, Token got, Token expected)
    {
      std::cerr << "Invalid token at " << data.pos << std::endl;
      std::cerr << "Got " << Tokens::GetName(got) << " but expected " << Tokens::GetName(expected) << std::endl;
    }

    static void PrintError(ParseData& data, const std::string& type)
    {
      std::cerr << "Invalid token at " << data.pos << std::endl;
      std::cerr << "Expected " << type << std::endl;
    }
};
