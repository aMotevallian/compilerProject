#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"

namespace
{
  class DeclCheck : public ASTVisitor
  {
    llvm::StringSet<> Scope;
    bool HasError;

    enum ErrorType
    {
      Twice,
      Not
    };

    void error(ErrorType ET, llvm::StringRef V)
    {
      llvm::errs() << "Variable " << V << " "
                   << (ET == Twice ? "already" : "not")
                   << " declared\n";
      HasError = true;
    }

  public:
    DeclCheck() : HasError(false) {}

    bool hasError() { return HasError; }

    virtual void visit(Factor &Node) override
    {
      if (Node.getKind() == Factor::Ident)
      {
        if (Scope.find(Node.getVal()) == Scope.end())
          error(Not, Node.getVal());
      }
    };

    virtual void visit(BinaryOp &Node) override
    {
      if (Node.getLeft())
        Node.getLeft()->accept(*this);
      else
        HasError = true;
      if (Node.getRight())
        Node.getRight()->accept(*this);
      else
        HasError = true;
    };

    virtual void visit(typeDecl &Node) override
    {
      for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        if (!Scope.insert(*I).second)
          error(Twice, *I);
      }
    };
  };
  class DeadCodeEliminationVisitor : public ASTVisitor
  {
  private:
    std::unordered_map<llvm::StringRef, std::unordered_set<llvm::StringRef>> lastAssignmentDependencies;

  public:
    DeadCodeEliminationVisitor() {}
    void visit(Factor &factor) override;
    void visit(BinaryOp &binaryOp) override;

    void setDependencies(llvm::StringRef variable, Expr *expr);
    void getDependencies(Expr *expr, std::unordered_set<llvm::StringRef> &dependencies);
    void propagateLiveness(llvm::StringRef variable);

    void visit(typeDecl &typeDecl) override;
  };
}
void DeadCodeEliminationVisitor::visit(Factor &factor)
{
  if (factor.getKind() == Factor::Ident)
  {
    if (factor.isLive())
    {
      factor.setLive(true);
    }
    else
    {
      factor.setLive(false);
    }
  }
}

void DeadCodeEliminationVisitor::visit(BinaryOp &binaryOp)
{
  binaryOp.getLeft()->accept(*this);
  binaryOp.getRight()->accept(*this);

  if (binaryOp.getOperator() == BinaryOp::EqualAssign && binaryOp.getLeft()->getKind() == Factor::Ident)
  {
    llvm::StringRef assignedVar = binaryOp.getLeft()->getVal();
    setDependencies(assignedVar, binaryOp.getRight());
  }
}

void DeadCodeEliminationVisitor::setDependencies(llvm::StringRef variable, Expr *expr)
{
  lastAssignmentDependencies[variable].clear();
  getDependencies(expr, lastAssignmentDependencies[variable]);
}

void DeadCodeEliminationVisitor::getDependencies(Expr *expr, std::unordered_set<llvm::StringRef> &dependencies)
{
  if (BinaryOp *binaryOp = llvm::dyn_cast<BinaryOp>(expr))
  {
    getDependencies(binaryOp->getLeft(), dependencies);
    getDependencies(binaryOp->getRight(), dependencies);
  }
  else if (Factor *factor = llvm::dyn_cast<Factor>(expr))
  {
    if (factor->getKind() == Factor::Ident)
    {
      dependencies.insert(factor);
      dependencies.insert(lastAssignmentDependencies[factor].begin(), lastAssignmentDependencies[factor].end());
    }
  }
}

void DeadCodeEliminationVisitor::propagateLiveness(llvm::StringRef variable)
{
  if (lastAssignmentDependencies.find(variable) == lastAssignmentDependencies.end() || lastAssignmentDependencies[variable].empty())
  {
    return; 
  }

  for (const auto &dependencyFactor : lastAssignmentDependencies[variable])
  {
    if (dependencyFactor->getKind() == Factor::Ident)
    {
      dependencyFactor->setLive(true);
    }
  }
}

void DeadCodeEliminationVisitor::visit(typeDecl &typeDecl)
{
  if (typeDecl.isLive())
  {
    typeDecl.E->accept(*this);
  }
}
void Sema::performDeadCodeElimination(AST &ast)
{
  DeadCodeEliminationVisitor dceVisitor;
  ast.accept(dceVisitor);
  dceVisitor.propagateLiveness("result");
}
bool Sema::semantic(AST *Tree)
{
  if (!Tree)
    return false;
  DeclCheck Check;
  Tree->accept(Check);
  performDeadCodeElimination(*Tree);
  return Check.hasError();
}
