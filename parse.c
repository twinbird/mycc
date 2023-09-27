#include "mycc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =======================
// 抽象構文木
// =======================

// パース結果のノード
Node *code[100];

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->ty = type_int();
  return node;
}

// primary = num
//         | ident ("(" ((expr ",")* expr)? ")")?
//         | "(" expr ")"
//         | ident
Node *primary() {
  // グルーピングのかっこ
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  // 関数呼び出し
  Token *tok = consume_ident();
  if (tok && consume("(")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FCALL;
    strncpy(node->fname, tok->str, tok->len);
    for (int i = 0; !consume(")"); i++) {
      node->params[i] = expr();
      consume(",");
    }
    return node;
  }

  // 変数
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->ty = lvar->ty;
      node->var = lvar;
    } else {
      error_at(tok->str, "宣言されていない変数です");
    }

    return node;
  }

  // 数値
  return new_node_num(expect_number());
}

// postfix = primary ("[" expr "]")*
Node *postfix() {
  Node *n = primary();
  if (consume("[")) {
    Node *e = expr();
    expect("]");
    n = new_node(ND_DEREF, new_node(ND_ADD, n, e), NULL);
  }
  return n;
}

// unary = "+"? postfix
//       | "-"? postfix
//       | "*" unary
//       | "&" unary
//       | "sizeof" unary
Node *unary() {
  if (consume("+"))
    return postfix();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), postfix());
  if (consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  if (consume("sizeof")) {
    Node *n = unary();
    attach_type(n);
    return new_node_num(size_of(n->ty));
  }
  return postfix();
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    attach_type(node);

    if (consume("+")) {
      if (is_pointer(node->ty) || is_array(node->ty)) {
        // ポインタ演算
        Node *n = new_node_num(size_of(node->ty->ptr_to));
        Node *m = new_node(ND_MUL, n, mul());
        node = new_node(ND_ADD, node, m);
      } else {
        // 通常の演算
        node = new_node(ND_ADD, node, mul());
      }
    } else if (consume("-")) {
      if (is_pointer(node->ty) || is_array(node->ty)) {
        // ポインタ演算
        Node *n = new_node_num(size_of(node->ty->ptr_to));
        Node *m = new_node(ND_MUL, n, mul());
        node = new_node(ND_SUB, node, m);
      } else {
        // 通常の演算
        node = new_node(ND_SUB, node, mul());
      }
    } else {
      return node;
    }
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// assign = equality ("=" asssign)*
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// expr = equality
Node *expr() { return assign(); }

// compound-stmt = "{" stmt* "}"
Node *compound_stmt() {
  if (consume("{")) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    for (int i = 0; !consume("}"); i++) {
      node->stmts[i] = stmt();
    }
    return node;
  }
  return NULL;
}

// type_declare = "int" "*"*
Type *type_specifier() {
  if (!consume("int")) {
    return NULL;
  }
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_INT;
  while (consume("*")) {
    Type *t = calloc(1, sizeof(Type));
    t->ty = P_PTR;
    t->ptr_to = ty;
    ty = t;
  }
  return ty;
}

// array_specifier = ("[" num "]")?
Type *array_specifier(Type *base) {
  if (consume("[")) {
    int n = expect_number();
    base = array_of(base, n);
    expect("]");
  }
  return base;
}

// stmt = expr ";"
//      | "int" "*"* ident ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" stmt* "}"
Node *stmt() {
  Node *node;

  Type *ty = type_specifier();
  if (ty) {
    Token *tok = consume_ident();
    ty = array_specifier(ty);
    node->var = append_locals(tok, ty);
    node->ty = ty;
    expect(";");

    node = calloc(1, sizeof(Node));
    node->kind = ND_VAR_DECLARE;
    return node;
  }

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(";");
    return node;
  }

  if (consume("if")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    node->cond = expr();
    expect(")");
    node->lhs = stmt();
    if (consume("else")) {
      node->rhs = stmt();
    }
    return node;
  }

  if (consume("while")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    node->cond = expr();
    expect(")");
    node->lhs = stmt();
    return node;
  }

  if (consume("for")) {
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    if (!consume(";")) {
      node->init = expr();
      expect(";");
    }
    if (!consume(";")) {
      node->cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      node->post = expr();
      expect(")");
    }
    node->lhs = stmt();
    return node;
  }

  node = compound_stmt();
  if (node) {
    return node;
  }

  node = expr();
  expect(";");
  return node;
}

// function-definition = "int" ident "(" (("int" ident ",")* "int" ident)? ")"
// compound-stmt
Node *function_definition(Token *fname_tok) {
  locals = NULL;

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCTION;
  strncpy(node->fname, fname_tok->str, fname_tok->len);

  for (int i = 0; !consume(")"); i++) {
    Type *ty = type_specifier();
    if (!ty) {
      error("引数の型が未指定です");
    }

    Token *arg_tok = consume_ident();
    Node *arg_node = calloc(1, sizeof(Node));
    arg_node->kind = ND_LVAR;
    arg_node->var = append_locals(arg_tok, ty);
    node->arguments[i] = arg_node;

    consume(",");
  }

  node->lhs = compound_stmt();
  if (!node->lhs) {
    error("関数のブロックが未指定です");
  }

  node->locals = locals;

  return node;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    Type *ty = type_specifier();
    Token *tok = consume_ident();
    if (!ty) {
      error("関数かグローバル変数の型定義が未指定です");
    }
    if (consume("(")) {
      code[i++] = function_definition(tok);
    } else {
      // グローバル変数の処理
      ty = array_specifier(ty);
      append_globals(tok, ty);
      expect(";");
    }
  }
  code[i] = NULL;
}
