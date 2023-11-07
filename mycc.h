#include <stdbool.h>

// ====================
// エラー表示
// ====================
// 入力プログラムファイル名
extern char *src_filename;
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
  TK_STRING,   // 文字列リテラル
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
Token *consume_string();
void expect(char *op);
int expect_number();
bool at_eof();


// ====================
// 型
// ====================
// プリミティブ型
enum PType {
  P_INT,   // int
  P_CHAR,  // char
  P_LONG,  // long
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
// char型を返す
Type *type_char();
// long型を返す
Type *type_long();
// 引数の型のサイズを返す
int size_of(Type *ty);
// 指定型を示す配列型を返す
Type *array_of(Type *to, int size);
// 配列なら1
bool is_array(Type *t);
// 指定型を示すポインタ型を返す
Type *pointer_to(Type *to);
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
  GVar *next;    // 次の変数かNULL
  char *name;    // 変数の名前
  int len;       // 名前の長さ
  Type *ty;      // 型
  int is_inited; // 初期化式なら1
  int init_int;  // int型の初期値
};
// グローバル変数のリスト
extern GVar *globals;
GVar *find_gvar(Token *tok);
GVar *append_globals(Token *tok, Type *ty);

// ====================
// 文字列リテラル
// ====================
typedef struct Literal Literal;
struct Literal {
  Literal *next;  // 次の変数かNULL
  Token *tok;     // リテラルトークン
  int n;          // 通し番号
};
// グローバル変数のリスト
extern Literal *literals;
Literal *find_literal(Token *tok);
Literal *append_literals(Token *tok);

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
  ND_GVAR,// グローバル変数
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
  ND_LITERAL, // 文字列リテラル
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
  GVar *gvar;         // kindがND_GVARの場合のグローバル変数
  LVar *locals;       // 関数定義で使うローカル変数のリスト
  int stack_size;     // 関数定義で使うローカル変数のスタックサイズ
  Literal *literal;   // 文字列リテラルへのポインタ
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
