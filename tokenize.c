#include "mycc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        *p == '{' || *p == '}' || *p == ',' || *p == '&' || *p == '[' ||
        *p == ']') {
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

    if (is_reserve_word(p, "char")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("char"));
      p += strlen("char");
      continue;
    }

    if (is_reserve_word(p, "long")) {
      cur = new_token(TK_RESERVED, cur, p, strlen("long"));
      p += strlen("long");
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
