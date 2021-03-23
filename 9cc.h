#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum {
	TK_RESERVED, //記号
	TK_NUM, //整数トークン
	TK_EOF, //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind; //トークンの型
	Token *next; //次の入力トークン
	int val; //kindがTK_NUMの場合、その数値
	char *str; //トークンの文字列
	int len; //トークンの長さ
};

Token *token;

char *user_input;

// parse.cで使う関数のプロトタイプ宣言

void error_at(char *loc, char *fmt, ...);

bool consume(char *op);

void expect(char *op);

int expect_number();

bool at_eof();

bool startwith(char *p, char *q);

Token *tokenize();

typedef enum {
	ND_ADD, //+
	ND_SUB, //-
	ND_MUL, //*
	ND_DIV, // /
	ND_EQ, // ==
	ND_NE, // !=
	ND_LT, // <
	ND_LE, // <=
	ND_NUM, // Integer
}NodeKind;

//AST node type 
typedef struct Node Node;

struct Node {
	NodeKind kind; // Node kind
	Node *lhs; //Left-hand size
	Node *rhs; //Right-hand size
	int val; // Used if kind == ND_NUM
};

Node *new_node(NodeKind kind);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);

Node *new_num(int val);

Node *expr();

Node *equality();

Node *relational();

Node *add();

Node *mul();

Node *primary();

Node *unary();

//codegen.cで使う関数のプロトタイプ宣言

void gen(Node *node);





























