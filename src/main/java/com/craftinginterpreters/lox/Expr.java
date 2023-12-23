package com.craftinginterpreters.lox;

import java.util.List;

abstract class Expr {
  interface Visitor<R> {
    R visitBinaryExpr(Binary expr);
    R visitGroupingExpr(Grouping expr);
    R visitLiteralExpr(Literal expr);
    R visitUnaryExpr(Unary expr);
    R visitVariableExpr(Variable expr);
    R visitAssignExpr(Assign expr);
    R visitLogicalExpr(Logical expr);
    R visitCallExpr(Call expr);
  }
  static class Binary extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitBinaryExpr(this);
    }

    Binary(Expr left, Token operator, Expr right) {
      this.left = left;
      this.operator = operator;
      this.right = right;
    }

    final Expr left;
    final Token operator;
    final Expr right;
  }
  static class Grouping extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitGroupingExpr(this);
    }

    Grouping(Expr expression) {
      this.expression = expression;
    }

    final Expr expression;
  }
  static class Literal extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitLiteralExpr(this);
    }

    Literal(Object value) {
      this.value = value;
    }

    final Object value;
  }
  static class Unary extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitUnaryExpr(this);
    }

    Unary(Token operator, Expr right) {
      this.operator = operator;
      this.right = right;
    }

    final Token operator;
    final Expr right;
  }
  static class Variable extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitVariableExpr(this);
    }

    Variable(Token name) {
      this.name = name;
    }

    final Token name;
  }
  static class Assign extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitAssignExpr(this);
    }

    Assign(Token name, Expr value) {
      this.name = name;
      this.value = value;
    }

    final Token name;
    final Expr value;
  }
  static class Logical extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitLogicalExpr(this);
    }

    Logical(Expr left, Token operator, Expr right) {
      this.left = left;
      this.operator = operator;
      this.right = right;
    }

    final Expr left;
    final Token operator;
    final Expr right;
  }
  static class Call extends Expr {
    @Override
    <R> R accept(Visitor<R> visitor) {
      return visitor.visitCallExpr(this);
    }

    Call(Expr callee, Token paren, List<Expr> arguments) {
      this.callee = callee;
      this.paren = paren;
      this.arguments = arguments;
    }

    final Expr callee;
    final Token paren;
    final List<Expr> arguments;
  }

  abstract <R> R accept(Visitor<R> visitor);
}
