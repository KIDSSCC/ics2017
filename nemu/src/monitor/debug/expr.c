#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_LB=255,TK_RB=254,
  TK_NUM=253,
  TK_NEGATIVE=252,
  TK_0xNUM=251,
  TK_REG=250,
  TK_DERE=249,
  TK_NEQ=248,
  TK_AND=247,
  TK_OR=246,
  TK_NOT=245

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\(", TK_LB},	// LB
  {"\\)", TK_RB},	// RB
  {"^(0|[1-9][0-9]*)", TK_NUM},	//NUM
  {"^0x[0-9a-fA-F]+", TK_0xNUM},	//0xNUM
  {"^\\$[a-z]+", TK_REG},	//REG
  {"==", TK_EQ},	// equal
  {"!=", TK_NEQ},	// nequal
  {"&&", TK_AND},	// AND
  {"\\|\\|", TK_OR},	//OR
  {"\\+", '+'},         // plus
  {"-", '-'},	//sub
  {"\\*", '*'},	//mul
  {"/", '/'},	//div
  {"!", TK_NOT}	//NOT
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
		case '+':
			tokens[nr_token++].type='+';
			break;
		case '-':
			tokens[nr_token++].type='-';
	                break;
		case '*':
			tokens[nr_token++].type='*';
			break;
		case '/':
			tokens[nr_token++].type='/';
			break;
		case TK_EQ:
			tokens[nr_token++].type=TK_EQ;
			break;
		case TK_NEQ:
			tokens[nr_token++].type=TK_NEQ;
			break;
		case TK_AND:
			tokens[nr_token++].type=TK_AND;
			break;
		case TK_OR:
			tokens[nr_token++].type=TK_OR;
			break;
		case TK_LB:
			tokens[nr_token++].type=TK_LB;
			break;
		case TK_RB:
			tokens[nr_token++].type=TK_RB;
			break;
		case TK_NOT:
			tokens[nr_token++].type=TK_NOT;
			break;
		case TK_NUM:
			Assert(substr_len<32,"the expr token is too long\n");
			tokens[nr_token].type=TK_NUM;
			memcpy(tokens[nr_token].str, substr_start, substr_len);
			memset(tokens[nr_token].str+substr_len, '\0', 1);
			nr_token++;
			break;
		case TK_0xNUM:
			Assert(substr_len<32,"the expr token is too long\n");
			tokens[nr_token].type=TK_0xNUM;
			memcpy(tokens[nr_token].str, substr_start, substr_len);
			memset(tokens[nr_token].str+substr_len, '\0', 1);
			nr_token++;
			break;
		case TK_REG:
			Assert(substr_len<32,"the expr token is too long\n");
			tokens[nr_token].type=TK_REG;
			memcpy(tokens[nr_token].str, substr_start, substr_len);
			memset(tokens[nr_token].str+substr_len, '\0', 1);
			nr_token++;
			break;
          default: break;
        }

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

bool check_parentheses(int p, int q, int* errorType)
{
	if((tokens[p].type!=TK_LB)||(tokens[q].type!=TK_RB)){
		return false;
	}
	int numOfBr = 0;
	int theRightMatchForLeft = q;
	for(int i=p; i<=q; i++){
		if(tokens[i].type==TK_LB){
			numOfBr++;
		}else if(tokens[i].type==TK_RB){
			if(numOfBr==1){
				theRightMatchForLeft = i;
			}
			numOfBr--;
			if(numOfBr<0){
				*errorType=1;
				return false;
			}
		}
	}
	if(theRightMatchForLeft!=q){
		return false;
	}
	if(numOfBr!=0){
		*errorType = 2;
		return false;
	}
	return (int)true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
