#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// トークンの種類
typedef enum {
	TK_RESERVED, //記号
	TK_NUM,	//整数トークン
	TK_EOF,	//入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token {
	TokenKind kind; //トークンの型
	Token *next; //次の入力トークン
	int val; //kindがTK_NUMの場合、その数値
	char *str; //トークンの文字列
};

Token *token;

char *user_input;

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");//pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}


//次のトークンが期待している記号のときには、トークンを１つ読み進めて真を返す。それ以外の場合には偽を返す。
bool consume(char op) {
	if(token->kind != TK_RESERVED || token->str[0] != op){
	       return false;
	}
	token = token->next;
	return true;
}

//次のトークンが期待している記号のときには、トークンを１つ読み進める。それ以外の場合にはエラーを報告する。
void expect(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op){
		error_at(token->str,"'%c'ではありません", op);
	}
	token = token->next;
}

//次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM){
		error_at(token->str,"数ではありません");
	}
	int val =token->val;
	token = token->next;
	return val;
}

//リストの最後かどうか
bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur ,char *str) {
	Token *tok = calloc(1, sizeof(Token));//Tokenのサイズでtokを初期化
	tok->kind = kind; //tokのｋindにトークンの種類を格納
	tok->str = str;//tokのstrにトークンを格納
	cur->next = tok;//curのnextに構造体tokを入れる
	return tok;
}

//入力文字列ｐをトークナイズしてそれを返す
Token *tokenize() {
	char *p = user_input;
	Token head;// headという名前の構造体を宣言
	head.next = NULL;//nextにNULLを入れる
	Token *cur = &head;//headのアドレスをcurにセット

	while (*p) {
		//空白文字をスキップ
		if (isspace(*p)) {//今チェックしている文字が空白であるのか空白であれば、次の文字に進める
			p++;
			continue;
		}
		//今チェックしている文字が記号であるならば、新しいトークンを作成してcurにつなげる
		if (strchr("+-*/()", *p)) {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}
		//今チェックしてる文字が１０進数かどうか、１０進数であらば、新しいトークンを作成してcurにつなげる
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}


		error_at(p, "invalid token");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

//
//Parser
//

typedef enum {
	ND_ADD, //+
	ND_SUB, //-
	ND_MUL, //*
	ND_DIV, // / 
	ND_NUM, // Integer
}NodeKind;

//AST node type
typedef struct Node Node;

struct Node {
	NodeKind kind; // Node kind
	Node *lhs; //Left-had size
	Node *rhs; //Right-hand size
	int val; // Used if kind == ND_NUM
};

Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

Node *expr();
Node *mul();
Node *primary();
Node *unary();

// expr = mul ("+" mul | "-" mul)*
Node *expr() {
	Node *node = mul();

	for (;;) {
		if (consume('+')){

			node = new_binary(ND_ADD, node, mul());
		
		}else if(consume('-')){

			node = new_binary(ND_SUB, node, mul());
		
		}else{
			return node;
		}
	}
}

// mul = primary ("*" primary | "/" primary)*
Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume('*')){

			node = new_binary(ND_MUL, node, unary());

		}else if(consume('/')){

			node = new_binary(ND_DIV, node, unary());
		}else{
			return node;
		}
	}
}
// unary = ("+" | "-")? primary
Node *unary() {
	if (consume('+')){
		return primary();
	}
	if (consume('-')){
		return new_binary(ND_SUB, new_num(0), primary());
	}
	return primary();
}

//primary = "(" expr ")" | num
Node *primary() {
	if (consume('(')) {
		Node *node = expr();
		expect(')');
		return node;
	}

	return new_num(expect_number());
}

//
//Code generator
//

void gen(Node *node) {
	if (node->kind == ND_NUM) {
		printf(" push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf(" pop rdi\n");
	printf(" pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf(" add rax, rdi\n");
			break;

		case ND_SUB:
			printf(" sub rax, rdi\n");
			break;

		case ND_MUL:
			printf(" imul rax, rdi\n");
			break;

		case ND_DIV:
			printf(" cqo\n");
			printf(" idiv rdi\n");
		       break;
	}

	printf(" push rax\n");
}	


int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "invalid args\n");
		return 1;
	}

	// トークナイズする
	user_input = argv[1];
	token = tokenize();
	Node *node = expr();
	
	//アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	gen(node);

	printf(" pop rax\n");
	printf(" ret\n");

	return 0;

}

















































