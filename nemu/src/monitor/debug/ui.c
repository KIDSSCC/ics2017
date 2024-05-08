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

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Step instructions in the program", cmd_si },
  { "info", "Print the information of registers or watchpoints", cmd_info },
  { "x", "Examine memory", cmd_x },
  { "p", "Print the value of expression", cmd_p },
  { "w", "Set a watchpoint", cmd_w },
  { "d", "Delete a watchpoint", cmd_d },

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
  } else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    Log("Unknown command '%s'", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
  int n = 1;
  if (args != NULL) {
    n = atoi(args);
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args) {
  if (args == NULL) {
    Log("Invalid arguments");
    printf("info r - Print the information of registers\n");
    printf("info w - Print the information of watchpoints\n");
  } else if (strcmp(args, "r") == 0) {
    printf("eax: 0x%08x\n", cpu.eax);
    printf("ecx: 0x%08x\n", cpu.ecx);
    printf("edx: 0x%08x\n", cpu.edx);
    printf("ebx: 0x%08x\n", cpu.ebx);
    printf("esp: 0x%08x\n", cpu.esp);
    printf("ebp: 0x%08x\n", cpu.ebp);
    printf("esi: 0x%08x\n", cpu.esi);
    printf("edi: 0x%08x\n", cpu.edi);
    printf("eip: 0x%08x\n", cpu.eip);
  } else if (strcmp(args, "w") == 0) {
    show_wp();
  } else {
    Log("Unknown command '%s'", args);
    printf("info r - Print the information of registers\n");
    printf("info w - Print the information of watchpoints\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  int n = 0;
  uint32_t expression;
  if (args != NULL) {
    args = strtok(args, " ");
    n = atoi(args);
    args += strlen(args) + 1;
    if (*args != '\0') {
      bool success = true;
      expression = expr(args, &success);
      if (success) {
        for (int i = 0; i < n; i++) {
          printf("0x%08x: 0x%08x\n", expression + i * 4, vaddr_read(expression + i * 4, 4));
        }
      } else {
        Log("Invalid expression");
        printf("x N EXPR - Examine memory\n");
      }
    } else {
      Log("Invalid arguments");
      printf("x N EXPR - Examine memory\n");
    }
  } else {
    Log("Invalid arguments");
    printf("x N EXPR - Examine memory\n");
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args != NULL) {
    bool success = true;
    uint32_t result = expr(args, &success);
    if (success) {
      printf("0x%08x\n", result);
    } else {
      Log("Invalid expression");
      printf("p EXPR - Print the value of expression\n");
    }
  } else {
    Log("Invalid arguments");
    printf("p EXPR - Print the value of expression\n");
  }
  return 0;
}

static int cmd_w(char *args) {
  if (args != NULL) {
    bool success = true;
    uint32_t result = expr(args, &success);
    if (success) {
      WP* wp = new_wp();
      strcpy(wp->expr, args);
      wp->value = result;
      printf("Watchpoint %d: %s (0x%08x)\n", wp->NO, wp->expr, wp->value);
    } else {
      Log("Invalid expression");
      printf("w EXPR - Set a watchpoint\n");
    }
  } else {
    Log("Invalid arguments");
    printf("w EXPR - Set a watchpoint\n");
  }
  return 0;
}

static int cmd_d(char *args) {
  if (args != NULL) {
    int n = atoi(args);
    WP* wp = get_wp(n);
    if (wp != NULL) {
      free_wp(wp);
      printf("Watchpoint %d deleted\n", n);
    } else {
      Log("No such watchpoint");
    }
  } else {
    Log("Invalid arguments");
    printf("d N - Delete a watchpoint\n");
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
