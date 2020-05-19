#pragma once

#include "Token.h"
#include "Ast.h"

#include <vector>
#include <iostream>

#define VALID_TOKEN(token) if(!data.Read(token)) { PrintError(data, data.Top(), token); return nullptr; };
#define VALID_PRODUCTION(type, node, production) \
  type* node = production; \
  if(node == nullptr) \
{ \
  return nullptr; \
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
        AstFunction* func = Function(data);
        if(func == nullptr)
        {
          std::cerr << "Failed to parse file" << std::endl;
          std::cerr << "Invalid symbol at " << data.TopPos() << std::endl;
          return false;
        }
        std::cout << func << std::endl;
      }
      return true;
    }

  private:

    // FUNC -> FTYPE name ( FPARAMS ) { Ss }
    static AstFunction* Function(ParseData& data)
    {
      Type type;
      if((type = FunctionType(data)) == Type::INVALID)
        return nullptr;
      VALID_TOKEN(Token::NAME);
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(AstFuncParams, params, FunctionParams(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_TOKEN(Token::OPEN_CURLY);
      VALID_PRODUCTION(AstStatements, body, Statements(data));
      VALID_TOKEN(Token::CLOSE_CURLY);
      return new AstFunction(new AstName(type), params, body);
    }

    // Ss -> S
    //    -> S Ss
    static AstStatements* Statements(ParseData& data)
    {
      AstStatements* statements = new AstStatements(Statement(data), nullptr);
      if(statements->first == nullptr)
        return statements;

      AstStatement* statement;
      while((statement = Statement(data)) != nullptr)
      {
        statements->tail = new AstStatements(statement, nullptr);
      }
      return statements;
    }

    // S -> IF
    //   -> for ( EA ; E ; EA ) { Ss }
    //   -> while ( E ) { Ss }
    static AstStatement* Statement(ParseData& data)
    {
      if(data.Top() == Token::IF)
      {
        return StatementIf(data);
      }
      else if(data.Read(Token::FOR))
      {
        VALID_TOKEN(Token::OPEN_PARAM);
        VALID_PRODUCTION(AstExpression, init, Expression(data));
        VALID_TOKEN(Token::SEMICOLON);
        VALID_PRODUCTION(AstExpression, until, Expression(data));
        VALID_TOKEN(Token::SEMICOLON);
        VALID_PRODUCTION(AstExpression, next, Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        VALID_PRODUCTION(AstStatements, body, ControlFlowBody(data));
        return new AstStatement();
      }
      else if(data.Read(Token::WHILE))
      {
        VALID_TOKEN(Token::OPEN_PARAM);
        VALID_PRODUCTION(AstExpression, until, Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        VALID_PRODUCTION(AstStatements, body, ControlFlowBody(data));
        return new AstStatement();
      }
      else if(data.Read(Token::RETURN))
      {
        AstNode* node = Expression(data);
        VALID_TOKEN(Token::SEMICOLON);
        return new AstStatement();
      }
      else if(data.Read(Token::SEMICOLON))
      {
        return new AstExpressionImpl();
      }
      else
      {
        int pos = data.pos;
        AstExpression* node = Expression(data);
        if(node != nullptr)
        {
          VALID_TOKEN(Token::SEMICOLON);
          return node;
        }

        return nullptr;
      }
      return nullptr;
    }

    // IF -> if ( E ) CFBODY
    //    -> if ( E ) CFBODY ELSE
    static AstIf* StatementIf(ParseData& data)
    {
      VALID_TOKEN(Token::IF);
      VALID_TOKEN(Token::OPEN_PARAM);
      VALID_PRODUCTION(AstExpression, condition, Expression(data));
      VALID_TOKEN(Token::CLOSE_PARAM);
      VALID_PRODUCTION(AstStatements, body, ControlFlowBody(data));

      if(data.Top() == Token::ELSE)
      {
        VALID_PRODUCTION(AstStatements, elseBody, StatementElse(data));
        return new AstIf(condition, body, elseBody);
      }
      return new AstIf(condition, body);
    }

    // ELSE -> else CFBODY
    static AstStatements* StatementElse(ParseData& data)
    {
      VALID_TOKEN(Token::ELSE);
      VALID_PRODUCTION(AstStatements, body, ControlFlowBody(data));
      return body;
    }

    // E -> LVALD = EL
    //   -> EL
    static AstExpression* Expression(ParseData& data)
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
    static AstExpression* ExpressionLogical(ParseData& data)
    {
      VALID_PRODUCTION(AstExpression, left, ExpressionCompare(data));
      if(data.Read(Token::AND))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionLogical(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::OR))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionLogical(data));
        return new AstExpressionImpl();
      }
      return left;
    }

    // EC -> EAS
    //    -> EAS == EC
    //    -> EAS != EC
    //    -> EAS >= EC
    //    -> EAS <= EC
    //    -> EAS > EC
    //    -> EAS < EC
    static AstExpression* ExpressionCompare(ParseData& data)
    {
      VALID_PRODUCTION(AstExpression, left, ExpressionAddSub(data));
      if(data.Read(Token::EQUAL))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::NEQUAL))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::GTE))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::LTE))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::GT))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::LT))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionCompare(data));
        return new AstExpressionImpl();
      }
      return left;
    }

    // EAS -> EMD
    //     -> EMD + EAS
    //     -> EMD - EAS
    static AstExpression* ExpressionAddSub(ParseData& data)
    {
      VALID_PRODUCTION(AstExpression, node, ExpressionMulDiv(data));
      if(data.Read(Token::ADD))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionAddSub(data));
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::SUB))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionAddSub(data));
        return new AstExpressionImpl();
      }
      return node;
    }

    // EMD -> EV
    //     -> EV * EMD
    //     -> EV / EMD
    static AstExpression* ExpressionMulDiv(ParseData& data)
    {
      VALID_PRODUCTION(AstExpression, left, ExpressionUnary(data));
      if(data.Read(Token::MUL))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionMulDiv(data));
        return new AstMul(left, right);
      }
      else if(data.Read(Token::DIV))
      {
        VALID_PRODUCTION(AstExpression, right, ExpressionMulDiv(data));
        return new AstDiv(left, right);
      }
      return left;
    }

    // EU -> ! EU
    //    -> EP
    static AstExpression* ExpressionUnary(ParseData& data)
    {
      if(data.Read(Token::NOT))
      {
        return ExpressionUnary(data);
      }
      else if(data.Read(Token::SUB))
      {
        return ExpressionUnary(data);
      }
      VALID_PRODUCTION(AstExpression, node, RValue(data));
      return node;
    }

    // EP -> ( E )
    //    -> RVAL
    static AstExpression* ExpressionParam(ParseData& data)
    {
      if(data.Read(Token::OPEN_PARAM))
      {
        return Expression(data);
      }
      VALID_PRODUCTION(AstExpression, node, RValue(data));
      return node;
    }

    // RVAL -> number
    //      -> string
    //      -> char
    //      -> ( E )
    //      -> name INDEX
    //      -> name ( FARGS )
    //      -> name
    static AstExpression* RValue(ParseData& data)
    {
      if(data.Read(Token::NUMBER))
      {
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::STRING))
      {
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::CHAR))
      {
        return new AstExpressionImpl();
      }
      else if(data.Read(Token::OPEN_PARAM))
      {
        VALID_PRODUCTION(AstExpression, node, Expression(data));
        VALID_TOKEN(Token::CLOSE_PARAM);
        return node;
      }
      else if(data.Read(Token::NAME))
      {
        AstExpression* topNode = new AstExpressionImpl();
        if(data.Top() == Token::OPEN_SQUARE)
        {
          VALID_PRODUCTION(AstNode, node, Indexing(data));
        }
        if(data.Top() == Token::OPEN_PARAM)
        {
          VALID_TOKEN(Token::OPEN_PARAM);
          VALID_PRODUCTION(AstNode, node, FunctionArguments(data));
          VALID_TOKEN(Token::CLOSE_PARAM);
        }
        return topNode;
      }
      return nullptr;
    }

    // LVALD -> LVAL
    //       -> PRIM LVAL
    static AstNode* LValueDefine(ParseData& data)
    {
      Primitive(data);
      return LValue(data);
    }

    // LVAL -> name INDEX
    //      -> name
    static AstNode* LValue(ParseData& data)
    {
      if(data.Read(Token::NAME))
      {
        if(data.Top() == Token::OPEN_SQUARE)
        {
          VALID_PRODUCTION(AstNode, node, Indexing(data));
        }
        return new AstNodeImpl();
      }
      return nullptr;
    }

    // INDEX -> [ E ]
    static AstNode* Indexing(ParseData& data)
    {
      VALID_TOKEN(Token::OPEN_SQUARE);
      VALID_PRODUCTION(AstNode, node, Expression(data));
      VALID_TOKEN(Token::CLOSE_SQUARE);
      return new AstNodeImpl();
    }

    // FTYPE -> PRIM
    //       -> void
    static Type FunctionType(ParseData& data)
    {
      Type type = Type::INVALID;
      if((type = Primitive(data)) == Type::INVALID)
      {
        if(data.Read(Token::VOID))
        {
          return Type::VOID;
        }
        return Type::INVALID;
      }
      return type;
    }

    // FARGS -> name
    //       -> name, FARGS
    static AstNode* FunctionArguments(ParseData& data)
    {
      if(data.Top() == Token::CLOSE_PARAM)
        return new AstNodeImpl();

      VALID_PRODUCTION(AstNode, node, Expression(data));
      if(data.Read(Token::COMMA))
      {
        VALID_PRODUCTION(AstNode, args, FunctionArguments(data));
        return node;
      }
      return node;
    }

    // CFBODY -> { Ss }
    //        -> S
    static AstStatements* ControlFlowBody(ParseData& data)
    {
      if(data.Top() == Token::OPEN_CURLY)
      {
        VALID_TOKEN(Token::OPEN_CURLY);
        VALID_PRODUCTION(AstStatements, node, Statements(data));
        VALID_TOKEN(Token::CLOSE_CURLY);
        return node;
      }
      else
      {
        VALID_PRODUCTION(AstStatement, node, Statement(data));
        return new AstStatements(node, nullptr);
      }
    }

    // FPARAMS ->
    //         -> PRIM name
    //         -> PRIM name, FPARAMS
    static AstFuncParams* FunctionParams(ParseData& data)
    {
      AstFuncParams* top = new AstFuncParams{nullptr, nullptr};
      while(data.Top() != Token::CLOSE_PARAM)
      {
        Type type;
        if((type = Primitive(data)) == Type::INVALID)
          return nullptr;
        VALID_TOKEN(Token::NAME);
        // TODO: FIX NAME
        top = new AstFuncParams(new AstFuncParam(new AstName{type}), top);
        if(!data.Read(Token::COMMA))
          break;
      }

      return top;
    }

    // PRIM -> int
    //      -> float
    //      -> char_k
    //      -> string_k
    static Type Primitive(ParseData& data)
    {
      if(data.Read(Token::INT))
      {
        return Type::INT;
      }
      else if(data.Read(Token::FLOAT))
      {
        return Type::FLOAT;
      }
      else if(data.Read(Token::CHAR_K))
      {
        return Type::CHAR;
      }
      else if(data.Read(Token::STRING_K))
      {
        return Type::STRING;
      }
      return Type::INVALID;
    }

    static void PrintError(ParseData& data, Token got, Token expected)
    {
      std::cerr << "Invalid token at " << data.pos << std::endl;
      std::cerr << "Got " << Tokens::GetName(got) << " but expected " << Tokens::GetName(expected) << std::endl;
    }
};
