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
  while (Tok.isOneOf(Token::star, Token::slash))
  {
    BinaryOp::Operator Op =
        Tok.is(Token::star) ? BinaryOp::Mul : BinaryOp::Div;
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
