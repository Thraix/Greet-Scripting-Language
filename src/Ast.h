#pragma once

#define RETURN_FALSE(x) if(!(x)) return false

#include <iostream>

enum class Type
{
  INVALID, VOID, INT, FLOAT, CHAR, STRING
};

struct AstNode
{
  void PrintIndent(std::ostream& os, size_t indent)
  {
    for(int i = 0;i<indent;i++)
    {
      os << "| ";
    }
  }

  void PrintWithIndent(std::ostream& os, size_t indent)
  {
    PrintIndent(os, indent);
    Print(os, indent);
  }
  virtual bool Check() = 0;
  virtual void Print(std::ostream& os, size_t indent) = 0;

  friend std::ostream& operator<<(std::ostream& os, AstNode* node)
  {
    node->PrintWithIndent(os, 0);
    return os;
  }
};

struct AstNodeImpl : public AstNode
{
  bool Check(){ return true; }
  void Print(std::ostream& os, size_t index)
  {
    os << "AstNodeImpl" << std::endl;
  }
};

struct AstName : public AstNode
{
  Type type;
  AstName(Type type)
    : type{type}
  {}

  bool Check() override { return true; }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "AstName" << std::endl;
  }
};

struct AstFuncParam : public AstNode
{
  AstName* arg;
  AstFuncParam(AstName* arg)
    : arg{arg}
  {}

  bool Check() override { return true; }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "AstParam" << std::endl;
    arg->PrintWithIndent(os, indent+1);
  }
};

struct AstFuncParams : public AstNode
{
  AstFuncParam* first;
  AstFuncParams* tail;
  AstFuncParams(AstFuncParam* first, AstFuncParams* tail)
    : first{first}, tail{tail}
  {}

  bool Check() override { return true; }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "AstFuncParams" << std::endl;
    if(first)
    {
      first->PrintWithIndent(os, indent+1);
      if(tail)
        tail->PrintWithIndent(os, indent);
    }
  }
};

struct AstStatement : public AstNode
{
  AstStatement()
  {}

  bool Check() override { return true; }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "AstStatement" << std::endl;
  }
};

struct AstExpression : public AstStatement
{
  Type type;
  void* value;
};


struct AstStatements : public AstNode
{
  AstStatement* first;
  AstStatements* tail;
  AstStatements(AstStatement* first, AstStatements* tail)
    : first{first}, tail{tail}
  {}

  bool Check() override
  {
    if(first)
      RETURN_FALSE(first->Check());
    if(tail)
      RETURN_FALSE(tail->Check());
    return true;
  }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "AstStatements" << std::endl;
    if(first)
    {
      first->PrintWithIndent(os, indent+1);
      if(tail)
        tail->PrintWithIndent(os, indent+1);
    }
  }
};

struct AstIf : public AstStatement
{
  AstExpression* condition;
  AstStatements* body;
  AstStatements* elseBody;

  AstIf(AstExpression* condition, AstStatements* body)
    : condition{condition}, body{body}, elseBody{nullptr}
  {}

  AstIf(AstExpression* condition, AstStatements* body, AstStatements* elseBody)
    : condition{condition}, body{body}, elseBody{elseBody}
  {}

  bool Check() { return false; }

  void Print(std::ostream& os, size_t indent)
  {
    os << "[IF]" << std::endl;
    PrintIndent(os, indent+1);
    os << "[CONDITION]" << std::endl;
    condition->PrintWithIndent(os, indent+2);
    PrintIndent(os, indent+1);
    os << "[BODY]" << std::endl;
    body->PrintWithIndent(os, indent+2);
    if(elseBody)
    {
      PrintIndent(os, indent+1);
      os << "[ELSE] " << std::endl;
      elseBody->PrintWithIndent(os, indent+2);
    }
  }
};

struct AstFunction : public AstNode
{
  AstName* name;
  AstFuncParams* params;
  AstStatements* body;
  AstFunction(AstName* name, AstFuncParams* params, AstStatements* body)
    : name{name}, params{params}, body{body}
  {}

  bool Check() override { return body->Check(); }

  void Print(std::ostream& os, size_t indent) override
  {
    os << "[FUNCTION] " << std::endl;
    PrintIndent(os, indent+1);
    os << "[NAME] " << std::endl;
    name->PrintWithIndent(os, indent+2);
    PrintIndent(os, indent+1);
    os << "[PARAMS] " << std::endl;
    params->PrintWithIndent(os, indent+2);
    PrintIndent(os, indent+1);
    os << "[BODY] " << std::endl;
    body->PrintWithIndent(os, indent+2);
  }
};

struct AstExpressionImpl : public AstExpression
{
  void Print(std::ostream& os, size_t indent)
  {
    os << "AstExpressionImpl" << std::endl;
  }

  bool Check()
  {
    return true;
  }
};

struct AstBinOp : public AstExpression
{
  private:
    AstExpression* left;
    AstExpression* right;
  public:
    AstBinOp(AstExpression* left, AstExpression* right)
      : AstExpression{}, left{left}, right{right}
    {}

  void Print(std::ostream& os, size_t indent)
  {
    os << "AstBinOp" << std::endl;
    left->PrintWithIndent(os, indent);
    right->PrintWithIndent(os, indent);
  }

  bool Check()
  {
    RETURN_FALSE(left->Check());
    RETURN_FALSE(right->Check());
    type = left->type;
    return left->type == right->type;
  }
};

struct AstUnOp : public AstExpression
{
  private:
    AstExpression* expr;
  public:
    AstUnOp(AstExpression* expr)
      : AstExpression{}, expr{expr}
    {}

  bool Check()
  {
    RETURN_FALSE(expr->Check());
    type = expr->type;
    return true;
  }
};

struct AstAdd : public AstBinOp
{
  public:
    AstAdd(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstSub : public AstBinOp
{
  public:
    AstSub(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstMul : public AstBinOp
{
  public:
    AstMul(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstDiv : public AstBinOp
{
  public:
    AstDiv(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstEqual : public AstBinOp
{
  public:
    AstEqual(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstNEqual : public AstBinOp
{
  public:
    AstNEqual(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstLT : public AstBinOp
{
  public:
    AstLT(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstGT : public AstBinOp
{
  public:
    AstGT(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstLTE : public AstBinOp
{
  public:
    AstLTE(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstGTE : public AstBinOp
{
  public:
    AstGTE(AstExpression* left, AstExpression* right)
      : AstBinOp{left, right}
    {}
};

struct AstUMinus : public AstUnOp
{
  public:
    AstUMinus(AstExpression* expr)
      : AstUnOp{expr}
    {}
};

struct AstNot : public AstUnOp
{
  public:
    AstNot(AstExpression* expr)
      : AstUnOp{expr}
    {}
};

struct AstFor
{
};
