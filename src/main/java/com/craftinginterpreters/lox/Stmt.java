package com.craftinginterpreters.lox;

import java.util.List;

abstract class Stmt {
  interface Visitor<R> {
    R visitExpressionStmt(Expression stmt);
    R visitPrintStmt(Print stmt);
    R visitVarStmt(Var stmt);
    R visitBlockStmt(Block stmt);
    R visitIfStmt(If stmt);
    R visitWhileStmt(While stmt);
    R visitBreakStmt(Break stmt);
    R visitFunctionStmt(Function stmt);
    R visitReturnStmt(Return stmt);
  }
  static class Expression extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitExpressionStmt(this);
    }

    Expression(Expr expression) {
      this.expression = expression;
    }

    final Expr expression;
  }
  static class Print extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitPrintStmt(this);
    }

    Print(Expr expression) {
      this.expression = expression;
    }

    final Expr expression;
  }
  static class Var extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitVarStmt(this);
    }

    Var(Token name, Expr initializer) {
      this.name = name;
      this.initializer = initializer;
    }

    final Token name;
    final Expr initializer;
  }
  static class Block extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitBlockStmt(this);
    }

    Block(List<Stmt> statements) {
      this.statements = statements;
    }

    final List<Stmt> statements;
  }
  static class If extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitIfStmt(this);
    }

    If(Expr condition, Stmt thenBranch, Stmt elseBranch) {
      this.condition = condition;
      this.thenBranch = thenBranch;
      this.elseBranch = elseBranch;
    }

    final Expr condition;
    final Stmt thenBranch;
    final Stmt elseBranch;
  }
  static class While extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitWhileStmt(this);
    }

    While(Expr condition, Stmt body) {
      this.condition = condition;
      this.body = body;
    }

    final Expr condition;
    final Stmt body;
  }
  static class Break extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitBreakStmt(this);
    }
  }
  static class Function extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitFunctionStmt(this);
    }

    Function(Token name, List<Token> params, List<Stmt> body) {
      this.name = name;
      this.params = params;
      this.body = body;
    }

    final Token name;
    final List<Token> params;
    final List<Stmt> body;
  }
  static class Return extends Stmt {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitReturnStmt(this);
    }

    Return(Token keyword, Expr value) {
      this.keyword = keyword;
      this.value = value;
    }

    final Token keyword;
    final Expr value;
  }

  abstract <R> R accept(Visitor<R> visitor);
}
