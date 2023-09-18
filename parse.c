#include "mycc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===========================
// エラー表示
// ===========================
// 入力プログラム
char *user_input;

// エラー個所とエラーを報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラーを報告する関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// ====================
// トークナイザ
// ====================
// 現在着目しているトークン
Token *token;

// 次のトークンが期待している記号の時には、トークンを1つ読み進めて
// trueを返す。それ以外の時にはfalseを返す。
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

// 現在着目しているトークンが識別子ならそれを返す
// そうでなければNULLを返す
Token *consume_ident() {
  if (token->kind == TK_IDENT) {
    Token *ret = token;
    token = token->next;
    return ret;
  }
  return NULL;
}

// 次のトークンが期待している記号の時には、トークンを1つ読み進める。
// それ以外の時にはエラーを報告する。
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成してcurへつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) { return memcmp(p, q, strlen(q)) == 0; }

bool is_ident_first_char(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

bool is_ident_char(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

bool is_reserve_word(char *p, char *word) {
  int len = strlen(word);
  if (strncmp(p, word, len) == 0 && !is_ident_char(p[len])) {
    return true;
  }
  return false;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
        startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' ||
        *p == '{' || *p == '}' || *p == ',' || *p == '&') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (is_reserve_word(p, "return")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("return"));
      p += strlen("return");
      continue;
    }

    if (is_reserve_word(p, "sizeof")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("sizeof"));
      p += strlen("sizeof");
      continue;
    }

    if (is_reserve_word(p, "if")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("if"));
      p += strlen("if");
      continue;
    }

    if (is_reserve_word(p, "else")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("else"));
      p += strlen("else");
      continue;
    }

    if (is_reserve_word(p, "while")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("while"));
      p += strlen("while");
      continue;
    }

    if (is_reserve_word(p, "for")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("for"));
      p += strlen("for");
      continue;
    }

    if (is_reserve_word(p, "int")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("int"));
      p += strlen("int");
      continue;
    }

    if (is_ident_first_char(*p)) {
      int n = 0;
      char *q = p;
      for (n = 0; isalnum(*p) || *p == '_'; n++)
        p++;
      cur = new_token(TK_IDENT, cur, q, n);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// =======================
// 型
// =======================

// int型を返す
Type *type_int() {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_INT;
  return ty;
}

// 指定型を示すポインタ型を返す
Type *pointer_to(Type *to) {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_PTR;
  ty->ptr_to = to;
  return ty;
}

// node以下に型を付与して、付与した型を返す
Type *attach_type(Node *node) {
  // 関数呼び出しの場合は戻り値の型
  if (node->kind == ND_FCALL) {
    // 暫定で全部int
    node->ty = type_int();
    return node->ty;
  }
  // デリファレンスの場合は参照した先の型を付与
  if (node->kind == ND_DEREF) {
    Type *t = attach_type(node->lhs);
    node->ty = t->ptr_to;
    return node->ty;
  }
  // アドレス参照の場合はポインタを付与
  if (node->kind == ND_ADDR) {
    Type *t = attach_type(node->lhs);
    node->ty = pointer_to(t);
    return node->ty;
  }
  // 型がついているものはプリミティブ型
  if (node->ty) {
    return node->ty;
  }

  // その他は下位の型を付与
  node->ty = attach_type(node->lhs);
  return node->ty;
}

// 引数のノードの型のサイズを返す
int size_of(Node *node) {
  if (node->ty->ty == P_PTR) {
    return 8;
  }
  if (node->ty->ty == P_INT) {
    return 4;
  }
  error("サポートしていない型です");
}

// =======================
// ローカル変数
// =======================
// ローカル変数のリスト先頭へのポインタ
LVar *locals;

// ローカル変数を探す
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

void debug_print_lvar() {
  fprintf(stderr, "---------------------------\n");
  fprintf(stderr, "DEBUG\n");
  for (LVar *var = locals; var; var = var->next)
    fprintf(stderr, "%.*s\n", var->len, var->name);
  fprintf(stderr, "---------------------------\n");
}

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

// ローカル変数へ引数のトークンを加えてoffset値を返す
int append_locals(Token *tok, Type *ty) {
  LVar *lvar;
  lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->offset = locals == NULL ? 8 : locals->offset + 8;
  lvar->ty = ty;
  locals = lvar;

  return lvar->offset;
}

// primary = num
//         | ident ("(" ((expr ",")* expr)? ")")?
//         | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

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
  } else if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
      node->ty = lvar->ty;
    } else {
      error_at(tok->str, "宣言されていない変数です");
    }

    return node;
  }

  return new_node_num(expect_number());
}

// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
//       | "sizeof" unary
Node *unary() {
  if (consume("+"))
    return primary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  if (consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if (consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  if (consume("sizeof")) {
    Node *n = unary();
    attach_type(n);
    return new_node_num(size_of(n));
  }
  return primary();
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
  attach_type(node);

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
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
Type *type_declare() {
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

// stmt = expr ";"
//      | "int" "*"* ident ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "{" stmt* "}"
Node *stmt() {
  Node *node;

  Type *ty = type_declare();
  if (ty) {
    Token *tok = consume_ident();
    node->offset = append_locals(tok, ty);
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

// function-definition = "int" ident "(" (("int" ident ",")* "int" ident)? ")" compound-stmt
Node *function_definition() {
  expect("int");

  Token *tok = consume_ident();
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FUNCTION;
  strncpy(node->fname, tok->str, tok->len);

  expect("(");

  for (int i = 0; !consume(")"); i++) {
    Type *ty = type_declare();
    if (!ty) {
      error("引数の型が未指定です");
    }

    Token *arg_tok = consume_ident();
    int offset = append_locals(arg_tok, ty);
    Node *arg_node = calloc(1, sizeof(Node));
    arg_node->kind = ND_LVAR;
    arg_node->offset = offset;
    node->arguments[i] = arg_node;

    consume(",");
  }

  node->lhs = compound_stmt();
  if (!node->lhs) {
    error("関数のブロックが未指定です");
  }

  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = function_definition();
  code[i] = NULL;
}
