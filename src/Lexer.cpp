#include "Lexer.h"

namespace charinfo {
LLVM_READNONE inline bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
         c == '\r' || c == '\n';
}

LLVM_READNONE inline bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

LLVM_READNONE inline bool isLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
void Lexer::next(Token &token)
{
  while (*BufferPtr && charinfo::isWhitespace(*BufferPtr))
  {
    ++BufferPtr;
  }
  if (!*BufferPtr)
  {
    token.Kind = Token::eoi;
    return;
  }
  if (charinfo::isLetter(*BufferPtr))
  {
    const char *end = BufferPtr + 1;
    while (charinfo::isLetter(*end))
      ++end;
    llvm::StringRef Name(BufferPtr, end - BufferPtr);
    Token::TokenKind kind ;

    if (Name == "type")
    {
      kind = Token::KW_type;
    }else if (Name == "int")
    {
      kind = Token::KW_int;
    }else if (Name == "if")
    {
      kind = Token::KW_if;
    }else if (Name == "end")
    {
      kind = Token::KW_end;
    }else if (Name == "begin")
    {
      kind = Token::KW_begin;
    }else if (Name == "elif")
    {
      kind = Token::KW_elif;
    }else if (Name == "else")
    {
      kind = Token::KW_else;
    }else if (Name == "loopc")
    {
      kind = Token::KW_loopc;
    }else
    {
      kind = Token::ident;
    }

    // Token::TokenKind kind =
    //     Name == "type" ? Token::KW_type : (Name == "int" ? Token::KW_int : Token::ident);
    formToken(token, end, kind);
    return;
  }
  else if (charinfo::isDigit(*BufferPtr))
  {
    const char *end = BufferPtr + 1;
    while (charinfo::isDigit(*end))
      ++end;
    formToken(token, end, Token::number);
    return;
  }
  else
  {
    switch (*BufferPtr) {
#define CASE(ch, tok) \
case ch: formToken(token, BufferPtr + 1, tok); break


CASE('(', Token::Token::l_paren);
CASE(')', Token::Token::r_paren);
CASE('^', Token::Token::power);
CASE('*', Token::star);
CASE('/', Token::slash);
CASE('%', Token::Token::mod);
CASE('+', Token::plus);
CASE('-', Token::minus);
CASE('>', Token::Token::gt);
CASE('<', Token::Token::lt);
CASE('>=', Token::Token::gteq);
CASE('<=', Token::Token::lteq);
CASE('==', Token::Token::isEqual);
CASE('!=', Token::Token::notEqual);
CASE('and', Token::Token::logicalAnd);
CASE('or', Token::Token::logicalOr);
CASE('-=', Token::Token::minusEq);
CASE('/=', Token::Token::slashEq);
CASE('*=', Token::Token::starEq);
CASE('+=', Token::Token::plusEq);
CASE('=', Token::Token::equalAssign);
CASE(':', Token::Token::colon);
CASE(',', Token::Token::comma);
CASE(';', Token::Token::eoi);



#undef CASE
    default:
      formToken(token, BufferPtr + 1, Token::unknown);
    }
    return;
  }
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
  Tok.Kind = Kind;
  Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
  BufferPtr = TokEnd;
}
