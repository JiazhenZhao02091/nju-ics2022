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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
    TYPE_I, TYPE_U, TYPE_S,
    TYPE_N, TYPE_J,// none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
/*
#define immJ() do { 
int imm_20 = (i >> 31) & 1;
int imm_10_1 = (i >> 21) & 0x3FF;
int imm_11 = (i >> 20) & 1;
int imm_12_19 = (i >> 12) & 0xFF;
 *imm = imm_20 << 19 | imm_10_1 << 9 | imm_11 << 8 | imm_12_19; 
 } while(0)
 */

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd     = BITS(i, 11, 7);
    /*
    printf("rs1 = %d %x , %d %x.\n",rs1,rs1,R(rs1),R(rs1));
    printf("rs2 = %d %x , %d %x.\n",rs2,rs2,R(rs2),R(rs2));
    printf("rd = %d %x , %d %x.\n",*rd,*rd,R(*rd),R(*rd));
    */
    switch (type) {
	case TYPE_I: src1R();          immI(); break;
	case TYPE_U:                   immU(); break;
	case TYPE_S: src1R(); src2R(); immS(); break;
	case TYPE_J:		   
		     int imm_20 = (i >> 31) & 1;
		     int imm_10_1 = (i >> 21) & 0x3FF;
		     int imm_11 = (i >> 20) & 1;
		     int imm_12_19 = (i >> 12) & 0xFF;
		     *imm = imm_20 << 19 | imm_12_19 << 11 | imm_11 << 10 | imm_10_1;
		     if(imm_20 == 0){
		     	*imm = *imm << 1;
		     	*imm -= 4;
		     }
		     else{
			*imm |= 0xFFF00000;
		     	*imm = *imm << 1;
			*imm -= 4;
		     }
		     break;
    }
}

static int decode_exec(Decode *s) {
    int rd = 0;
    word_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__ ; \
}

    INSTPAT_START();
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, 
	    Mw(src1 + imm, 4, src2));


    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd)=src1+imm);
    INSTPAT("? ?????????? ? ???????? ????? 11011 11", jal    , J , R(rd)=s->pc+4, s -> dnpc += imm);
INSTPAT("0000 0000 0000 0000 1000 0000 0110 0111", ret	,U	, s -> dnpc = R(1));
INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
INSTPAT_END();

R(0) = 0; // reset $zero to 0

return 0;
}

int isa_exec_once(Decode *s) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4); //s->snpc += 4
 //   Log("isa-riscv32-isa_exec_once : %x %x %x\n", s -> pc, s -> dnpc, s -> snpc);
    return decode_exec(s);
}
