# Compiler-Project-P1

Grammer :
Goal           -> statement*
statement         -> typeDecl | assignment | ifStatement | loopStatement | expr semi_colon
typeDecl          -> KW_type KW_int ident typeDeclRest
typeDeclRest      -> (comma ident typeDeclRest)? equal assign expr
assignment        -> ident assignmentRest
assignmentRest    -> (equal expr | minusEq expr | slashEq expr | starEq expr | plusEq expr)?
ifStatement       -> KW_if expr KW_then statement ifRest
ifRest            -> (KW_elif expr KW_then statement)* (KW_else statement)?
loopStatement     -> KW_loopc expr KW_do statement*
expr              -> logicalExpr
logicalExpr       -> relationalExpr logicalExprRest
logicalExprRest   -> logicalOr relationalExpr logicalExprRest | ε
relationalExpr    -> addExpr relationalExprRest
relationalExprRest -> (gt addExpr | lt addExpr | gteq addExpr | lteq addExpr | isEqual addExpr | notEqual addExpr)?
addExpr           -> term addExprRest
addExprRest       -> (plus term addExprRest | minus term addExprRest)? 
term              -> factor termRest
termRest          -> (star factor termRest | slash factor termRest | mod factor termRest)? 
factor            -> number | ident | l_paren expr r_paren





