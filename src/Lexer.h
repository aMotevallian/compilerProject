#ifndef LEXER_H
#define LEXER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/MemoryBuffer.h"

class Lexer;

class Token
{
  friend class Lexer;

public:
  enum TokenKind : unsigned short
  {
    eoi,
    unknown,
    ident,
    number,
    comma,
    colon, //:
    semi_colon,
    plus,
    minus,
    equal,
    star,
    slash,
    l_paren,
    r_paren,
    KW_type,
    KW_int,
    KW_if,
    KW_end,
    KW_begin,
    KW_elif,
    KW_else,
    KW_loopc,
    mod, //%
    power, //^
    lt, //<
    gt, //>
    gteq, //>=
    lteq, //<=
    isEqual, //==
    notEqual, //!=
    logicalAnd, //and
    logicalOr,//or
    minusEq, //-=
    equalAssign, // =
    slashEq, // /=
    plusEq // +=
    starEq,// *=
  };

private:
  TokenKind Kind;
  llvm::StringRef Text;

public:
  TokenKind getKind() const { return Kind; }
  llvm::StringRef getText() const
  {
    return Text;
  }

  bool is(TokenKind K) const { return Kind == K; }
  bool isOneOf(TokenKind K1, TokenKind K2) const
  {
    return is(K1) || is(K2);
  }
  template <typename... Ts>
  bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks) const
  {
    return is(K1) || isOneOf(K2, Ks...);
  }
};

class Lexer {
  const char *BufferStart;
  const char *BufferPtr;

public:
  Lexer(const llvm::StringRef &Buffer) {
    BufferStart = Buffer.begin();
    BufferPtr = BufferStart;
  }

  void next(Token &token);

private:
  void formToken(Token &Result, const char *TokEnd,
                 Token::TokenKind Kind);
};
#endif
