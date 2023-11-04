#define _XOPEN_SOURCE 700
#include "mycc.h"
#include <stdio.h>
#include <string.h>

// =============================
// コードジェネレータ
// =============================

// 分岐ラベルにつける数値
int branch_label_counter;

// デバッグアセンブリを挿入
void comment_gen(char *str) { printf("#%s\n", str); }

// 指定リテラルノードのリテラルへのアドレスをスタックへ積む
void gen_literal_addr(Node *node) {
  if (node->kind != ND_LITERAL) {
    error("リテラル以外のノードがリテラルとして到達した");
  }
  printf("  mov rax, OFFSET FLAT:.LC%d\n", node->literal->n);
  printf("  push rax\n");
}

// 指定nodeの変数へのアドレスをスタックに積む
void gen_var_addr(Node *node) {
  if (node->kind != ND_LVAR && node->kind != ND_GVAR) {
    error("代入の左辺値が変数ではありません");
  }
  if (node->var == NULL && node->gvar == NULL) {
    error("varが設定されていないgen_var_addr");
  }

  if (node->kind == ND_LVAR) {
    printf("  lea rax, [rbp-%d]\n", node->var->offset);
  }
  if (node->kind == ND_GVAR) {
    printf("  lea rax, %s[rip]\n", strndup(node->gvar->name, node->gvar->len));
  }
  printf("  push rax\n");
}

// スタック先頭のアドレスの変数の値をスタック先頭に積む
// tyは変数の型
void gen_var_val(Type *ty) {
  // 配列の場合には先頭にそのままアドレスを積んでおく
  if (is_array(ty)) {
    return;
  } 

  if (ty->ty == P_CHAR) {
    printf("  pop rax\n");
    printf("  movsx eax, BYTE PTR [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (ty->ty == P_INT) {
    printf("  pop rax\n");
    printf("  mov eax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

// スタック先頭の値をスタック2番目の値のアドレスへコピーする
// コピーした値をスタック先頭へ積む
void assign_as(Type *ty) {
  // src
  printf("  pop rdx\n");
  // dst
  printf("  pop rax\n");

  switch (ty->ty) {
  case P_CHAR:
    printf("  mov [rax], dl\n");
    break;
  case P_INT:
    printf("  mov [rax], edx\n");
    break;
  default:
    printf("  mov [rax], rdx\n");
    break;
  }

  // srcをスタック先頭へ(代入式は代入した値が評価結果)
  printf("  push rdx\n");
}

// グローバル変数の領域を出力する
void gen_global() {
  for (GVar *var = globals; var; var = var->next) {
    char *name = strndup(var->name, var->len);
    printf("%s:\n", name);
    printf("  .zero %d\n", size_of(var->ty));
  }
}

// 文字列リテラルの領域を出力する
void gen_literal() {
  int i = 0;
  for (Literal *lit = literals; lit; lit = lit->next) {
    lit->n = i;
    printf(".LC%d:\n", i);
    printf("  .string %s\n", strndup(lit->tok->str, lit->tok->len));
    i++;
  }
}

// 関数呼び出し時のパラメータをレジスタへ設定する
void set_function_params(Node *node) {
  int i;

  for (i = 0; node->params[i]; i++) {
    gen(node->params[i]);
    printf("  pop rax\n");
    switch (i) {
    case 0:
      printf("  mov rdi, rax\n");
      break;
    case 1:
      printf("  mov rsi, rax\n");
      break;
    case 2:
      printf("  mov rdx, rax\n");
      break;
    case 3:
      printf("  mov rcx, rax\n");
      break;
    case 4:
      printf("  mov r8, rax\n");
      break;
    case 5:
      printf("  mov r9, rax\n");
      break;
    }
  }
}

// 関数が呼び出された際の仮引数をスタックへ割り当てる
void set_callee_arguments(Node *func_node) {
  int i;
  LVar *arg = func_node->locals;
  for (i = 0; i < 6 && func_node->arguments[i]; i++) {
    gen_var_addr(func_node->arguments[i]);
    printf("  pop rax\n");

    switch (i) {
    case 0:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], edi\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], dil\n");
      } else {
        printf("  mov [rax], rdi\n");
      }
      break;
    case 1:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], esi\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], sil\n");
      } else {
        printf("  mov [rax], rsi\n");
      }
      break;
    case 2:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], edx\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], dl\n");
      } else {
        printf("  mov [rax], rdx\n");
      }
      break;
    case 3:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], ecx\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], cl\n");
      } else {
        printf("  mov [rax], rcx\n");
      }
      break;
    case 4:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], r8d\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], r8b\n");
      } else {
        printf("  mov [rax], r8\n");
      }
      break;
    case 5:
      if (size_of(arg->ty) == 4) {
        printf("  mov [rax], r9d\n");
      } else if (size_of(arg->ty) == 1) {
        printf("  mov [rax], r9b\n");
      } else {
        printf("  mov [rax], r9\n");
      }
      break;
    }
    arg = arg->next;
  }
}

// nをalignの倍数になるように切り上げる
// 例: align_to(24, 16) => 32
int align_to(int n, int align) { return (n + align - 1) / align * align; }

// 関数定義で使うローカル変数のオフセット値とスタックサイズを設定する
void assign_stack_offset(Node *node) {
  int total = 0;
  for (LVar *var = node->locals; var; var = var->next) {
    var->offset = total + size_of(var->ty);
    total += size_of(var->ty);
  }
  // ABIの制約でcallする時にはスタックの境界値を16バイトに
  // する必要があるため16バイトでアライメントする
  node->stack_size = align_to(total, 16);
}

void gen(Node *node) {
  int else_start_label;
  int if_end_label;
  int loop_start_label;
  int loop_end_label;

  switch (node->kind) {
  case ND_VAR_DECLARE:
    comment_gen("ND_VAR_DECLARE");
    // 文を実行するごとにpop raxしているので
    // スタックの調整のために追加
    printf("  push 0\n");
    return;
  case ND_NUM:
    comment_gen("ND_NUM");
    printf("  push %d\n", node->val);
    return;
  case ND_LITERAL:
    comment_gen("ND_LITERAL");
    gen_literal_addr(node);
    return;
  case ND_LVAR:
    comment_gen("ND_LVAR");
    gen_var_addr(node);
    gen_var_val(node->var->ty);
    return;
  case ND_GVAR:
    comment_gen("ND_GVAR");
    gen_var_addr(node);
    gen_var_val(node->gvar->ty);
    return;
  case ND_ADDR:
    comment_gen("ND_ADDR");
    gen_var_addr(node->lhs);
    return;
  case ND_DEREF:
    comment_gen("ND_DEREF");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    comment_gen("ND_ASSIGN");
    if (node->lhs->kind == ND_DEREF) {
      gen(node->lhs->lhs);
    } else {
      gen_var_addr(node->lhs);
    }
    gen(node->rhs);
    assign_as(node->lhs->ty);

    return;
  case ND_RETURN:
    comment_gen("ND_RETURN");
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
    comment_gen("ND_IF");
    else_start_label = branch_label_counter++;
    if_end_label = branch_label_counter++;

    gen(node->cond);
    if (node->rhs) {
      // if - else
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LelseStart%d\n", else_start_label);
      gen(node->lhs);
      printf("  jmp .LifEnd%d\n", if_end_label);
      printf(".LelseStart%d:\n", else_start_label);
      gen(node->rhs);
    } else {
      // if
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LifEnd%d\n", if_end_label);
      gen(node->lhs);
    }
    printf(".LifEnd%d:\n", if_end_label);
    return;
  case ND_WHILE:
    comment_gen("ND_WHILE");
    loop_start_label = branch_label_counter++;
    loop_end_label = branch_label_counter++;

    printf(".LloopStart%d:\n", loop_start_label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .LloopEnd%d\n", loop_end_label);
    gen(node->lhs);
    printf("  jmp .LloopStart%d\n", loop_start_label);
    printf(".LloopEnd%d:\n", loop_end_label);
    return;
  case ND_FOR:
    comment_gen("ND_FOR");
    loop_start_label = branch_label_counter++;
    loop_end_label = branch_label_counter++;

    if (node->init) {
      gen(node->init);
    }
    printf(".LloopStart%d:\n", loop_start_label);
    if (node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .LloopEnd%d\n", loop_end_label);
    }
    gen(node->lhs);
    if (node->post) {
      gen(node->post);
    }
    printf("  jmp .LloopStart%d\n", loop_start_label);
    printf(".LloopEnd%d:\n", loop_end_label);
    return;
  case ND_BLOCK:
    comment_gen("ND_BLOCK");
    for (int i = 0; node->stmts[i]; i++) {
      gen(node->stmts[i]);
      printf("  pop rax\n");
    }
    return;
  case ND_FCALL:
    comment_gen("ND_FCALL");
    set_function_params(node);
    printf("  call %s\n", node->fname);
    printf("  push rax\n");
    return;
  case ND_FUNCTION:
    comment_gen("ND_FUNCTION");
    assign_stack_offset(node);

    printf("%s:\n", node->fname);
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", node->stack_size);
    set_callee_arguments(node);

    gen(node->lhs);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

void codegen() {
  printf(".intel_syntax noprefix\n");

  printf(".data\n");
  gen_global();
  gen_literal();

  printf(".text\n");
  printf(".global main\n");

  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    // 式の評価結果をraxへ設定しておく
    printf("  pop rax\n");
  }
}
