#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

class AST;
class Expr;
class Factor;
class BinaryOp;
class typeDecl;

class ASTVisitor
{
public:
  virtual void visit(AST &){};
  virtual void visit(Expr &){};
  virtual void visit(Factor &) = 0;
  virtual void visit(BinaryOp &) = 0;
  virtual void visit(typeDecl &) = 0;
};

class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0;
};
class IfStatement : public AST
{
  Expr *condition;
  AST *ThenStmt;
  llvm::SmallVector<std::pair<Expr *, AST *>,8> ElifStmts;
  AST *ElseStmt;
  public:
    IfStatement(Expr * Cond , AST *Then , llvm::SmallVector<std::pair<Expr *, AST *>,8> Elif, AST * Else):
      condition(Cond), ThenStmt(Then) , ElifStmts(Elif) , ElseStmt(Else) {}
    Expr *getCondition(){ return condition;}
    AST * getThenStmt(){ return ThenStmt;}
    llvm::SmallVector<std::pair<Expr *, AST *>,8> getElifStmts(){ return ElifStmts;}
    AST * getElseStmt(){return ElseStmt;}
    virtual void accept(ASTVisitor &V) override
    {
      V.visit(*this);
    }

};
class loopcStatement : public AST
{
  Expr *condition;
  llvm::SmallVector<Expr *,8> body;

public:
  loopcStatement(Expr *cond , llvm::SmallVector<Expr *,8> Body):
            condition(cond),
            body(Body){}
  Expr *getCondition(){
    return condition;
  }
  llvm::SmallVector<Expr *,8> getBody(){
    return body;
  }
  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
}

class Expr : public AST
{
public:
  Expr() {}
};

class Factor : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind;
  llvm::StringRef Val;

public:
  Factor(ValueKind Kind, llvm::StringRef Val)
      : Kind(Kind), Val(Val) {}
  ValueKind getKind() { return Kind; }
  llvm::StringRef getVal() { return Val; }
  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Plus,
    Minus,
    Mul,
    Div,
    Equal
  };

private:
  Expr *Left;
  Expr *Right;
  Operator Op;

public:
  BinaryOp(Operator Op, Expr *L, Expr *R)
      : Op(Op), Left(L), Right(R) {}
  Expr *getLeft() { return Left; }
  Expr *getRight() { return Right; }
  Operator getOperator() { return Op; }
  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class typeDecl : public AST
{
  using VarVector = llvm::SmallVector<llvm::StringRef, 8>;
  VarVector Vars;
  Expr *E;

public:
  typeDecl(llvm::SmallVector<llvm::StringRef, 8> Vars)
      : Vars(Vars) {}
  VarVector::const_iterator begin() { return Vars.begin(); }
  VarVector::const_iterator end() { return Vars.end(); }
  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};
#endif