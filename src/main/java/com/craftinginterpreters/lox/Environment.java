package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
  final Environment enclosing;
  private final Map<String, Object> values = new HashMap<>();
  private final Map<String, Boolean> assigned = new HashMap<>();

  Environment() {
    enclosing = null;
  }

  Environment(Environment enclosing) {
    this.enclosing = enclosing;
  }

  void assign(Token name, Object value) {
    if (values.containsKey(name.lexeme)) {
      values.put(name.lexeme, value);
      assigned.put(name.lexeme, true);
      return;
    }

    if (enclosing != null) {
      enclosing.assign(name, value);
      return;
    }

    throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }

  void define(String name, Object value, Boolean assigned) {
    values.put(name, value);
    this.assigned.put(name, assigned);
  }

  Object get(Token name) {
    if (values.containsKey(name.lexeme)) {
      if (assigned.get(name.lexeme)) {
        return values.get(name.lexeme);
      }

      throw new RuntimeError(name, "Variable '" + name.lexeme + "' evaluated before assignment.");
    }

    if (enclosing != null) return enclosing.get(name);

    throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }
}
