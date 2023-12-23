package com.craftinginterpreters.lox;

import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import static com.craftinginterpreters.lox.TokenType.*;

class Parser {
  private static class ParseError extends RuntimeException {}
  private static class ParseErrorBinary extends RuntimeException {}

  private final List<Token> tokens;
  private int current = 0;
  private Boolean insideWhile = false;

  Parser(List<Token> tokens) {
    this.tokens = tokens;
  }

  private boolean match(TokenType... tokens) {
    for (TokenType type : tokens) {
      if (check(type)) {
        // System.out.println(peek());
        advance();
        return true;
      }
    }
    return false;
  }

  private Token peek() {
    return tokens.get(current);
  }

  private Token previous() {
    return tokens.get(current - 1);
  }

  private boolean check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
  }

  private Token advance() {
    if (!isAtEnd()) current++;
    return previous();
  }

  private boolean isAtEnd() {
    return peek().type == EOF;
  }

  private Token consume(TokenType type, String message) {
    if (check(type)) return advance();
    throw error(peek(), message);
  }

  private ParseError error(Token token, String message) {
    Lox.error(token, message);
    return new ParseError();
  }

  private void synchronize() {
    advance();

    while (!isAtEnd()) {
      if (previous().type == SEMICOLON) return;

      switch (peek().type) {
        case CLASS: case FOR: case FUN: case IF: case PRINT:
	case RETURN: case VAR: case WHILE:
        return;
      }

      advance();
    }
  }

  private Stmt declaration() {
    try {
      if (match(VAR)) return varDeclaration(false);
      return statement();
    } catch (ParseError error) {
      synchronize();
      return null;
    }
  }

  private Stmt varDeclaration(Boolean requireInitializer) {
    Token identifier = consume(IDENTIFIER, "Expect identifier in variable declaration.");

    Expr initializer = null;
    if (match(EQUAL)) {
      initializer = expression();
    } else if (requireInitializer) {
      throw error(peek(), "Expect '=' in for-loop variable declaration.");
    }

    consume(SEMICOLON, "Expect ';' after identifier.");
    return new Stmt.Var(identifier, initializer);
  }

  private Stmt statement() {
    if (match(PRINT)) return printStatement();
    if (match(LEFT_BRACE)) return new Stmt.Block(block());
    if (match(IF)) return ifStatement();
    if (match(WHILE)) return whileStatement();
    if (match(FOR)) return forStatement();
    if (match(BREAK)) return breakStatement();
    if (match(FUN)) return function("function");

    return expressionStatement();
  }

  private Stmt.Function function(String kind) {
    Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
    consume(LEFT_PAREN, "Expect '(' after function name.");

    List<Token> params = new ArrayList<>();
    if (!check(RIGHT_PAREN)) {
      do {
        if (params.size() >= 255) {
          error(peek(), "Cant have more than 255 prameters.");
        }
        params.add(
          consume(IDENTIFIER, "Function parameter must be an identifier.")
        );
      } while (match(COMMA));
    }

    consume(RIGHT_PAREN, "Expect ')' after parameters.");
    consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
    List<Stmt> body = block();
    return new Stmt.Function(name, params, body);
  }

  private Stmt.Break breakStatement() {
    if (insideWhile) {
      consume(SEMICOLON, "Expect ';' after break.");
      return new Stmt.Break();
    }

    throw error(previous(), "Break statement must be inside a loop.");
  }

  private Stmt forStatement() {
    consume(LEFT_PAREN, "Expect '(' after for statement.");

    Stmt initializer;
    if (match(SEMICOLON)) {
      initializer = null;
    } else if (match(VAR)) {
      initializer = varDeclaration(true);
    } else {
      initializer = expressionStatement();
    }

    Expr condition = null;
    if (!check(SEMICOLON)) {
      condition = expression();
    }

    consume(SEMICOLON, "Expect ';' after loop condition .");

    Expr increment = null;
    if (!check(SEMICOLON)) {
      increment = expression();
    }

    consume(RIGHT_PAREN, "Expect ')' after loop increment.");

    Stmt body;
    if (insideWhile) {
      body = statement();
    } else {
      insideWhile = true;
      body = statement();
      insideWhile = false;
    }

    if (increment != null) {
      body = new Stmt.Block(
        Arrays.asList(
          body,
          new Stmt.Expression(increment)
        )
      );
    }

    if (condition == null) condition = new Expr.Literal(true);
    body = new Stmt.While(condition, body);

    if (initializer != null) {
      body = new Stmt.Block(Arrays.asList(initializer, body
));
    }

    return body;
  }

  private Stmt.While whileStatement() {
    consume(LEFT_PAREN, "Expect '(' after while statement.");
    Expr condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after expression.");

    Stmt body;
    if (insideWhile) {
      body = statement();
    } else {
      insideWhile = true;
      body = statement();
      insideWhile = false;
    }

    return new Stmt.While(condition, body);
  }

  private Stmt.If ifStatement() {
    consume(LEFT_PAREN, "Expect '(' after if statement.");
    Expr condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after expression.");
    Stmt thenBranch = statement();

    Stmt elseBranch = null;
    if (match(ELSE)) {
      elseBranch = statement();
      return new Stmt.If(condition, thenBranch, elseBranch);
    }

    return new Stmt.If(condition, thenBranch, elseBranch);
  }

  private List<Stmt> block() {
    List<Stmt> statements = new ArrayList<>();
    while(!check(RIGHT_BRACE) && !isAtEnd()) {
      statements.add(declaration());
    }
    consume(RIGHT_BRACE, "Expect '}' after block.");
    return statements;
  }

  private Stmt.Print printStatement() {
    Expr value = expression();
    consume(SEMICOLON, "Expect ';' after value.");
    return new Stmt.Print(value);
  }

  private Stmt.Expression expressionStatement() {
    Expr expr = expression();
    consume(SEMICOLON, "Expect ';' after expression.");
    return new Stmt.Expression(expr);
  }

  private Expr expression() {
    return comma();
  }

  private Expr comma() {
    Expr expr = assignment();
    while (match(COMMA)) {
      Token operator = previous();
      Expr right = assignment();
      expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
  }

  private Expr assignment() {
    Expr expr = or();

    if (match(EQUAL)) {
      Token equals = previous();
      Expr value = assignment();

      if (expr instanceof Expr.Variable) {
        Token name = ((Expr.Variable)expr).name;
        return new Expr.Assign(name, value);
      }

      throw error(equals, "Invalid assignment target.");
    }

    return expr;
  }

  private Expr or() {
    Expr expr = and();

    while (match(OR)) {
      Token operator = previous();
      Expr right = and();
      expr = new Expr.Logical(expr, operator, right);
    }

    return expr;
  }

  private Expr and() {
    Expr expr = ternary();

    while (match(AND)) {
      Token operator = previous();
      Expr right = ternary();
      expr = new Expr.Logical(expr, operator, right);
    }

    return expr;
  }

  private Expr ternary() {
    Expr expr = equality();
    if (match(QUESTION_MARK)) {
      Token questionmark = previous();
      Expr middle = equality();

      consume(DOUBLE_DOT, "Missing ':' in ternary operator.");

      expr = new Expr.Binary(expr, questionmark, middle);

      Token doudleComma = previous();
      Expr right = equality();
      expr = new Expr.Binary(expr, doudleComma, right);
    }
    return expr;
  }

  private Expr equality() {
    Expr expr = comparison();
    while (match(BANG_EQUAL, EQUAL_EQUAL)) {
      Token operator = previous();
      Expr right = comparison();
      expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
  }

  private Expr comparison() {
    Expr expr = term();
    while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
      Token operator = previous();
      Expr right = term();
      expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
  }

  private Expr term() {
    Expr expr = factor();
    while (match(MINUS, PLUS)) {
      Token operator = previous();
      Expr right = factor();
      expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
  }

  private Expr factor() {
    Expr expr = unary();
    while (match(SLASH, STAR)) {
      Token operator = previous();
      Expr right = unary();
      expr = new Expr.Binary(expr, operator, right);
    }
    return expr;
  }

  private Expr unary() {
    if (match(BANG, MINUS)) {
      Token operator = previous();
      Expr right = unary();
      return new Expr.Unary(operator, right);
    }
    return call();
  }

  private Expr call() {
    Expr expr = primary();

    while (true) {
      if (match(LEFT_PAREN)) {
        expr = finishCall(expr);
      } else {
        break;
      }
    }

    return expr;
  }

  private Expr.Call finishCall(Expr expr) {
    List<Expr> arguments = new ArrayList<>();
    if (!check(RIGHT_PAREN)) {
      do {
        if (arguments.size() >= 255) {
          error(peek(), "Cant have more than 255 arguments.");
        }
        // assignment because bybass comma
        arguments.add(assignment());
      } while (match(COMMA));
    }
    Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
    return new Expr.Call(expr, paren, arguments);
  }

  private Expr primary() {
    if (match(FALSE)) return new Expr.Literal(false);
    if (match(TRUE)) return new Expr.Literal(true);
    if (match(NIL)) return new Expr.Literal(null);

    if (match(IDENTIFIER)) {
      return new Expr.Variable(previous());
    }

    if (match(NUMBER, STRING)) {
      return new Expr.Literal(previous().literal);
    }

    if (match(LEFT_PAREN)) {
      Expr expr = expression();
      consume(RIGHT_PAREN, "Expect ')' after expression.");
      return new Expr.Grouping(expr);
    }

    // binary operator in wrong place
    if (match(PLUS, MINUS, DOT, SLASH, GREATER, GREATER_EQUAL, LESS, LESS_EQUAL, EQUAL_EQUAL)) {
      Lox.error(previous(), "Binary operator cannot start an expression.");
      // parse rest
      equality();
      throw new ParseError();
    }

    throw error(peek(), "Expect expression.");
  }

  List<Stmt> parse() {
    List<Stmt> statements = new ArrayList<>();
    while (!isAtEnd()) {
      statements.add(declaration());
    }

    return statements;
  }
}
