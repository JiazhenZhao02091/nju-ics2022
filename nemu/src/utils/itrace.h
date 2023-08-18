#ifndef ITRACE_H
#define ITRACE_H

#define MAX_IRINGBUF 16 // MAX BUF INST SIZE

typedef struct {
  word_t pc;
  uint32_t inst;
} ItraceNode;



void trace_inst(word_t pc, uint32_t inst);
void display_inst();

#endif

