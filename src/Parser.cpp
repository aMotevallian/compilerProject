#include "Parser.h"

AST *Parser::parse()
{
  AST *Res = parseCalc();
  expect(Token::eoi);
  return Res;
}

AST *Parser::parseCalc()
{
  if (Tok.is(Token::KW_type))
  {
    llvm::SmallVector<llvm::StringRef, 8> Vars;
    advance();
    if (expect(Token::KW_int))
    {
      goto _error;
    }

    advance();
    if (expect(Token::ident))
      goto _error;
    Vars.push_back(Tok.getText());

    advance();
    while (Tok.is(Token::comma))
    {
      advance();
      if (expect(Token::ident))
        goto _error;
      Vars.push_back(Tok.getText());
      advance();
    }

     if (Tok.is(Token::equal))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::gteq))
    {
        advance();
        E = parseExpr();
    }


    if (Tok.is(Token::lteq))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::lt))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::gt))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::equalAssign))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::notEqual))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::slashEq))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::plusEq))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::minusEq))
    {
        advance();
        E = parseExpr();
    }

    if (Tok.is(Token::starEq))
    {
        advance();
        E = parseExpr();
    }



    if (consume(Token::semi_colon))
      goto _error;

    return new typeDecl(Vars);
  }

  if (Tok.is(Token::ident))
  {

    Expr *Left = new Factor(Factor::Ident, Tok.getText());

    advance();
    if (expect(Token::equal))
      goto _error;

    BinaryOp::Operator Op = BinaryOp::Equal;
    Expr *E;

    advance();
    E = parseExpr();

    if (expect(Token::eoi))
      goto _error;

    Left = new BinaryOp(Op, Left, E);
    return Left;
  }
  if (Tok.is(Token::KW_if)){

    advance();
    Expr *Condition = parseExpr();
    if(expect(Token::colon)){
      goto _error;
    }
    advance();
    if(expect(Token::KW_begin)){
      goto _error;
    }
    advance();
    AST *Thenstmt = nullptr;
    llvm::SmallVector<std::pair<Expr *,AST *>,8> ElifStmts;
    AST *ElseStmt = nullptr;
    Thenstmt = parseCalc();
    if(Thenstmt == nullptr)
      goto _error;
    if(expect(Token::KW_end))
      goto _error;
    advance();
    if(Tok.is(Token::KW_elif)){
      advance();
      Expr *ElifCondition = parseExpr();
      if(expect(Token::colon))
        goto _error;
      advance();
      if(expect(Token::KW_begin))
        goto _error;
      advance();
      AST * ElifStmt = parseCalc();
      if(ElifStmt == nullptr)
        goto _error;
      ElifStmts.push_back({ElifCondition , ElifStmt});
      if(expect(Token::KW_end))
        goto _error;
      advance();
    }
    if(Tok.is(Token::KW_else)){

      advance();
      if(expect(Token::colon))
        goto _error;
      advance();
      if(expect(Token::KW_begin))
        goto _error;
      advance();
      ElseStmt = parseCalc();
      if(expect(Token::KW_end))
        goto _error;
      advance();
    }
    return new IfStatement(Condition, Thenstmt , ElifStmts , ElseStmt);
  }
  if (Tok.is(Token::KW_loopc)){
    advance();
    Expr *Condition = parseExpr();
    if(expect(Token::colon)){
      goto _error;
    }
    advance();
    if(expect(Token::KW_begin)){
      goto _error;
    }
    advance();
    llvm::SmallVector<Expr *, 8> LoopBody;
    while (!Token.is(Token::KW_end) && !Token.is(Token::eoi)){
      Expr *Expr = parseExpr();
      if(!Expr){
        goto _error;
      }
      LoopBody.push_back(Expr);
    }
    if (consume(Token::KW_end))
    {
      return new loopcStatement(Condition, LoopBody)
    }
    else{
      goto _error;
    }

  }
_error:
  while (Tok.getKind() != Token::eoi)
    advance();
  return nullptr;
}

Expr *Parser::parseExpr()
{
  Expr *Left = parseTerm();
  while (Tok.isOneOf(Token::plus, Token::minus))
  {
    BinaryOp::Operator Op = Tok.is(Token::plus)
                                ? BinaryOp::Plus
                                : BinaryOp::Minus;
    advance();
    Expr *Right = parseTerm();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

Expr *Parser::parseTerm()
{
  Expr *Left = parseFactor();
  while (Tok.isOneOf(Token::star, Token::slash , Token::mod))
  {
    BinaryOp::Operator Op =
        Tok.is(Token::star) ? BinaryOp::Mul : (Token::slash ? BinaryOp::Div : BinaryOp::Mod);
    advance();
    Expr *Right = parseFactor();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

Expr *Parser::parseFactor()
{
  Expr *Res = nullptr;
  switch (Tok.getKind())
  {
  case Token::number:
    Res = new Factor(Factor::Number, Tok.getText());
    advance();
    break;
  case Token::ident:
    Res = new Factor(Factor::Ident, Tok.getText());
    advance();
    break;
  case Token::l_paren:
    advance();
    Res = parseExpr();
    if (!consume(Token::r_paren))
      break;
  default:
    if (!Res)
      error();
    while (!Tok.isOneOf(Token::r_paren, Token::star,
                        Token::plus, Token::minus,
                        Token::slash, Token::equal, Token::eoi))
      advance();
  }
  return Res;
}
Expr *Parser::parsePower() {
  Expr *Left = parseFactor();
  while (Tok.is(Token::power)) {
    advance();
    Expr *Right = parseFactor();
    Left = new BinaryOp(BinaryOp::Power, Left, Right);
  }
  return Left;
}
Expr *Parser::parseRelational() {
  Expr *Left = parseExpr();
  while (Tok.isOneOf(Token::lt, Token::gt, Token::gteq, Token::lteq, Token::isEqual, Token::notEqual)) {
    BinaryOp::Operator Op;
    switch (Tok.getKind()) {
      case Token::lt:
        Op = BinaryOp::Less;
        break;
      case Token::gt:
        Op = BinaryOp::Greater;
        break;
      case Token::gteq:
        Op = BinaryOp::GreaterEqual;
        break;
      case Token::lteq:
        Op = BinaryOp::LessEqual;
        break;
      case Token::isEqual:
        Op = BinaryOp::Equal;
        break;
      case Token::notEqual:
        Op = BinaryOp::NotEqual;
        break;
      default:
        error();
        return nullptr;
    }
    advance();
    Expr *Right = parseExpr();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

Expr *Parser::parseLogical() {
  Expr *Left = parseRelational();
  while (Tok.isOneOf(Token::logicalAnd, Token::logicalOr)) {
    BinaryOp::Operator Op;
    switch (Tok.getKind()) {
      case Token::logicalAnd:
        Op = BinaryOp::LogicalAnd;
        break;
      case Token::logicalOr:
        Op = BinaryOp::LogicalOr;
        break;
      default:
        error();
        return nullptr;
    }
    advance();
    Expr *Right = parseRelational();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

Expr *Parser::parseAssignment() {
  Expr *Left = parseLogical();
  while (Tok.isOneOf(Token::equalAssign, Token::minusEq, Token::slashEq, Token::starEq, Token::plusEq)) {
    BinaryOp::Operator Op;
    switch (Tok.getKind()) {
      case Token::equalAssign:
        Op = BinaryOp::EqualAssign;
        break;
      case Token::minusEq:
        Op = BinaryOp::MinusEqual;
        break;
      case Token::slashEq:
        Op = BinaryOp::SlashEqual;
        break;
      case Token::starEq:
        Op = BinaryOp::StarEqual;
        break;
      case Token::plusEq:
        Op = BinaryOp::PlusEqual;
        break;
      default:
        error();
        return nullptr;
    }
    advance();
    Expr *Right = parseLogical();
    Left = new BinaryOp(Op, Left, Right);
  }
  return Left;
}

