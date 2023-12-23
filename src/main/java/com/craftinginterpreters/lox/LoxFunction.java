package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
  private final Stmt.Function declaration;

  LoxFunction(Stmt.Function declaration) {
    this.declaration = declaration;
  }

  @Override
  public Object call(Interpreter interpreter, List<Object> arguments) {
    Environment environment = new Environment(interpreter.globals);
    for (int i = 0; i < arity(); i++) {
      environment.define(
        declaration.params.get(i).lexeme,
        arguments.get(i),
        true
      );
    }

    interpreter.executeBlock(declaration.body , environment);
    return null;
  }

  @Override
  public int arity() {
    return declaration.params.size();
  }

  @Override
  public String toString() {
    return "<fn " + declaration.name.lexeme + ">";
  }
}
