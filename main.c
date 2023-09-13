#include "mycc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  token = tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  for (int i = 0; code[i]; i++) {
    gen(code[i]);
    // 式の評価結果をraxへ設定しておく
    printf("  pop rax\n");
  }

  return 0;
}
