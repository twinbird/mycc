#include "mycc.h"
#include <stdio.h>

// =============================
// コードジェネレータ
// =============================

// 分岐ラベルにつける数値
int branch_label_counter;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node *node) {
  int else_start_label;
  int if_end_label;
  int loop_start_label;
  int loop_end_label;

  switch (node->kind) {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF:
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
