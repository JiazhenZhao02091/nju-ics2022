/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
    TK_NOTYPE = 256,
    NUM = 1,
    RESGISTER = 2,
    HEX = 3,
    EQ = 4,
    NOTEQ = 5,
    OR = 6,
    AND = 7,
    ZUO = 8,
    YOU = 9,
    POINT, NEG

	/* TODO: Add more token types */

};

static struct rule {
    const char *regex;
    int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE},    // spaces
    {"\\+", '+'},         // plus
    {"\\-", '-'},         // sub
    {"\\*", '*'},         // mul
    {"\\/", '/'},         // div

    {"\\(", ZUO},
    {"\\)", YOU},
    /*
     * Inset the '(' and ')' on the [0-9] bottom case Bug.
     */


    {"\\$[a-z]+", RESGISTER},
    {"0[xX][0-9a-fA-F]+", HEX},
    {"[0-9]*", NUM},
    // zimu

    {"==", EQ},        // equal
    {"!=", NOTEQ},


    {"\\|\\|", OR},       // Opetor
    {"&&", AND},
    {"!", '!'},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i ++) {
	ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
	if (ret != 0) {
	    regerror(ret, &re[i], error_msg, 128);
	    panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
	}
    }
}

typedef struct token {
    int type;
    char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

int len = 0; // Record the struct tokens length.


static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') { 
	/* Try all rules one by one. */
	for (i = 0; i < NR_REGEX; i ++) {
	    if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
	//	char *substr_start = e + position;
		int substr_len = pmatch.rm_eo;

		//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
		//	i, rules[i].regex, position, substr_len, substr_len, substr_start);

		position += substr_len;

		/* TODO: Now a new token is recognized with rules[i]. Add codes
		 * to record the token in the array `tokens'. For certain types
		 * of tokens, some extra actions should be performed.
		 */
		Token tmp_token;
		switch (rules[i].token_type) {
		    case '+':
			tmp_token.type = '+';
			tokens[nr_token ++] = tmp_token;
			break;
		    case '-':
			tmp_token.type = '-';
			tokens[nr_token ++] = tmp_token;
			break;
		    case '*':
			tmp_token.type = '*';
			tokens[nr_token ++] = tmp_token;
			break;
		    case '/':
			tmp_token.type = '/';
			tokens[nr_token ++] = tmp_token;
			break;
		    case 256:
			break;
		    case '!':
			tmp_token.type = '!';
			tokens[nr_token ++] = tmp_token;
			break;
		    case 9:
			tmp_token.type = ')';
			tokens[nr_token ++] = tmp_token;
			break;
		    case 8:
			tmp_token.type = '(';
			tokens[nr_token ++] = tmp_token;
			break;

			// Special
		    case 1: // num
			tokens[nr_token].type = 1;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    case 2: // regex
			tokens[nr_token].type = 2;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    case 3: // HEX
			tokens[nr_token].type = 3;
			strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
			nr_token ++;
			break;
		    case 4:
			tokens[nr_token].type = 4;
			strcpy(tokens[nr_token].str, "==");
			nr_token++;
			break;
		    case 5:
			tokens[nr_token].type = 5;
			strcpy(tokens[nr_token].str, "!=");
			nr_token++;
			break;
		    case 6:
			tokens[nr_token].type = 6;
			strcpy(tokens[nr_token].str, "||");
			nr_token++;
			break;
		    case 7:
			tokens[nr_token].type = 7;
			strcpy(tokens[nr_token].str, "&&");
			nr_token++;
			break;
		    default:
			printf("i = %d and No rules is com.\n", i);
			break;
		}
		len = nr_token;
		break;
	    }
	}

	if (i == NR_REGEX) {
	    printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
	    return false;
	}
    }

    return true;
}


bool check_parentheses(int p, int q)
{
   // return true;
//    printf("p = %d, q = %d\n",tokens[p].type, tokens[q].type);
    if(tokens[p].type != '('  || tokens[q].type != ')')
	return false;
    int l = p , r = q;
    while(l < r)
    {
	if(tokens[l].type == '('){
	    if(tokens[r].type == ')')
	    {
		l ++ , r --;
		continue;
	    } 

	    else 
		r --;
	}
	else if(tokens[l].type == ')')
	    return false;
	else l ++;
    }
    return true;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}


uint32_t eval(int p, int q) {
    if (p > q) {
	/* Bad expression */
	printf("p  = %d, q = %d , p > q\n",p,q);
	assert(0);
	return -1;
    }
    else if (p == q) {
	/* Single token.
	 * For now this token should be a number.
	 * Return the value of the number.
	 */
	return atoi(tokens[p].str);
    }
    else if (check_parentheses(p, q) == true) {
	/* The expression is surrounded by a matched pair of parentheses.
	 * If that is the case, just throw away the parentheses.
	 */
	// printf("check p = %d, q = %d\n",p + 1 , q - 1);
	return eval(p + 1, q - 1);
    }
   /* else if(check_parentheses(p, q) == false){
    	printf("Unique\n");
	return -1;
    }
    */
    else {
	int op = -1; // op = the position of 主运算符 in the token expression;
	bool flag = false; 
	for(int i = p ; i <= q ; i ++)
	{
	    if(tokens[i].type == '(')
	    {
		while(tokens[i].type != ')')
			i ++;
	    }
	    if(tokens[i].type == '+' || tokens[i].type == '-'){
		flag = true;
		op = max(op, i);
	    }
	    if(!flag && (tokens[i].type == '*' || tokens[i].type == '/') ){
		op = max(op, i);
	    }
	}
//	printf("op position is %d\n", op);
	// if register return $register
	int  op_type = tokens[op].type;

	// 递归处理剩余的部分
	uint32_t  val1 = eval(p, op - 1);
	uint32_t  val2 = eval(op + 1, q);
//	printf("val1 = %d, val2 = %d \n", val1, val2);

	switch (op_type) {
	    case '+': 
		return val1 + val2;
	    case '-': 
		return val1 - val2;
	    case '*': 
		return val1 * val2;
	    case '/': 
		return val1 / val2;
	    default: 
		printf("No Op type.");
		assert(0);
	}
    }
}



word_t expr(char *e, bool *success)
{
    if (!make_token(e)) {
	printf("make_token Success\n");
	*success = false;
	return 0;
    }

    /* TODO: Insert codes to evaluate the expression. */
    *success = true;// Return token zhuang tai

    // for(int i = 0 ; i < len ; i ++){printf("token[%d] type = [%d]\n", i, tokens[i].type);}
    /*
     *  Get length
     */ 
    int tokens_len = 0;
    for(int i = 0 ; i < 30 ; i ++)
    {
	if(tokens[i].type == 0)
	    break;
	tokens_len ++;
    }
    /*
     * True Expr
     * Data tokens (num, op_type)
     */ 
    uint32_t res = 0;
//    printf("Begin calc <===================>\n");
    res = eval(0, tokens_len - 1);
 //   printf("check flag = %d\n",check_parentheses(0, tokens_len - 1));
    printf("uint32_t res = %d\n", res);		

    return 0;
}
