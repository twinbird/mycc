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

// プリミティブ型
enum PType {
  P_INT,  // int
  P_PTR,  // ポインタ
};

// 変数の型
typedef struct Type Type;
struct Type {
  enum PType ty;         // プリミティブ型
  struct Type *ptr_to;   // PTRの場合示す先の型
};

// int型を返す
Type *type_int();

// ローカル変数
typedef struct LVar LVar;
struct LVar {
  LVar *next;  // 次の変数かNULL
  char *name;  // 変数の名前
  int len;     // 名前の長さ
  int offset;  // RBPからのオフセット
  Type *ty;    // 型
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
  ND_IF, // if
  ND_WHILE, // while
  ND_FOR, // for
  ND_BLOCK, // { }のコードブロック
  ND_FCALL, // 関数呼び出し
  ND_FUNCTION, // 関数定義
  ND_DEREF, // *(単項)
  ND_ADDR, // &(単項)
  ND_VAR_DECLARE, // 変数宣言
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;      // ノードの型
  Node *lhs;          // 左辺
  Node *rhs;          // 右辺
  Node *cond;         // 条件式
  Node *init;         // forループの初期化式
  Node *post;         // forループの更新式
  Node *stmts[100];   // ND_BLOCKの場合に利用
  int val;            // kindがND_NUMの場合のみ利用
  int offset;         // kindがND_LVARの場合のみ利用
  Type *ty;           // kindがND_LVARの場合のみ利用
  char fname[100];    // kindがND_FCALLまたはND_FUNCTIONの場合関数名
  Node *params[6];    // 関数呼び出し時のパラメータ
  Node *arguments[6]; // 関数定義の仮引数
};
Node *expr();
Node *stmt();

// =============================
// コードジェネレータ
// =============================
void gen(Node *node);
void program();

// パース結果のノード
extern Node *code[100];
