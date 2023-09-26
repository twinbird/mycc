#include <stdbool.h>

// ====================
// エラー表示
// ====================
// 入力プログラム
extern char *user_input;
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

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
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();


// ====================
// 型
// ====================
// プリミティブ型
enum PType {
  P_INT,   // int
  P_PTR,   // ポインタ
  P_ARRAY, // 配列
};

// 変数の型
typedef struct Type Type;
struct Type {
  enum PType ty;         // プリミティブ型
  struct Type *ptr_to;   // PTRの場合示す先の型
  int size;              // 配列のサイズ
};

// int型を返す
Type *type_int();
// 引数の型のサイズを返す
int size_of(Type *ty);
// 指定型を示す配列型を返す
Type *array_of(Type *to, int size);
// 配列なら1
bool is_array(Type *t);
// ポインタ型なら1
bool is_pointer(Type *t);

// ====================
// ローカル変数
// ====================
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
LVar *find_lvar(Token *tok);
LVar *append_locals(Token *tok, Type *ty);

// ====================
// グローバル変数
// ====================
typedef struct GVar GVar;
struct GVar {
  GVar *next;  // 次の変数かNULL
  char *name;  // 変数の名前
  int len;     // 名前の長さ
  Type *ty;    // 型
};
// グローバル変数のリスト
extern GVar *globals;
GVar *find_gvar(Token *tok);
GVar *append_globals(Token *tok, Type *ty);

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
  Type *ty;           // ノードの型
  char fname[100];    // kindがND_FCALLまたはND_FUNCTIONの場合関数名
  Node *params[6];    // 関数呼び出し時のパラメータ
  Node *arguments[6]; // 関数定義の仮引数
  LVar *var;          // kindがND_LVARの場合のローカル変数
  LVar *locals;       // 関数定義で使うローカル変数のリスト
  int stack_size;     // 関数定義で使うローカル変数のスタックサイズ
};
Node *expr();
Node *stmt();

// 引数のノードへ型を加える
Type *attach_type(Node *node);

// トップレベルのパース
void program();

// =============================
// コードジェネレータ
// =============================
void gen(Node *node);
void codegen();

// パース結果のノード
extern Node *code[100];
