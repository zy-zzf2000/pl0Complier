/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0,  Visual C++.NET and
 * Visual C++.NET 2003,  on Win98,  WinNT,  Win2000,  WinXP and Win2003
 *
 */

//typedef enum {
//    false,
//    true
//} bool;
#include <vector>
class Partition_Table{    //空闲&占用分区表
public:
    int id;  //分区序号
    int size;//分区大小
    int addr;//分区起始地址
};

std::vector<Partition_Table> free_table;   //空闲分区表
std::vector<Partition_Table> used_table;   //分配分区表

#define norw 20     /* 关键字个数,13->15新增了forsym和tosym */  /*16新增downtosym*/    /*17新增breaksym*/   /*18新增continuesym*/  /*20新增ptr和malloc*/
#define txmax 100   /* 名字表容量 */
#define nmax 14     /* number的最大位数 */
#define al 10       /* 符号的最大长度 */
#define amax 2047   /* 地址上界*/
#define levmax 3    /* 最大允许过程嵌套声明层数 [0,  levmax]*/
#define cxmax 500   /* 最多的虚拟机代码数 */

//新增宏，定义最大的循环嵌套数为8
#define max_loop  8
#define strmaxlen 50  // 字符串最大长度
#define heapsize 500 //堆空间大小

//新增全局变量
int cur_loop_num;   //定义当前的循环数
int break_cx;   //存放break语句的指令地址
bool contains_break[max_loop];   //用于判断某层循环是否含有break语句

char cur_str[strmaxlen];   //存放字符串

int continue_cx;   //存放continue语句的指令地址
bool contains_continue[max_loop];   //用于判断某层循环是否含有continue语句

float heap[heapsize];   //堆
int heap_ptr=0;


/* 符号 */
//格式化输入：新增str
//for循环，新增forsym
//for循环，新增tosym
//for循环，新增downtosym
//动态内存分配，新增ptrsym
//动态内存分配，新增mallocsym
//支持指针操作，新增lfang和rfang
enum symbol {
    nul,         ident,     number,     plus,      minus,
    times,       slash,     oddsym,     eql,       neq,
    lss,         leq,       gtr,        geq,       lparen,
    rparen,      comma,     semicolon,  period,    becomes,
    beginsym,    endsym,    ifsym,      thensym,   whilesym,
    writesym,    readsym,   dosym,      callsym,   constsym,
    varsym,      procsym,

    str,        forsym,    tosym,
    downtosym,   breaksym,  continuesym,ptrsym,    mallocsym,
    lfang,       rfang
};
#define symnum 42

/* 名字表中的类型 */
enum object {
    constant,
    variable,
    procedur,
    ptr,
};

/* 虚拟机代码 */
enum fct {
    lit,     opr,     lod,
    sto,     cal,     inte,
    jmp,     jpc,
    dya,   //用于开辟栈空间
    sth,   //用于向堆空间中存储数据
    ldh,   //用于从堆空间中读出数据
};
#define fctnum 8

/* 虚拟机代码结构 */
struct instruction
{
    enum fct f; /* 虚拟机代码指令 */
    int l;      /* 引用层与声明层的层次差 */
    float a;      /* 根据f的不同而不同 */
};

FILE* fas;  /* 输出名字表 */
FILE* fa;   /* 输出虚拟机代码 */
FILE* fa1;  /* 输出源文件及其各行对应的首地址 */
FILE* fa2;  /* 输出结果 */
bool listswitch;    /* 显示虚拟机代码与否 */
bool tableswitch;   /* 显示名字表与否 */
char ch;            /* 获取字符的缓冲区，getch 使用 */
enum symbol sym;    /* 当前的符号 */
char id[al+1];      /* 当前ident, 多出的一个字节用于存放0 */
float num;            /* 当前number */
int cc, ll;          /* getch使用的计数器，cc表示当前字符(ch)的位置 */
int cx;             /* 虚拟机代码指针, 取值范围[0, cxmax-1]*/
char line[81];      /* 读取行缓冲区 */
char a[al+1];       /* 临时符号, 多出的一个字节用于存放0 */
struct instruction code[cxmax]; /* 存放虚拟机代码的数组 */
char word[norw][al];        /* 保留字 */
enum symbol wsym[norw];     /* 保留字对应的符号值 */
enum symbol ssym[256];      /* 单字符的符号值 */
char mnemonic[fctnum][5];   /* 虚拟机代码指令名称 */
bool declbegsys[symnum];    /* 表示声明开始的符号集合 */
bool statbegsys[symnum];    /* 表示语句开始的符号集合 */
bool facbegsys[symnum];     /* 表示因子开始的符号集合 */

/* 名字表结构 */
struct tablestruct
{
    char name[al];      /* 名字 */
    enum object kind;   /* 类型：const, var, array or procedure */
    int val;            /* 数值，仅const使用 */
    int level;          /* 所处层，仅const不使用 */
    int adr;            /* 地址，仅const不使用 */
    int size;           /* 需要分配的数据区空间, 仅procedure使用 */
};


struct tablestruct table[txmax]; /* 名字表 */

FILE* fin;
FILE* fout;
char fname[al];
int err; /* 错误计数器 */

/* 当函数中会发生fatal error时，返回-1告知调用它的函数，最终退出程序 */
#define getsymdo                      if(-1 == getsym()) return -1
#define getchdo                       if(-1 == getch()) return -1
#define testdo(a, b, c)               if(-1 == test(a, b, c)) return -1
#define gendo(a, b, c)                if(-1 == gen(a, b, c)) return -1
#define expressiondo(a, b, c)         if(-1 == expression(a, b, c)) return -1
#define factordo(a, b, c)             if(-1 == factor(a, b, c)) return -1
#define termdo(a, b, c)               if(-1 == term(a, b, c)) return -1
#define conditiondo(a, b, c)          if(-1 == condition(a, b, c)) return -1
#define statementdo(a, b, c)          if(-1 == statement(a, b, c)) return -1
#define constdeclarationdo(a, b, c)   if(-1 == constdeclaration(a, b, c)) return -1
#define vardeclarationdo(a, b, c)     if(-1 == vardeclaration(a, b, c)) return -1

void error(int n);
int getsym();
int getch();
void init();
int gen(enum fct x, int y, float z);
int test(bool* s1, bool* s2, int n);
int inset(int e, bool* s);
int addset(bool* sr, bool* s1, bool* s2, int n);
int subset(bool* sr, bool* s1, bool* s2, int n);
int mulset(bool* sr, bool* s1, bool* s2, int n);
int block(int lev, int tx, bool* fsys);
void interpret();
int factor(bool* fsys, int* ptx, int lev);
int term(bool* fsys, int* ptx, int lev);
int condition(bool* fsys, int* ptx, int lev);
int expression(bool* fsys, int* ptx, int lev);
int statement(bool* fsys, int* ptx, int lev);
void listcode(int cx0);
int vardeclaration(int* ptx, int lev, int* pdx);
int constdeclaration(int* ptx, int lev, int* pdx);
int position(char* idt, int tx);
void enter(enum object k, int* ptx, int lev, int* pdx);
int base(int l, float* s, int b);
