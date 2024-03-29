#include "mycc.h"
#include <stdlib.h>

// =======================
// 型
// =======================

// 配列なら1
bool is_array(Type *t) { return t->ty == P_ARRAY; }

// ポインタ型なら1
bool is_pointer(Type *t) { return t->ty == P_PTR; }

// int型を返す
Type *type_int() {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_INT;
  return ty;
}
// char型を返す
Type *type_char() {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_CHAR;
  return ty;
}

// long型を返す
Type *type_long() {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_LONG;
  return ty;
}

// 指定型を示す配列型を返す
Type *array_of(Type *to, int size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->ty = P_ARRAY;
  ty->size = size;
  ty->ptr_to = to;
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
  // 文字列リテラルは文字ポインタ
  if (node->kind == ND_LITERAL) {
    return pointer_to(type_char());
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
int size_of(Type *ty) {
  if (ty->ty == P_PTR) {
    return 8;
  }
  if (ty->ty == P_LONG) {
    return 8;
  }
  if (ty->ty == P_INT) {
    return 4;
  }
  if (ty->ty == P_CHAR) {
    return 1;
  }
  if (ty->ty == P_ARRAY) {
    return ty->size * size_of(ty->ptr_to);
  }
  error("サポートしていない型です");
}
