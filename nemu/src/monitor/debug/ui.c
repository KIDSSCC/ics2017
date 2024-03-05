#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);
static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Run N instructions", cmd_si },
  { "info", "print the information of reg or watchpoint", cmd_info},
  { "x", "scan the memory", cmd_x},
  { "p", "calculate the expr", cmd_p},

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		cpu_exec(1);
	} else {
		char* tmp;
		long value = strtol(arg, &tmp, 10);
		cpu_exec(value);
	}
	return 0;
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("Without argument\n");
		return 0;
	} else {
		if(strcmp(arg,"r") == 0) {
			printf("----------register begin----------\n");
			printf("eax: 0x%08x\t\t\tebx: 0x%08x\n", cpu.eax, cpu.ebx);
			printf("ecx: 0x%08x\t\t\tedx: 0x%08x\n", cpu.ecx, cpu.edx);
			printf("esp: 0x%08x\t\t\tebp: 0x%08x\n", cpu.esp, cpu.ebp);
			printf("esi: 0x%08x\t\t\tedi: 0x%08x\n", cpu.esi, cpu.edi);
			printf("eip: 0x%08x\n", cpu.eip);
			printf("----------register end----------\n");
			return 0;
		} 
		if(strcmp(arg, "w") == 0) {
			printf("need to print watchpoint\n");
			return 0;
		}
		printf("wrong arguments\n");
		return 0;
	}
}

static int cmd_x(char *args)
{
	char *N_char = strtok(NULL, " ");
	char *expr_char = strtok(NULL, " ");
	if((N_char == NULL)||(expr_char == NULL))
	{
		printf("wrong argument number\n");
		return 0;
	}
	char *tmp;
	int N = strtol(N_char, &tmp, 10);
	unsigned int start_address = strtol(expr_char, &tmp, 16);
	printf("----------memory begin----------\n");
	for(int i = 0; i<4*N; i+=4)
	{
		unsigned int curr = start_address + i;
		printf("0x%08x: ", curr);
		printf("0x%08x\n", vaddr_read(curr, 4));
	}
	printf("----------memoey end----------\n");
	return 0;
}

static int cmd_p(char *args){
	char *arg = strtok(args, " ");
	if(arg == NULL){
		printf("without argument\n");
		return 0;
	}else{
		int result = 0;
		uint32_t value = expr(arg, &result);
		/*
		 * result=0:success;
		 * result=1:make token error;
		 * result=2:calculate failed p>q;
		 * result=3:the right are more than left
		 * result=4:the left are more than right
		 */
		if(result==0){
			return value;
		}else{
			printf("error: ");
			switch (result) {
				case 1:
					printf("make token error\n");
					return 0;
				case 2:
					printf("calculate failed p>q\n");
					return 0;
				case 3:
					printf("the right are more than left\n");
					return 0;
				case 4:
					printf("the left are more than right\n");
					return 0;
				case 5:
					printf("cannot find the reg\n");
					return 0;
				case 6:
					printf("access memory out of bound\n");
				default:return 0;
			}
			return 0;
		}
	}
	return 0;
}
void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
