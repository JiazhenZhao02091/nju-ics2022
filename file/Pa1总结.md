# Pa1总结

## Pa1.1

### 在开始愉快的PA之旅之前

​	本次实验是基于`riscv32`来完成的，如果选择的其他ISA，可以参考思路

### 开天辟地的篇章

​	计算机可以没有寄存器吗？（建议二周目思考）--等待二周目补充

​	尝试理解计算机如何计算？画出1+2+...+100的状态机

> ​	(0,x ,x )->(1,0 ,x )->(2,0 ,0 )->(3,0 ,1 )->(4,1 ,1 )->(5,1 ,2 )->(6,3 ,2 )->(7,3 ,3 )->(8,6 ,3 )->(9,6 ,4 )->(10,10 ,4 )...依次类推（两次一循环，r2+1,r1+=r2）

### RTFSC

​	在`cmd_c()`函数中, 调用`cpu_exec()`的时候传入了参数`-1`, 你知道这是什么意思吗?

`cmd_c`函数在`monitior/sdb/sdb.c`中定义，我们可以追踪`cpu_exec()`函数，可以在`CPU`相关代码中找到如下函数定义

```c
/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
    g_print_step = (n < MAX_INST_TO_PRINT);
    switch (nemu_state.state) {
        case NEMU_END: case NEMU_ABORT:
            printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
            return;
        default: nemu_state.state = NEMU_RUNNING;
    }
    uint64_t timer_start = get_time();
    execute(n);
 ...
    }
}
```

​	可以发现继续调用了函数`execute`

```c
static void execute(uint64_t n) {
    Decode s;
    for (;n > 0; n --) {
        exec_once(&s, cpu.pc);
        g_nr_guest_inst ++;
        trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) break;
        IFDEF(CONFIG_DEVICE, device_update());
    }
}
```

​	因此当传入`-1`的时候，`for`循环不会执行，因此直接在`cpu_exec`函数中执行后续代码，而不会进行新的指令。



**为了测试大家是否已经理解框架代码, 我们给大家设置一个练习: 如果在运行NEMU之后直接键入`q`退出, 你会发现终端输出了一些错误信息. 请分析这个错误信息是什么原因造成的, 然后尝试在NEMU中修复它.**

​	这里我们可以先去观察输入`q`之后会发生什么？

```c
static int cmd_q(char *args) {
    return -1;
}
```

​	它会直接返回`-1`，我们回到`main_loop`中

```c
int i;
for (i = 0; i < NR_CMD; i ++) {
    if (strcmp(cmd, cmd_table[i].name) == 0) {
    if (cmd_table[i].handler(args) < 0) { return; }
        break;
    }
}
```

​	从这段代码中，我们可以发现返回值小于0的时候会直接`return`我们继续追踪，`return`之后回到了`nemu-main.c`中

```c
/* Start engine. */
  engine_start();
  return is_exit_status_bad();
}
```

​	其中`engine_start()`就是启动`main_loop`的函数，我们可以发现返回的是`is_exit_status_bad`我们可以在`utils`中找到他

```c
int is_exit_status_bad() {
        printf("nemu_state = %d\n",nemu_state.state);
  int good = (nemu_state.state == NEMU_END && nemu_state.halt_ret == 0) ||
    (nemu_state.state == NEMU_QUIT);
  return !good;
}
```

​	因此我们只需要在`cmd_q`函数中将`NEMU_STATE`设置为`NEMU_QUIT`即可,代码如下

```c
static int cmd_q(char *args) {
    nemu_state.state = NEMU_QUIT;
    return -1;
}
```

### 基础设施

在这里以及接下一个小章节我们需要实现如下几个基本功能：

| 命令         | 格式          | 使用举例          | 说明                                                         |
| ------------ | ------------- | ----------------- | ------------------------------------------------------------ |
| 帮助(1)      | `help`        | `help`            | 打印命令的帮助信息                                           |
| 继续运行(1)  | `c`           | `c`               | 继续运行被暂停的程序                                         |
| 退出(1)      | `q`           | `q`               | 退出NEMU                                                     |
| 单步执行     | `si [N]`      | `si 10`           | 让程序单步执行`N`条指令后暂停执行, 当`N`没有给出时, 缺省为`1` |
| 打印程序状态 | `info SUBCMD` | `info r` `info w` | 打印寄存器状态 打印监视点信息                                |
| 扫描内存(2)  | `x N EXPR`    | `x 10 $esp`       | 求出表达式`EXPR`的值, 将结果作为起始内存 地址, 以十六进制形式输出连续的`N`个4字节 |
| 表达式求值   | `p EXPR`      | `p $eax + 1`      | 求出表达式`EXPR`的值, `EXPR`支持的 运算请见[调试中的表达式求值](https://nju-projectn.github.io/ics-pa-gitbook/ics2022/1.6.html)小节 |
| 设置监视点   | `w EXPR`      | `w *0x2000`       | 当表达式`EXPR`的值发生变化时, 暂停程序执行                   |
| 删除监视点   | `d N`         | `d 2`             | 删除序号为`N`的监视点                                        |

#### 单步执行

​	这里直接给出代码，直接调用`cpu_exec`函数即可

```c
static int cmd_si(char *args){
    int step = 0;
    if(args == NULL)
        step = 1;
    else
        sscanf(args,"%d",&step);// 读入 Step
    cpu_exec(step);
    return 0;
}
```

#### 打印寄存器

​	我们经过`RTFSC`之后，可以发现在`reg.c`文件中定义了当前指令集的寄存器结构，同时在`isa-def`中可以找到`CPU_state`的定义

```c
typedef struct {
  word_t gpr[32];       // general purpose register;
  vaddr_t pc;
} riscv32_CPU_state;
```

​	在这里`gpr`的值就代表了寄存器对应的值，因此我们可以得出打印寄存器的值的函数如下:

```c
void isa_reg_display() {
    int length =  sizeof(regs) / sizeof(regs[0]);
    for(int i = 0  ; i < length ; i ++)
        printf("reg$%s ---> %d\n",regs[i], cpu.gpr[i]);
}
```

​	同时需要在`sdb.c`中定义`info r`

```c
static int cmd_info(char *args){
    if(args == NULL)
        printf("No args.\n");
    else if(strcmp(args, "r") == 0)
        isa_reg_display();
    else if(strcmp(args, "w") == 0)
        sdb_watchpoint_display();
    return 0;
}
```

#### 扫描内存

​	为了实现扫描内存的功能，我们可以在`memory`相关的代码中找到`paddr_read`这个函数，接下来我们只需要处理输入参数即可

```c
static int cmd_x(char *args){
    char* n = strtok(args," ");
    char* baseaddr = strtok(NULL," ");
    int len = 0;
    paddr_t addr = 0;
    sscanf(n, "%d", &len);
    sscanf(baseaddr,"%x", &addr);
    for(int i = 0 ; i < len ; i ++)
    {
        printf("%x\n",paddr_read(addr,4));//addr len
        addr = addr + 4;
    }
    return 0;
}
```

**至此pa1阶段1结束**

------

## Pa1.2

### 表达式求值

#### 词法分析

​	词法分析需要实现的功能就是将一个字符串表达式分解成我们定义的token类型，及`type,val_str`的类型

```c
typedef struct token {
    int type;
    char str[32];
} Token;
```

​	通过阅读手册，我们发现词法分析的过程是根据一个个自定义的正则表达式规则来的，因此我们先定义规则`rules`，这里直接给出参考代码

```c
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
    LEQ = 10,
    YINYONG = 11,
    POINT, NEG
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

    {"\\<\\=", LEQ},            // TODO
    {"\\=\\=", EQ},        // equal
    {"\\!\\=", NOTEQ},

    {"\\|\\|", OR},       // Opetor
    {"\\&\\&", AND},
    {"\\!", '!'},

    //{"\\$[a-z]*", RESGISTER},
    {"\\$[a-zA-Z]*[0-9]*", RESGISTER},
    {"0[xX][0-9a-fA-F]+", HEX},
    {"[0-9]*", NUM},

};

```

​	这里代码没什么复杂的地方，主要就是根据不同的存储类型对应上不同的正则表达式以及`type`类型，但是一定要注意**先后顺不同**以及**转义符号的使用**

​	在定义好规则之后，我们直接在原本的`make_tokens`代码上进行修改即可，参考代码如下

```c
static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '\0') {
        /* Try all rules one by one. */
        for (i = 0; i < NR_REGEX; i ++) {
            if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
                //      char *substr_start = e + position;
                int substr_len = pmatch.rm_eo;
                /*
                   Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
                   i, rules[i].regex, position, substr_len, substr_len, substr_start);
                   */

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
                        nr_token++;case 6:
                        tokens[nr_token].type = 6;
                        strcpy(tokens[nr_token].str, "||");
                        nr_token++;
                        break;
                    case 7:
                        tokens[nr_token].type = 7;
                        strcpy(tokens[nr_token].str, "&&");
                        nr_token++;
                        break;
                    case 10:
                        tokens[nr_token].type = 10;
                        strcpy(tokens[nr_token].str, "<=");
                        nr_token ++;
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

```

​	这里简单解释一下代码，代码的执行逻辑是把原始字符串进行分割，逐个匹配我们自定义的`rules`规则，匹配成功之后，就进行处理。在我们的`tokens`数组之中加入我们匹配成功后的`token`，主要包括它的类型以及记录的值。但是这里的匹配一定要对应之前定义的规则。

#### 递归求值

​	通过上述的表达式分解之后，我们就可以开始进行表达式求值了，先给出`check_parentheses()`参考代码，就是一个很简单的双指针算法

```c
bool check_parentheses(int p, int q)
{
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
```

​	从`expr.c`的函数中我们发现，所有的表达式行为都是从`expr`函数开始的，同时我们发现执行计算的函数是`eval`,这里直接给出参考代码

```c
uint32_t eval(int p, int q) {
    if (p > q) {
        /* Bad expression */
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
            if(!flag && tokens[i].type == 6){
                flag = true;
                op = max(op,i);
            }

            if(!flag && tokens[i].type == 7 ){
				flag = true;
                op = max(op,i);
            }

            if(!flag && tokens[i].type == 5){
                flag = true;
                op = max(op,i);
            }

            if(!flag && tokens[i].type == 4){
                flag = true;
                op = max(op,i);
            }
            if(!flag && tokens[i].type == 10){
                flag = true;
                op = max(op, i);
            }
            if(!flag && (tokens[i].type == '+' || tokens[i].type == '-')){
                flag = true;
                op = max(op, i);
            }
            if(!flag && (tokens[i].type == '*' || tokens[i].type == '/') ){
                op = max(op, i);
            }
        }
        //      printf("op position is %d\n", op);
        // if register return $register
        int  op_type = tokens[op].type;

        // 递归处理剩余的部分
        uint32_t  val1 = eval(p, op - 1);
        uint32_t  val2 = eval(op + 1, q);
        //      printf("val1 = %d, val2 = %d \n", val1, val2);

        switch (op_type) {
            case '+':
                return val1 + val2;
            case '-':
                return val1 - val2;
            case '*':
                return val1 * val2;
            case '/':
                if(val2 == 0){//printf("division can't zero;\n");
                    division_zero = true;
                    return 0;
                }
                return val1 / val2;
            case 4:
                return val1 == val2;
            case 5:
                return val1 != val2;
            case 6:
                return val1 || val2;
            case 7:
                return val1 && val2;
            default:
                printf("No Op type.");
                assert(0);
        }
    }
}
```

​	下面我们来分析这段代码，在条件判断时我们可以发现如果`p>q`代表代码出现了错误，直接报错即可；当`p==q`的时候代表我们需要取出其中的值，因此我们可以使用`atoi`函数直接将字符数组转为`int`即可；当`p,q`通过括号校验的时候，我们直接递归处理括号里面的部分即可；最后就是进入了计算过程，在这里我们需要根**据运算符的优先级得出主运算符的位置**，这里的实现思路是通过记录一个标记`flag`的值，然后再遍历的时候，根据优先级来进行`if`判断，优先级低的先记录同时将`flag`的值进行改变，这样最终就能得出主运算符的位置同时也保证了**低优先级以及最后出现的运算符为主运算符**；之后就进行计算处理即可。

​	但是在这里我们为了处理一些特殊值（负数，十六进制数等），我们需要再`make_tokens`之后进行预处理，主要代码如下：

```c
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
     * Init the tokens regex
     * TODO
     *
     */
    for(int i = 0 ; i < tokens_len ; i ++)
    {
	if(tokens[i].type == 2)
	{
	    bool flag = true;
	    int tmp = isa_reg_str2val(tokens[i].str, &flag);
	    if(flag){
		int2char(tmp, tokens[i].str); // transfrom the str --> $egx
	    }else{
		printf("Transfrom error. \n");
		assert(0);
	    }
	}
    }
    /*
     * Init the tokens HEX
     */
    for(int i = 0 ; i < tokens_len ; i ++)
    {
        if(tokens[i].type == 3)// Hex num
        {
            int value = strtol(tokens[i].str, NULL, 16);
            int2char(value, tokens[i].str);
        }
    }
    /*
     * Init the tokens str. 1 ==> -1.
     *
     */
    for(int i = 0 ; i < tokens_len ; i ++)
    {
	if((tokens[i].type == '-' && i > 0 && tokens[i-1].type != NUM && tokens[i+1].type == NUM)
		||
		(tokens[i].type == '-' && i == 0)
	  )
	{
	    //printf("%s\n", tokens[i+1].str);
	    tokens[i].type = TK_NOTYPE;
	    //tokens[i].str = tmp;
	    for(int j = 31 ; j >= 0 ; j --){
		tokens[i+1].str[j] = tokens[i+1].str[j-1];
	    }
	    tokens[i+1].str[0] = '-';
	    // printf("%s\n", tokens[i+1].str);
	    for(int j = 0 ; j < tokens_len ; j ++){
		if(tokens[j].type == TK_NOTYPE)
		{
		    for(int k = j +1 ; k < tokens_len ; k ++){
			tokens[k - 1] = tokens[k];
		    }
		    tokens_len -- ;
		}
	    }
	}
    }

    /*
     * Init the tokens !
     * TODO 
     */
    for(int i = 0 ; i < tokens_len ; i ++)
    {
	if(tokens[i].type == '!')
	{
	    tokens[i].type = TK_NOTYPE;
	    int tmp = char2int(tokens[i+1].str);
	    if(tmp == 0){
		memset(tokens[i+1].str, 0 ,sizeof(tokens[i+1].str));
		tokens[i+1].str[0] = '1';
	    }
	    else{
		memset(tokens[i+1].str, 0 , sizeof(tokens[i+1].str));
	    }
	    for(int j = 0 ; j < tokens_len ; j ++){
		if(tokens[j].type == TK_NOTYPE)
		{
		    for(int k = j +1 ; k < tokens_len ; k ++){
			tokens[k - 1] = tokens[k];
		    }
		    tokens_len -- ;
		}
	    }
	}
    }
    /*
     * TODO
     * Jie yin yong
     * */
    for(int i = 0 ; i < tokens_len ; i ++)
    {
	if(	(tokens[i].type == '*' && i > 0 
		    && tokens[i-1].type != NUM && tokens[i-1].type != HEX && tokens[i-1].type != RESGISTER
		    && tokens[i+1].type == NUM 
		    )
                ||
		(tokens[i].type == '*' && i > 0
                    && tokens[i-1].type != NUM && tokens[i-1].type != HEX && tokens[i-1].type != RESGISTER
                    && tokens[i+1].type == HEX
                    )
		||
                (tokens[i].type == '*' && i == 0)
          )
		{
            tokens[i].type = TK_NOTYPE;
            int tmp = char2int(tokens[i+1].str);
            uintptr_t a = (uintptr_t)tmp;
            int value = *((int*)a);
            int2char(value, tokens[i+1].str);	    
            // 
            for(int j = 0 ; j < tokens_len ; j ++){
                if(tokens[j].type == TK_NOTYPE){
                    for(int k = j +1 ; k < tokens_len ; k ++){
                    tokens[k - 1] = tokens[k];
                }
                    tokens_len -- ;
                }
            }
		}
    }
```

​	上面的代码主要实现了预处理过程，希望读者们仔细细心理解并尝试自己实现，在上面我们使用了`char2int`以及`int2char`函数，下面也给出了代码：

```c
int char2int(char s[]){
    int s_size = strlen(s);
    int res = 0 ;
    for(int i = 0 ; i < s_size ; i ++)
    {
	res += s[i] - '0';
	res *= 10;
    }
    res /= 10;
    return res;
}
void int2char(int x, char str[]){
    int len = strlen(str);
    memset(str, 0, len);
    int tmp_index = 0;
    int tmp_x = x;
    int x_size = 0, flag = 1;
    while(tmp_x){
	tmp_x /= 10;
	x_size ++;
	flag *= 10;
    }
    flag /= 10;
    while(x)
    {
	int a = x / flag; 
	x %= flag;
	flag /= 10;
	str[tmp_index ++] = a + '0';
    }
}
```

​	在完成词法分析以及递归求值功能之后，我们就可以在`main_loop`中调用了

```c
static int cmd_p(char* args){
    if(args == NULL){
        printf("No args\n");
        return 0;
    }
    //  printf("args = %s\n", args);
    bool flag = false;
    expr(args, &flag);
    return 0;
}

```

​	至此，就完成了表达式求值的功能

### 如何测试你的代码

​	这里直接给出`gen-expr.c`的参考代码，主要就是代码逻辑，没什么复杂度

```c
int choose(int n){
    int flag = rand() % 3 ; // 0 1 2
	printf("index = %d, flag = %d. \n",index_buf, flag);
    return flag;
}
void gen_num(){
    int num = rand()% 100;
    int num_size = 0, num_tmp = num;
    while(num_tmp){
	num_tmp /= 10;
	num_size ++;
    }
    int x = 1;
    while(num_size)
    {
	x *= 10;
	num_size -- ;
    }
    x /= 10;
    while(num)
    {
	char c = num / x + '0';
	num %= x;
	x /= 10;
	buf[index_buf ++] = c;
    }
}
void gen(char c){
    buf[index_buf ++] = c;
}
void gen_rand_op(){
    char op[4] = {'+', '-', '*', '/'};
    int op_position = rand() % 4;
    buf[index_buf ++] = op[op_position];
}

static void gen_rand_expr() {
    //    buf[0] = '\0';	
   if(index_buf > 65530)
       	printf("overSize\n");
    switch (choose(3)) {
	case 0:
	    gen_num();
	    break;
	case 1:
	    gen('(');
	    gen_rand_expr();
	    gen(')');
	    break;
	default:
	    gen_rand_expr();
	    gen_rand_op();
	    gen_rand_expr();
	    break;
    }
}
```

**至此pa1.2完成**

------

## PA1.3

### 扩展表达式求值的功能

​	这个在之前已经实现了，可以会看参考

### 实现监视点

​	我们需要先对监视点的结构体进行补充

```c
typedef struct watchpoint {
    int NO;
    struct watchpoint *next;
    //  TODO: Add more members if necessary 
    bool flag; // use / unuse
    char expr[100];
    int new_value;
    int old_value;
} WP;
```

​	这里主要添加了一个`flag`来记录是否使用，`new_value`和`old_value`来记录表达式前后的值

​	之后就来实现`new_wp()`和`free_wp(WP *wp)`函数，分别代表获取一个空闲节点和释放节点，参考代码如下：

```c
WP* new_wp(){
    for(WP* p = free_ ; p -> next != NULL ; p = p -> next){
        if( p -> flag == false){
            p -> flag = true;
            if(head == NULL){
                head = p;
            }
            return p;
        }
    }
    printf("No unuse point.\n");
    assert(0);
    return NULL;
}
void free_wp(WP *wp){
    if(head -> NO == wp -> NO){
        head -> flag = false;
        head = NULL;
        printf("Delete watchpoint  success.\n");
        return ;
    }
    for(WP* p = head ; p -> next != NULL ; p = p -> next){
        if(p -> next -> NO  == wp -> NO)
        {
            p -> next = p -> next -> next;
            p -> next -> flag = false; // 没有被使用
            printf("free succes.\n");
            return ;
        }
    }
}
```

​	到这里我们就实现了基本函数，接下来就是监视器相关的代码，主要的增删查监视点代码如下：

```c
void sdb_watchpoint_display(){
    bool flag = true;
    for(int i = 0 ; i < NR_WP ; i ++){
        if(wp_pool[i].flag){
            printf("Watchpoint.No: %d, expr = \"%s\", old_value = %d, new_value = %d\n",
                    wp_pool[i].NO, wp_pool[i].expr,wp_pool[i].old_value, wp_pool[i].new_value);
                flag = false;
        }
    }
    if(flag) printf("No watchpoint now.\n");
}
void delete_watchpoint(int no){
    for(int i = 0 ; i < NR_WP ; i ++)
        if(wp_pool[i].NO == no){
            free_wp(&wp_pool[i]);
            return ;
        }
}
void create_watchpoint(char* args){
    WP* p =  new_wp();
    strcpy(p -> expr, args);
    bool success = false;
    int tmp = expr(p -> expr,&success);
   if(success) p -> old_value = tmp;
   else printf("创建watchpoint的时候expr求值出现问题\n");
    printf("Create watchpoint No.%d success.\n", p -> NO);
}
```

​	同时调用代码如下：

```c
static int cmd_info(char *args){
    if(args == NULL)
        printf("No args.\n");
    else if(strcmp(args, "r") == 0)
        isa_reg_display();
    else if(strcmp(args, "w") == 0)
        sdb_watchpoint_display();
    return 0;
}
static int cmd_d (char *args){
    if(args == NULL)
        printf("No args.\n");
    else{
        delete_watchpoint(atoi(args));
    }
    return 0;
}
static int cmd_w(char* args){
    create_watchpoint(args);
    return 0;
}
```

​	为了每次在`CPU`运行一次之后都进行一次检查，这里我们在`cpu`中的`trace_and_difftest`函数中进行修改，参考代码如下：

```c
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
    if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
    // Scan all watchpoint.
    for(int i = 0 ; i < NR_WP; i ++){
        if(wp_pool[i].flag)
        {
            bool success = false;
            int tmp = expr(wp_pool[i].expr,&success);
            if(success){
                if(tmp != wp_pool[i].old_value)
                {
                    nemu_state.state = NEMU_STOP;
                    printf("NO EQ\n");
                    return ;
                }
            }
            else{
                printf("expr error.\n");
                assert(0);
            }
        }
    }
}
```

### 如何阅读手册

​	状态机在文章就已经给出了

RTFM 理解了科学查阅手册的方法之后, 请你尝试在你选择的ISA手册中查阅以下问题所在的位置, 把需要阅读的范围写到你的实验报告里面:

- riscv32
  - riscv32有哪几种指令格式?
    - 6种，R/I/S/U/B/J
  - LUI指令的行为是什么?
    - 用于将一个立即数加载到目标寄存器的高位
  - mstatus寄存器的结构是怎么样的?
    - 存储寄存器状态，详情见`VolumeⅡ`

​	`	shell`统计代码行数，在`scripts/native.mk`文件的最后添加如下代码：

```makefile
count:

        @echo "Counting functions in .c and .h files..."
        @find . \( -name "*.c" -o -name "*.h" \) -exec cat {} + | grep -c '.*'

countNoun:

        @echo "Counting functions in .c and .h files...No Have Space"
        @find . \( -name "*.c" -o -name "*.h" \) -exec cat {} + | grep -cE '^.+$$'

```

​	注意缩进，不要使用空格，统一用`tab`，保存退出后可以使用`make count`和`make countNoun`来统计含与不含空行的代码数量

​	**请解释gcc中的`-Wall`和`-Werror`有什么作用? 为什么要使用`-Wall`和`-Werror`?**

​	这里的`-Wall`和`-Werror`是为了启用所有的警告信息，以及将警告信息转为错误信息，作用是为了方便调试，减少`Bug`的出现