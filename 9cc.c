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

//エラーを報告するための関数
//printfと同じ引数を取る
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
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
		error("'%c'ではありません", op);
	}
	token = token->next;
}

//次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM){
		error("数ではありません");
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
Token *tokenize(char *p) {
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
		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}
		//今チェックしてる文字が１０進数かどうか、１０進数であらば、新しいトークンを作成してcurにつなげる
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}


		error("トークナイズできません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}


int main(int argc, char **argv) {
	if (argc != 2) {
		error("引数の個数が正しくありません");
		return 1;
	}

	// トークナイズする
	token = tokenize(argv[1]);
	
	//アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");


	//式の最初は数でなければならないので、それをチェックして最初のmov命令を出力
	printf(" mov rax, %d\n", expect_number());

	//’＋　＜数＞”あるいは”ー　＜数＞”というトークンの並びを消費しつつアセンブリを出力
	while (!at_eof()) {
		if (consume('+')) {
			printf(" add rax, %d\n", expect_number());
			continue;
		}

		expect('-');
		printf(" sub rax, %d\n", expect_number());
	}

	printf(" ret\n");
	return 0;
}

















































