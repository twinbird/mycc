#include "mycc.h"
#include <stdlib.h>
#include <string.h>

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

// ローカル変数へ引数のトークンを加える
LVar *append_locals(Token *tok, Type *ty) {
  LVar *lvar;
  lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->ty = ty;
  locals = lvar;

  return lvar;
}

// =======================
// グローバル変数
// =======================
// グローバル変数のリスト先頭へのポインタ
GVar *globals;

GVar *find_gvar(Token *tok) {
  for (GVar *var = globals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

// グローバル変数へ引数のトークンを加える
GVar *append_globals(Token *tok, Type *ty) {
  GVar *gvar;
  gvar = calloc(1, sizeof(GVar));
  gvar->next = globals;
  gvar->name = tok->str;
  gvar->len = tok->len;
  gvar->ty = ty;
  globals = gvar;

  return gvar;
}

// =======================
// 文字列リテラル
// =======================
// 文字列リテラルのリスト先頭へのポインタ
Literal *literals;

// 引数のトークンのリテラルを探す
Literal *find_literal(Token *tok) {
  for (Literal *lit = literals; lit; lit = lit->next)
    if (lit->tok->len == tok->len && !memcmp(tok->str, lit->tok->str, tok->len))
      return lit;
  return NULL;
}

// 文字列リテラルのリストへ引数のトークンを加える
Literal *append_literals(Token *tok) {
  Literal *lit;
  lit = calloc(1, sizeof(Literal));
  lit->next = literals;
  lit->tok = tok;
  literals = lit;

  return lit;
}
