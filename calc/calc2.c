// @file         calc2.c
// @description  四則演算(+,-,*,/)と括弧を扱える電卓

// ビルド方法
// $ gcc calc2.c

// 実行方法
// $ ./a [ [ 1 + 2 ] * 3 ] - 4

// 中置記法の処理イメージ
// 
// 1 + 2
//  [1][2]
//  [+]
// 
// 1 + 2 * 3
//  [1][2][3]
//  [+][*]
// 
// 1 * 2 + 3
//  [1][2] [2] [2][3]
//  [*]    []  [+]
// 
// 1 + 2 - 3 * 4
//  [1][2][3][4]
//  [+][-][*]

#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#	define dprintf printf
#else
#	define dprintf(...)
#endif

#define dmpaf(array, n, format) \
	{ \
		int i__; \
		printf("%s = [ ", #array); \
		for (i__ = 0; i__ < n; i__++) { \
			printf(format ", ", array[i__]); \
		} \
		printf("]\n"); \
	}

#ifdef DEBUG
#	define dmpia(array, n) dmpaf(array, n, "%d")
#	define dmpla(array, n) dmpaf(array, n, "%ld")
#else
#	define dmpia(array, n)
#	define dmpla(array, n)
#endif

typedef enum {
	TkAdd, 
	TkSub, 
	TkMul, 
	TkDiv, 
	TkLParen, 
	TkRParen
} Token;

const char * token_strings[] = {
	"+", // TkAdd
	"-", // TkSub
	"x", // TkMul  // '*'はシェルが解釈するので代わりにxを使う
	"/", // TkDiv
	"[", // TkLParen  // '('はシェルが解釈するので代わりに[を使う
	"]"  // TkRParen  // ')'はシェルが解釈するので代わりに]を使う
};

const int token_priorities[] = {
	1, // TkAdd
	1, // TkSub
	2, // TkMul
	2, // TkDiv
   -1, // TkLParen 問答無用でスタックに積まないといけない
	0  // TkRParen
};

#ifdef DEBUG
#define dmpta(array, n) \
	{ \
		int i__; \
		printf("%s = [ ", #array); \
		for (i__ = 0; i__ < n; i__++) { \
			printf("%s, ", token_strings[array[i__]]); \
		} \
		printf("]\n"); \
	}
#else
#	define dmpta(array, n)
#endif

int ope(Token t, long a1, long a2, long *ans)
{
	if (ans == NULL) return -1;
	
	switch (t)
	{
	case TkAdd:
		*ans = a1 + a2;
		break;
	case TkSub:
		*ans = a1 - a2;
		break;
	case TkMul:
		*ans = a1 * a2;
		break;
	case TkDiv:
		if (a2 == 0)
		{
			fprintf(stderr, "Error: %ld / %ld\n", a1, a2);
			return -2;
		}
		*ans = a1 / a2;
		break;
	default:
		fprintf(stderr, "Error: Invalid token '%d'\n", t);
		return -3;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	const int StackMax = 100;
	
	long	nstack[StackMax];
	Token	tstack[StackMax];
	int		np = 0;
	int		tp = 0;
	
	int		i, j, ret;
	long	n, a1, a2;
	Token	t, t_last;
	
	if (argc <= 1) {
		goto calc_NoArgument;
	}
	
	dprintf("argc = %d\n", argc - 1);
	dprintf("argv = [ ");
	for (i = 1; i < argc; i++)
	{
		dprintf("%s, ", argv[i]);
	}
	dprintf("]\n");
	
	dprintf("--- calculation process ---\n");
	for (i = 1; i < argc; i++)
	{
		dprintf("# loop %d\n", i);
		n = atol(argv[i]);
		if (n != 0 || strcmp(argv[i], "0") == 0) {
			dprintf("take %ld\n", n);
			
			if (np >= StackMax) goto nstack_StackOverflow;
			nstack[np++] = n;
		}
		else {
			if (strcmp(argv[i], token_strings[TkAdd]) == 0) {
				t = TkAdd;
			}
			else if (strcmp(argv[i], token_strings[TkSub]) == 0) {
				t = TkSub;
			}
			else if (strcmp(argv[i], token_strings[TkMul]) == 0) {
				t = TkMul;
			}
			else if (strcmp(argv[i], token_strings[TkDiv]) == 0) {
				t = TkDiv;
			}
			else if (strcmp(argv[i], token_strings[TkLParen]) == 0) {
				t = TkLParen;
			}
			else if (strcmp(argv[i], token_strings[TkRParen]) == 0) {
				t = TkRParen;
			}
			else {
				fprintf(stderr, "Error: Invalid argument \"%s\"\n", argv[i]);
				goto token_Invalid;
			}
			
			dprintf("take %s\n", token_strings[t]);
			
			if (tp >= StackMax) goto tstack_StackOverflow;
			
			if (t == TkLParen) {
				// 問答無用でスタックに積む
			}
			else {
				while (tp > 0)
				{
					t_last = tstack[tp-1];
					
					if (token_priorities[ t_last ] <= token_priorities[ t ]) {
						break;
					}
					else {
						if (np < 2) goto calc_ShortArgument;
						
						a2 = nstack[--np];
						a1 = nstack[--np];
						--tp;
						
						ret = ope(t_last, a1, a2, &n);
						if (ret != 0) {
							switch (ret)
							{
							case -1: goto app_NullPointer;
							case -2: goto calc_DevideByZero;
							case -3: goto token_Invalid;
							default:
								fprintf(stderr, "Error: ret = %d\n", ret);
								goto app_InvalidReturnCode;
							}
						}
						dprintf("calc %ld %s %ld = %ld\n", a1, token_strings[t_last], a2, n);
						
						nstack[np++] = n;
						
						if (tp > 0) {
							dmpla(nstack, np);
							dmpta(tstack, tp);
						}
					}
				}
			}
			
			if (t == TkRParen)
			{
				// スタックからTkLParenを読み捨てる
				if (tp <= 0) {
					fprintf(stderr, "Error: TkLParen is not found\n");
					goto token_NotFound;
				}
				
				t = tstack[--tp];
				if (t != TkLParen) {
					fprintf(stderr, "Error: token '%d' is not TkLParen\n", t);
					goto token_Invalid;
				}
			}
			else {
				tstack[tp++] = t;
			}
		}
		
		dmpla(nstack, np);
		dmpta(tstack, tp);
	}
	
	dprintf("--- all arguments were processed. ---\n");
	dmpla(nstack, np);
	dmpta(tstack, tp);
	
	while (tp > 0)
	{
		if (np < 2) goto calc_ShortArgument;
		
		a2 = nstack[--np];
		a1 = nstack[--np];
		t = tstack[--tp];
		
		ret = ope(t, a1, a2, &n);
		if (ret != 0) {
			switch (ret)
			{
			case -1: goto app_NullPointer;
			case -2: goto calc_DevideByZero;
			case -3: goto token_Invalid;
			default:
				fprintf(stderr, "Error: ret = %d\n", ret);
				goto app_InvalidReturnCode;
			}
		}
		
		nstack[np++] = n;
	}
	
	dprintf("--- all tokens were processed. ---\n");
	dmpla(nstack, np);
	dmpta(tstack, tp);
	
	if (np != 1) goto calc_ArgumentsRemaining;
	
	printf("ans = %ld\n", nstack[0]);
	return 0;
nstack_StackOverflow:
	fprintf(stderr, "Stack Overflow Error: np = %d\n", np);
	return 1;
tstack_StackOverflow:
	fprintf(stderr, "Stack Overflow Error: tp = %d\n", tp);
	return 2;
token_Invalid:
	fprintf(stderr, "Invalid Token Error\n");
	return 3;
token_NotFound:
	fprintf(stderr, "Token Not Found Error\n");
	return 4;
calc_NoArgument:
	fprintf(stderr, "No Argument Error\n");
	return 5;
calc_ShortArgument:
	fprintf(stderr, "Short Argument Error\n");
	return 6;
calc_ArgumentsRemaining:
	fprintf(stderr, "Argument Remaining Error\n");
	return 7;
calc_DevideByZero:
	fprintf(stderr, "Devide By Zero Error\n");
	return 8;
app_NullPointer:
	fprintf(stderr, "Null Pointer Error\n");
	return 9;
app_InvalidReturnCode:
	fprintf(stderr, "Invalid Return Code Error\n");
	return 10;
}

// vim: set fenc=sjis ff=dos:
