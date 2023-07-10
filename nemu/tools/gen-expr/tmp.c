#include <stdint.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

static int count=0;
static char buf[60000];
// %s不能写入65535，最大写入65484。
static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int choose(unsigned int i) {
  return rand()%i;
}

static void gen_num() {
  int i=choose(65536);
  sprintf(buf+count, "%d", i);
  while(buf[count]) {
    count++;
  }
}

static void gen_rand_op() {
  switch(choose(4)) {
    case 0:
      sprintf(buf+count, "%c", '+');
      break;
    case 1: 
      sprintf(buf+count, "%c", '-');
      break;
    case 2:
      sprintf(buf+count, "%c", '*');
      break;
    case 3:
      sprintf(buf+count, "%c", '/');
      break;
  }
  count++;
}

static void gen(char c) {
  sprintf(buf+count, "%c", c);
  count++;
}

static inline void gen_rand_expr() {
//  buf[0] = '\0';
  // 这里不删掉会报错，因为循环
  int i = choose(3);
  if(count>20) { i = 0; }
  // 防止表达式太长
  switch (i) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  // time(0)返回当前时间戳，如果失败返回0。
  srand(seed);
  // srand是随机数的初始化函数，通过参数生成伪随机数赋值给rand。
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  // 把程序执行时的第一个参数存入loop
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    sprintf(code_buf, code_format, buf);
    // 把code_format计算器和buf表达式写入code_buf
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    // 把code_buf的数据存入文件
    fclose(fp);
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    // 执行计算器
    if (ret != 0) continue;
    // 执行成功返回0，否则继续循环
    fp = popen("/tmp/.expr", "r");
    // 返回执行结果到fp
    assert(fp != NULL);
    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);
    printf("%u %s\n", result, buf);
  }
  return 0;
}
