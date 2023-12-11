#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace
{
  class ToIRVisitor : public ASTVisitor
  {
    Module *M;
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;

    Value *V;
    StringMap<Value *> nameMap;

  public:
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      VoidTy = Type::getVoidTy(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext());
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);
    }

    void run(AST *Tree)
    {
      FunctionType *MainFty = FunctionType::get(
          Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(
          MainFty, GlobalValue::ExternalLinkage, "main", M);
      BasicBlock *BB = BasicBlock::Create(M->getContext(),
                                          "entry", MainFn);
      Builder.SetInsertPoint(BB);

      Tree->accept(*this);

      Builder.CreateRet(Int32Zero);
    }

    virtual void visit(Factor &Node) override
    {
      if (Node.getKind() == Factor::Ident)
      {
        V = nameMap[Node.getVal()];
      }
      else
      {
        int intval;
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

    virtual void visit(BinaryOp &Node) override
    {
      Node.getLeft()->accept(*this);
      Value *Left = V;
      Node.getRight()->accept(*this);
      Value *Right = V;
      switch (Node.getOperator())
      {

      case BinaryOp::Plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
      case BinaryOp::Minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
      case BinaryOp::Mul:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case BinaryOp::Div:
        V = Builder.CreateSDiv(Left, Right);
        break;
      case BinaryOp::Equal:
        V = Builder.CreateStore(Right, Left);
        FunctionType *CalcWriteFnTy =
            FunctionType::get(VoidTy, {Int32Ty}, false);
        Function *CalcWriteFn = Function::Create(
            CalcWriteFnTy, GlobalValue::ExternalLinkage,
            "calc_write", M);
        Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {V});
        break;
      }
    };

    virtual void visit(typeDecl &Node) override
    {
      FunctionType *ReadFty =
          FunctionType::get(Int32Ty, {Int8PtrTy}, false);
      Function *ReadFn = Function::Create(
          ReadFty, GlobalValue::ExternalLinkage, "calc_read",
          M);
      for (auto I = Node.begin(), E = Node.end(); I != E;
           ++I)
      {
        StringRef Var = *I;

        // Create call to calc_read function.
        Constant *StrText = ConstantDataArray::getString(
            M->getContext(), Var);
        GlobalVariable *Str = new GlobalVariable(
            *M, StrText->getType(),
            /*isConstant=*/true, GlobalValue::PrivateLinkage,
            StrText, Twine(Var).concat(".str"));
        Value *Ptr = Builder.CreateInBoundsGEP(
            Str->getType(), Str, {Int32Zero, Int32Zero}, "ptr");
        CallInst *Call =
            Builder.CreateCall(ReadFty, ReadFn, {Ptr});

        nameMap[Var] = Call;
      }
    };
    virtual void visit(IfStatement &ifstmt) override{
      llvm::Value *conditionValue = ifstmt.getCondition()->accept(*this);
      llvm::Value *zeroval = llvm::ConstantInt::get(TheContext , 0);
      llvm::Value *conditionResult = Builder.CreateICmpNE(zeroval , conditionValue , "ifCond");
      llvm::Function *function =Builder.GetInsertBlock()->getParent();
      llvm::BasicBlock *thenBlock = llvm::Create(TheContext , "then" , function);
      llvm::BasicBlock *elseBlock = llvm::Create(TheContext , "else") ;      llvm::BasicBlock *elseBlock = llvm::Create(TheContext , "else" ;
      llvm::BasicBlock *mergeBlock = llvm::Create(TheContext , "ifCond") ;
      Builder.CreateCondBr(conditionResult , thenBlock , elseBlock);
      Builder.SetInsertPoint(thenBlock);
      ifstmt.getThenStmt()->accept(*this);
      Builder.CreateBr(mergeBlock);
      function->getBasicBlockList().push_back(elseBlock);
      Builder.SetInsertPoint(elseBlock);

      for(const auto &elifBlock : ifstmt.getElifStmts()){
        llvm::Value *elifConditionValue = elifBlock.first->accept(*this);
        llvm::Value *elifConditionResult = Builder.CreateICmpNE(elifConditionValue , zeroval , "elifCond");
        llvm::BasicBlock *elifThenBlock = llvm::BasicBlock::Create(TheContext , "elif.then" , function);
        Builder.CreateCondBr(elifConditionResult , elifThenBlock , elseBlock);
        Builder.SetInsertPoint(elifThenBlock);
        elifBlock.second->accept(*this);
        Builder.CreateBr(mergeBlock);

      }
      function->getBasicBlockList().push_back(mergeBlock);
      Builder.SetInsertPoint(mergeBlock);


    }
    virtual void visit(loopcStatement &loopcStmt) override{
      llvm::Function *function =Builder.GetInsertBlock()->getParent();
      llvm::BasicBlock *loopCondBlock = llvm::BasicBlock::Create(Context,"loopCond",function);
      llvm::BasicBlock *loopBodyBlock = llvm::BasicBlock::Create(Context,"loopBody",function);
      llvm::BasicBlock *loopEndBlock = llvm::BasicBlock::Create(Context,"loopEnd",function);
      Builder.CreateBr(loopCondBlock);
      Builder.SetInsertPoint(loopCondBlock);
      llvm::Value *loopCondition = loopcStmt.getCondition()->accept(*this);
      llvm::Value *zeroval = llvm::ConstantInt::get(Int32Ty, 0);
      llvm::Value *loopConditionResult = Builder.CreateICmpNE(zeroval,loopCondition,"loopCond");
      Builder.CreateCondBr(loopConditionResult, loopBodyBlock,loopEndBlock);
      function->getBasicBlockList().push_back(loopBodyBlock);
      Builder.SetInsertPoint(loopBodyBlock);
      loopcStmt.getBody()->accept(*this);
      Builder.CreateBr(loopCondBlock);
      function->getBasicBlockList().push_back(loopEndBlock);
      Builder.SetInsertPoint(loopEndBlock);
    }
  };
} // namespace

void CodeGen::compile(AST *Tree)
{
  LLVMContext Ctx;
  Module *M = new Module("calc.expr", Ctx);
  ToIRVisitor ToIR(M);
  ToIR.run(Tree);
  M->print(outs(), nullptr);
}