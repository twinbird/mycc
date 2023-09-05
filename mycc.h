// ====================
// エラー表示
// ====================
// 入力プログラム
extern char *user_input;
void error(char *fmt, ...);

// ====================
// トークナイザ
// ====================
typedef enum {
  TK_RESERVED, // 予約語
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わり
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 現在着目しているトークン
extern Token *token;

Token *tokenize(char *p);

// ローカル変数
typedef struct LVar LVar;
struct LVar {
  LVar *next;  // 次の変数かNULL
  char *name;  // 変数の名前
  int len;     // 名前の長さ
  int offset;  // RBPからのオフセット
};
// ローカル変数のリスト
extern LVar *locals;

// =======================
// 抽象構文木
// =======================
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_LVAR,// ローカル変数
  ND_ASSIGN, // =
  ND_RETURN, // return
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // ノードの型
  Node *lhs;     // 左辺
  Node *rhs;     // 右辺
  int val;       // kindがND_NUMの場合のみ利用
  int offset;    // kindがND_LVARの場合のみ利用
};
Node *expr();

// =============================
// コードジェネレータ
// =============================
void gen(Node *node);
void program();

// パース結果のノード
extern Node *code[100];