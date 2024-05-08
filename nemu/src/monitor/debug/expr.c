#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_DEC, TK_HEX, TK_REG,
  TK_NEQ, TK_GEQ, TK_LEQ,
  TK_MINUS, TK_DEREF,
  TK_LS, TK_RS,
  TK_AND, TK_OR,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\*", '*'},
  {"/", '/'},
  {"%", '%'},
  {"\\+", '+'},         // plus
  {"-", '-'},
  {"<<", TK_LS},
  {">>", TK_RS},
  {"\\(", '('},
  {"\\)", ')'},
  {"!=", TK_NEQ},
  {">=", TK_GEQ},
  {"<=", TK_LEQ},
  {"==", TK_EQ},         // equal
  {">", '>'},
  {"<", '<'},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"&", '&'},
  {"\\|", '|'},
  {"\\^", '^'},
  {"!", '!'},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_DEC},
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|EAX|ECX|EDX|EBX|ESP|EBP|ESI|EDI|EIP)", TK_REG},
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
    if (nr_token >= 32) {
      Log("Too many tokens");
      return false;
    }
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

        tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type) {
          case '-':
            if (nr_token == 0 || (tokens[nr_token - 1].type != TK_DEC && tokens[nr_token - 1].type != TK_HEX && tokens[nr_token - 1].type != TK_REG && tokens[nr_token - 1].type != ')')) {
              tokens[nr_token].type = TK_MINUS;
            }
            break;
          case '*':
            if (nr_token == 0 || (tokens[nr_token - 1].type != TK_DEC && tokens[nr_token - 1].type != TK_HEX && tokens[nr_token - 1].type != TK_REG && tokens[nr_token - 1].type != ')')) {
              tokens[nr_token].type = TK_DEREF;
            }
            break;
          case TK_DEC:
          case TK_HEX:
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          default: break;
        }
        if (rules[i].token_type != TK_NOTYPE) {
          nr_token++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      Log("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool *success) {
  // 返回值为true表示首尾括号相匹配， success为true表示表达式合法
  int i, cnt = 0;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      cnt++;
    } else if (tokens[i].type == ')') {
      cnt--;
    }
    if (cnt < 0) {
      *success = false;
      return false;
    }
  }
  if (cnt == 0) {
    // 如果表达式合法，则继续判断首尾括号是否匹配
    bool res = true;
    if (tokens[p].type != '(' || tokens[q].type != ')') {
      res = false;
    } else {
      for (i = p; i <= q; i++) {
        if (tokens[i].type == '(') {
          cnt++;
        } else if (tokens[i].type == ')') {
          cnt--;
        }
        if (cnt == 0 && i != q) {
          res = false;
          break;
        }
      }
    }
    return res;
  } else {
    *success = false;
    return false;
  }
}

int dominant_operator(int p, int q) {
  // 返回值为-1表示表达式不合法，否则返回值为表达式中优先级最低的运算符的位置
  int i, cnt = 0, op = -1, op_priority = 0;
  for (i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      cnt++;
    } else if (tokens[i].type == ')') {
      cnt--;
    }
    if (cnt == 0) {
      if (tokens[i].type == TK_MINUS || tokens[i].type == TK_DEREF || tokens[i].type == '!') {
        if (op_priority <= 0 && op == -1) {
          op_priority = 0;
          op = i;
        }
      } else if (tokens[i].type == '*' || tokens[i].type == '/' || tokens[i].type == '%') {
        if (op_priority <= 1) {
          op_priority = 1;
          op = i;
        }
      } else if (tokens[i].type == '+' || tokens[i].type == '-') {
        if (op_priority <= 2) {
          op_priority = 2;
          op = i;
        }
      } else if (tokens[i].type == TK_LS || tokens[i].type == TK_RS) {
        if (op_priority <= 3) {
          op_priority = 3;
          op = i;
        }
      } else if (tokens[i].type == TK_GEQ || tokens[i].type == TK_LEQ || tokens[i].type == '>' || tokens[i].type == '<') {
        if (op_priority <= 4) {
          op_priority = 4;
          op = i;
        }
      } else if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) {
        if (op_priority <= 5) {
          op_priority = 5;
          op = i;
        }
      } else if (tokens[i].type == '&') {
        if (op_priority <= 6) {
          op_priority = 6;
          op = i;
        }
      } else if (tokens[i].type == '^') {
        if (op_priority <= 7) {
          op_priority = 7;
          op = i;
        }
      } else if (tokens[i].type == '|') {
        if (op_priority <= 8) {
          op_priority = 8;
          op = i;
        }
      } else if (tokens[i].type == TK_AND) {
        if (op_priority <= 9) {
          op_priority = 9;
          op = i;
        }
      } else if (tokens[i].type == TK_OR) {
        if (op_priority <= 10) {
          op_priority = 10;
          op = i;
        }
      }
    }
  }
  return op;
}

uint32_t eval(int p, int q, bool* success) {
  if (*success == false) {
    return -1;
  } else if (p > q) {
    Log("Bad expression");
    *success = false;
    return -1;
  } else if (p == q) {
    uint32_t val;
    if (tokens[q].type == TK_DEC) {
      sscanf(tokens[q].str, "%u", &val);
    } else if (tokens[q].type == TK_HEX) {
      sscanf(tokens[q].str, "%x", &val);
    } else if (tokens[q].type == TK_REG) {
      if (strcmp(tokens[q].str, "$eax") == 0 || strcmp(tokens[q].str, "$EAX") == 0) {
        val = cpu.eax;
      } else if (strcmp(tokens[q].str, "$ecx") == 0 || strcmp(tokens[q].str, "$ECX") == 0) {
        val = cpu.ecx;
      } else if (strcmp(tokens[q].str, "$edx") == 0 || strcmp(tokens[q].str, "$EDX") == 0) {
        val = cpu.edx;
      } else if (strcmp(tokens[q].str, "$ebx") == 0 || strcmp(tokens[q].str, "$EBX") == 0) {
        val = cpu.ebx;
      } else if (strcmp(tokens[q].str, "$esp") == 0 || strcmp(tokens[q].str, "$ESP") == 0) {
        val = cpu.esp;
      } else if (strcmp(tokens[q].str, "$ebp") == 0 || strcmp(tokens[q].str, "$EBP") == 0) {
        val = cpu.ebp;
      } else if (strcmp(tokens[q].str, "$esi") == 0 || strcmp(tokens[q].str, "$ESI") == 0) {
        val = cpu.esi;
      } else if (strcmp(tokens[q].str, "$edi") == 0 || strcmp(tokens[q].str, "$EDI") == 0) {
        val = cpu.edi;
      } else if (strcmp(tokens[q].str, "$eip") == 0 || strcmp(tokens[q].str, "$EIP") == 0) {
        val = cpu.eip;
      } else {
        Log("Unknown register: %s", tokens[q].str);
        *success = false;
        return -1;
      }
    } else {
      Log("Bad expression");
      *success = false;
      return -1;
    }
    return val;
  } else if (check_parentheses(p, q, success)) {
    return eval(p + 1, q - 1, success);
  } else {
    int op = dominant_operator(p, q);
    if (op == -1) {
      Log("Bad expression");
      *success = false;
      return -1;
    } else if (tokens[op].type == TK_MINUS || tokens[op].type == TK_DEREF || tokens[op].type == '!') {
      uint32_t val = eval(op + 1, q, success);
      switch (tokens[op].type) {
        case TK_MINUS: return -val;
        case TK_DEREF: return vaddr_read(val, 4);
        case '!': return !val;
        default: Log("Unknown operator: %d", tokens[op].type); *success = false; return -1;
      }
    } else {
      uint32_t val1 = eval(p, op - 1, success);
      uint32_t val2 = eval(op + 1, q, success);
      switch (tokens[op].type) {
        case '+': return val1 + val2;
        case '-': return val1 - val2;
        case '*': return val1 * val2;
        case '/': if (val2 == 0) {
                    Log("Divided by zero");
                    *success = false;
                    return -1;
                  }
                  return val1 / val2;
        case TK_EQ: return val1 == val2;
        case TK_NEQ: return val1 != val2;
        case TK_GEQ: return val1 >= val2;
        case TK_LEQ: return val1 <= val2;
        case '>': return val1 > val2;
        case '<': return val1 < val2;
        case TK_LS: return val1 << val2;
        case TK_RS: return val1 >> val2;
        case '&': return val1 & val2;
        case '^': return val1 ^ val2;
        case '|': return val1 | val2;
        case TK_AND: return val1 && val2;
        case TK_OR: return val1 || val2;
        default: Log("Unknown operator: %d\n", tokens[op].type); *success = false; return -1;
      }
    }
  }
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  *success = true;

  return eval(0, nr_token - 1, success);
}
