#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"

class CodeGen
{
  void eliminateDeadCode(Module *M);
  void eliminateDeadVariables(Module *M, ToIRVisitor &ToIR);

public:
 void compile(AST *Tree);

};
#endif
