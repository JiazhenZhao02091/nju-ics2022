#ifndef ITRACE_H
#define ITRACE_H

#define MAX_IRINGBUF 16 // MAX BUF INST SIZE

typedef struct {
  word_t pc;
  uint32_t inst;
} ItraceNode;



void trace_inst(word_t pc, uint32_t inst);
void display_inst();
void display_pread(paddr_t addr, int len);
void display_pwrite(paddr_t addr, int len, word_t data);
void parse_elf(const char *elf_file);
void trace_func_call(paddr_t pc, paddr_t target, bool is_tail);
void trace_func_ret(paddr_t pc);
#endif

