#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
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
  {"\\(",TK_LB},          //LB
  {"\\)",TK_RB},           //RB
  {"^0x[0-9a-fA-F]+",TK_0xNUM},//0xnum
  {"^(0|[1-9][0-9]*)",TK_NUM},//NUM
  {"^\\$[a-z]+",TK_REG},
  {"==", TK_EQ},         // equal
  {"!=",TK_NEQ},        //nequal
  {"&&",TK_AND},        //AND
  {"\\|\\|",TK_OR},         //OR
  {"\\+", '+'},         // plus
  {"-",'-'},             // sub
  {"\\*",'*'},            //mul
  {"/",'/'},            //div
  {"!",TK_NOT}          //NOT

};

//NR_REGEX为rules数组中定义了多少条规则
#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
//在monitor.c中调用了init_regex进行了正则匹配的预编译
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  //使用的是库函数，不是自定义的函数
  for (i = 0; i < NR_REGEX; i ++) {
      //REG_EXTENDED 以功能更加强大的扩展正则表达式的方式进行匹配。
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

  //没有到达休止符前一直通过while进行匹配
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
        //if的判定条件，从当前开始的字符串匹配到了某一正则表达式
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
          /*
           * 当前匹配成功的正则表达式的起始为e + position
           * 在pmatch结构体中，rm.eo代表了结束的位置，因为起始位置是0，所以结束位置也就是表达式的长度
           */
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            //i, rules[i].regex, position, substr_len, substr_len, substr_start);
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
            case TK_NUM:
                Assert(substr_len<=32,"the expr token is too long\n");
                tokens[nr_token].type=TK_NUM;
                memcpy(tokens[nr_token].str,substr_start,substr_len);
                memset(tokens[nr_token++].str+substr_len,'\0',1);
                break;
            case TK_0xNUM:
                Assert(substr_len<=32,"the expr token is too long\n");
                tokens[nr_token].type=TK_0xNUM;
                memcpy(tokens[nr_token].str,substr_start,substr_len);
                memset(tokens[nr_token++].str+substr_len,'\0',1);
                break;
            case TK_REG:
                Assert(substr_len<=32,"the expr token is too long\n");
                tokens[nr_token].type=TK_REG;
                memcpy(tokens[nr_token].str,substr_start,substr_len);
                memset(tokens[nr_token++].str+substr_len,'\0',1);
                break;
          default:
              break;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  return (int) true;
}

bool check_parentheses(int p,int q,int*error_type)
{
    //检查左右括号是否匹配，要求整个表达式都需要被包围起来
    if((tokens[p].type!=TK_LB)||(tokens[q].type!=TK_RB))
    {
        return false;
    }
    //到这里首先可以确定左右已经都是括号了，开始进行一遍扫描
    //numOfBr在左括号加一，右括号减一
    //the_right_match_for_left用来记录与最左括号匹配的右括号的位置
    int numOfBr=0;
    int the_right_match_for_left=q;
    for(int i=p;i<=q;i++)
    {
        if(tokens[i].type==TK_LB)
        {
            numOfBr++;
        }
        else if(tokens[i].type==TK_RB)
        {
            if(numOfBr==1)
            {
                the_right_match_for_left=i;
                //检查最左和最右是不是一对
            }
            numOfBr--;
            if(numOfBr<0)
            {
                *error_type=1;
                return false;
            }
        }
    }
    //检查是否所有括号都能匹配得上
    if(the_right_match_for_left!=q)
    {
        return false;
    }
    if(numOfBr!=0)
    {
        *error_type=2;
        return false;
    }
    return (int) true;
}

uint32_t regJudge(char* name,int *error_type);
uint32_t eval(int p,int q,int *error_type)
{
    if(p>q)
    {
        *error_type=1;
        return 0;
    }
    else if(p==q)
    {
        //return the value of the number
        char* tmp;
        if(tokens[p].type==TK_NUM)
        {
            int value=strtol(tokens[p].str,&tmp,10);
            return value;
        }
        else if (tokens[p].type==TK_0xNUM)
        {
            int value=strtol(tokens[p].str,&tmp,16);
            return value;
        }
        else if(tokens[p].type==TK_REG)
        {
            uint32_t result=regJudge(tokens[p].str,error_type);
            return result;
        }
    }
    else
    {
        int tryTocheck_parentheses=0;
        bool checkResult=check_parentheses(p,q,&tryTocheck_parentheses);
        if(tryTocheck_parentheses==0)
        {
            //tryTocheck_parentheses等于0，说明表达式不因为括号方面出现致命错误，仍可以根据其true或false结果进行向下计算
            if(checkResult)
            {
                int result= eval(p+1,q-1,error_type);
                if(*error_type==0)
                    return result;
                else
                    return 0;
            }
            else
            {
                //确定dominant operator,从p扫描到q
                int numofBr=0;
                int position=p-1;
                for(int i=p;i<=q;i++)
                {
                    if(tokens[i].type==TK_NUM)
                    {
                        continue;
                    }
                    else if(tokens[i].type==TK_LB)
                    {
                        numofBr++;
                    }
                    else if(tokens[i].type==TK_RB)
                    {
                        numofBr--;
                    }
                    else if((tokens[i].type=='+')||(tokens[i].type=='-')||(tokens[i].type=='*')||(tokens[i].type=='/')||(tokens[i].type==TK_EQ)||(tokens[i].type==TK_NEQ)||(tokens[i].type==TK_AND)||(tokens[i].type==TK_OR))
                    {
                        if(numofBr!=0)
                        {
                            continue;
                        }
                        if(position==p-1)
                        {
                            position=i;
                        }
                        else
                        {
                            if((tokens[position].type==TK_EQ)||(tokens[position].type==TK_NEQ)||(tokens[position].type==TK_AND)||(tokens[position].type==TK_OR))
                            {
                                if((tokens[i].type==TK_EQ)||(tokens[i].type==TK_NEQ)||(tokens[i].type==TK_AND)||(tokens[i].type==TK_OR))
                                {
                                    position=i;
                                }
                            }
                            else if((tokens[position].type=='+')||(tokens[position].type=='-'))
                            {
                                if((tokens[i].type!='*')&&(tokens[i].type!='/'))
                                {
                                    position=i;
                                }
                            }
                            else
                            {
                                position=i;
                            }
                        }
                    }
                }
                if(position!=p-1)
                {
                    //找到了domain operator，是一个二元表达式
                    int val1=eval(p,position-1,error_type);
                    if(*error_type!=0)
                        return 0;
                    int val2=eval(position+1,q,error_type);
                    if(*error_type!=0)
                        return 0;
                    switch (tokens[position].type) {
                        case '+':return val1+val2;
                        case '-':return val1-val2;
                        case '*':return val1*val2;
                        case '/':return val1/val2;
                        case TK_EQ:

                            return val1==val2?1:0;
                        case TK_NEQ:return val1!=val2?1:0;
                        case TK_AND:return val1*val2!=0?1:0;
                        case TK_OR:
                            if((val1==0)&&(val2==0))
                                return 0;
                            else
                                return 1;
                    }
                }
                else
                {
                    //没找到，暂时认为只有取负
                    if(tokens[p].type==TK_NEGATIVE)
                    {
                        int val=eval(p+1,q,error_type);
                        if(*error_type!=0)
                            return 0;
                        return -val;
                    }
                    else if(tokens[p].type==TK_DERE)
                    {
                        int val=eval(p+1,q,error_type);
                        if(*error_type!=0)
                            return 0;
                        if(val>=0x8000000)
                        {
                            *error_type=5;
                            return 0;
                        }
                        return vaddr_read(val,4);
                    }
                    else if(tokens[p].type==TK_NOT)
                    {
                        int val=eval(p+1,q,error_type);
                        if(*error_type!=0)
                            return 0;
                        return val==0?0:1;
                    }
                }

            }
        }
        else
        {
            //根据tryTocheck_parentheses的值设置error_type
            /*
             * tryTocheck_parentheses=1:右括号数量多于左括号
             * tryTocheck_parentheses=2:左括号数量多于右括号
             */
            switch (tryTocheck_parentheses) {
                case 1 :*error_type=2;return 0;
                case 2 :*error_type=3;return 0;
                default:return 0;
            }
        }
    }
    return 0;
}
bool certainType(int type)
{
    if((type!=TK_NUM)&&(type!=TK_RB)&&(type!=TK_REG))
    {
        return 1;
    }
    return 0;
}
uint32_t expr(char *e, int *success) {
    //expr函数本身不会造成异常结果
  if (!make_token(e)) {
      //make_token函数可能会造成异常结果，即token解析失败此时将success置为1
    *success = 1;
    return 0;
  }


  //关于减号，负数与乘法，解引用
  for(int i=0;i<nr_token;i++)
  {
      if((tokens[i].type=='-')&&(i==0||certainType(tokens[i-1].type)))
      {
          //是第一个字符或者其前一个字符不是数字或者右括号
          tokens[i].type=TK_NEGATIVE;
      }
      if((tokens[i].type=='*')&&(i==0||certainType(tokens[i-1].type)))
      {
          //是第一个字符或者其前一个字符不是数字或者右括号
          tokens[i].type=TK_DERE;
      }
  }
  /* TODO: Insert codes to evaluate the expression. */
  int tryToeval=0;
  int result= eval(0,nr_token-1,&tryToeval);
  if(tryToeval==0)
  {
      return result;
  }
  else
  {
      //根据tryToeval的错误类型设置success
      /*
       * tryToeval=1：出现p>q
       * tryToeval=2:出现右括号数量多于左括号
       * tryToeval=3:出现左括号数量多于右括号
       * tryToeval=4:找不到对应寄存器
       * tryToeval=5:访问内存越界
       */
      switch (tryToeval) {
          case 1:*success=2;return 0;
          case 2:*success=3;return 0;
          case 3:*success=4;return 0;
          case 4:*success=5;return 0;
          case 5:*success=6;return 0;
          default:return 0;
      }
      return 0;
  }
}

uint32_t regJudge(char* name,int *error_type)
{
    if(strcmp(name,"$eip")==0)
    {
        return cpu.eip;
    }
    for(int i=0;i<8;i++)
    {
        if(strcmp(name+1,regsl[i])==0)
        {
            return reg_l(i);
        }
    }
    for(int i=0;i<8;i++)
    {
        if(strcmp(name+1,regsw[i])==0)
        {
            return reg_w(i);
        }
    }
    for(int i=0;i<8;i++)
    {
        if(strcmp(name+1,regsb[i])==0)
        {
            return reg_b(i);
        }
    }
    //error_type=4:找不到对应的寄存器
    *error_type=4;
    return 0;
}
