#pragma once

#include "Token.h"

#include <vector>
#include <iostream>

#define VALID_TOKEN(token) if(!data.Read(token)) { PrintError(data, data.Top(), token); return false; };
#define VALID_PRODUCTION(production) \
  if(!production) \
{ \
  return false; \
}

struct ParseData
{
  const std::vector<TokenPos>& tokens;
  size_t pos;
  ParseData(const std::vector<TokenPos>& tokens)
    : tokens{tokens}, pos{0}
  {}

  bool Read(Token token)
  {
    if(Top() != token)
      return false;
    if(pos >= tokens.size())
    {
      std::cerr << "No more tokens to be read" << std::endl;
      return false;
    }
    ++pos;
    return true;
  }

  void Backtrack(size_t pos)
  {
    this->pos = pos;
  }

  bool Empty()
  {
    return pos == tokens.size();
  }

  Token Top()
  {
    return tokens[pos].token;
  }

  TokenPos TopPos()
  {
    return tokens[pos];
  }
};

class Parser
{
  public:
    static bool Parse(const std::vector<TokenPos>& tokens)
    {
      ParseData data{tokens};
      while(!data.Empty())
      {
        if(!Function(data))
        {
          std::cerr << "Failed to parse file" << std::endl;
          std::cerr << "Invalid symbol at " << data.TopPos() << std::endl;
          return false;
        }
      }
      return true;
    }

  private:

    // FUNC -> FTYPE name ( FPARAMS ) { Ss }
    static bool Function(ParseData& data)
    {
      VALID_PRODUCTION(FunctionType(data));
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
      // If something was read, read another statement
      if(Statement(data))
      {
        return Statements(data);
      }
      return true;
    }

    // S -> IF
    //   -> for ( EA ; E ; EA ) { Ss }
    //   -> while ( E ) { Ss }
    static bool Statement(ParseData& data)
    {
      if(data.Top() == Token::IF)
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
        VALID_PRODUCTION(ControlFlowBody(data));
        return true;
      }
      else if(data.Read(Token::WHILE))
      {
        VALID_TOKEN(Token::OPEN_PARAM);
        VALID_PRODUCTION(Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        VALID_PRODUCTION(ControlFlowBody(data));
        return true;
      }
      else if(data.Read(Token::RETURN))
      {
        Expression(data);
        VALID_TOKEN(Token::SEMICOLON);
        return true;
      }
      else if(data.Read(Token::SEMICOLON))
      {
        return true;
      }
      else
      {
        int pos = data.pos;
        if(Expression(data))
        {
          VALID_TOKEN(Token::SEMICOLON);
          return true;
        }

        // Failed to parse if the pos changed but expression failed
        return false;
      }
      return false;
    }

    // IF -> if ( E ) CFBODY
    //    -> if ( E ) CFBODY ELSE_IF
    //    -> if ( E ) CFBODY ELSE
    static bool StatementIf(ParseData& data)
    {
      VALID_TOKEN(Token::IF);
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(Expression(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_PRODUCTION(ControlFlowBody(data));

      if(data.Top() == Token::ELSE_IF)
      {
        return StatementElseIf(data);
      }
      else if(data.Top() == Token::ELSE)
      {
        return StatementElse(data);
      }
      return true;
    }

    // IF -> elif ( E ) CFBODY
    //    -> elif ( E ) CFBODY ELSE_IF
    //    -> elif ( E ) CFBODY ELSE
    static bool StatementElseIf(ParseData& data)
    {
      VALID_TOKEN(Token::ELSE_IF);
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(Expression(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_PRODUCTION(ControlFlowBody(data));

      if(data.Top() == Token::ELSE_IF)
      {
        return StatementElseIf(data);
      }
      else if(data.Top() == Token::ELSE)
      {
        return StatementElse(data);
      }
      return true;
    }

    // ELSE -> else CFBODY
    static bool StatementElse(ParseData& data)
    {
      VALID_TOKEN(Token::ELSE);
      VALID_PRODUCTION(ControlFlowBody(data));
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
      {
        return ExpressionLogical(data);
      }
      else if(data.Read(Token::OR))
      {
        return ExpressionLogical(data);
      }
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
      VALID_PRODUCTION(ExpressionUnary(data));
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

    // EU -> ! EU
    //    -> EP
    static bool ExpressionUnary(ParseData& data)
    {
      if(data.Read(Token::NOT))
      {
        return ExpressionUnary(data);
      }
      else if(data.Read(Token::SUB))
      {
        return ExpressionUnary(data);
      }
      VALID_PRODUCTION(RValue(data));
      return true;
    }

    // EP -> ( E )
    //    -> RVAL
    static bool ExpressionParam(ParseData& data)
    {
      if(data.Read(Token::OPEN_PARAM))
      {
        return Expression(data);
      }
      VALID_PRODUCTION(RValue(data));
      return true;
    }

    // RVAL -> number
    //      -> string
    //      -> char
    //      -> ( E )
    //      -> name INDEX
    //      -> name ( FARGS )
    //      -> name
    static bool RValue(ParseData& data)
    {
      if(data.Read(Token::NUMBER))
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
      else if(data.Read(Token::NAME))
      {
        if(data.Top() == Token::OPEN_SQUARE)
        {
          VALID_PRODUCTION(Indexing(data));
        }
        if(data.Top() == Token::OPEN_PARAM)
        {
          VALID_TOKEN(Token::OPEN_PARAM);
          VALID_PRODUCTION(FunctionArguments(data));
          VALID_TOKEN(Token::CLOSE_PARAM);
        }
        return true;
      }
      return false;
    }

    // LVALD -> LVAL
    //       -> PRIM LVAL
    static bool LValueDefine(ParseData& data)
    {
      Primitive(data);
      return LValue(data);
    }

    // LVAL -> name INDEX
    //      -> name
    static bool LValue(ParseData& data)
    {
      if(data.Read(Token::NAME))
      {
        if(data.Top() == Token::OPEN_SQUARE)
        {
          VALID_PRODUCTION(Indexing(data));
        }
        return true;
      }
      return false;
    }

    // INDEX -> [ E ]
    static bool Indexing(ParseData& data)
    {
      VALID_TOKEN(Token::OPEN_SQUARE);
      VALID_PRODUCTION(Expression(data));
      VALID_TOKEN(Token::CLOSE_SQUARE);
      return true;
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

    // FARGS -> name
    //       -> name, FARGS
    static bool FunctionArguments(ParseData& data)
    {
      if(data.Top() == Token::CLOSE_PARAM)
        return true;

      VALID_PRODUCTION(Expression(data));
      if(data.Read(Token::COMMA))
      {
        return FunctionArguments(data);
      }
      return true;
    }

    // CFBODY -> { Ss }
    //        -> S
    static bool ControlFlowBody(ParseData& data)
    {
      if(data.Top() == Token::OPEN_CURLY)
      {
        VALID_TOKEN(Token::OPEN_CURLY);
        VALID_PRODUCTION(Statements(data));
        VALID_TOKEN(Token::CLOSE_CURLY);
      }
      else
      {
        VALID_PRODUCTION(Statement(data));
      }
      return true;
    }

    // FPARAMS -> PRIM name
    //         -> PRIM name, FPARAMS
    static bool FunctionParams(ParseData& data)
    {
      if(data.Top() == Token::CLOSE_PARAM)
        return true;

      VALID_PRODUCTION(Primitive(data));
      VALID_TOKEN(Token::NAME);
      if(data.Read(Token::COMMA))
      {
        return FunctionParams(data);
      }
      return true;
    }

    // PRIM -> int
    //      -> float
    //      -> char_k
    //      -> string_k
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
};
