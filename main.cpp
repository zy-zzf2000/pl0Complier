/*
 * PL/0 complier program for win32 platform (implemented in C)
 *
 * The program has been test on Visual C++ 6.0, Visual C++.NET and
 * Visual C++.NET 2003, on Win98, WinNT, Win2000, WinXP and Win2003
 *
 * 使用方法：
 * 运行后输入PL/0源程序文件?
 * 回答是否输出虚拟机代码
 * 回答是否输出名字表
 * fa.tmp输出虚拟机代码
 * fa1.tmp输出源文件及其各行对应的首地址
 * fa2.tmp输出结?
 * fas.tmp输出名字表
 */

#include <stdio.h>
#include <iostream>

#include "mypl0.h"
#include "string.h"
#include <iomanip>

/* 解释执行时使用的栈 */
#define stacksize 500

int main(int argc, char** argv)
{
    bool nxtlev[symnum];
//    std::cout<<"args:"<<argv[1];

//    printf("Input pl/0 file?   ");
//    scanf("%s", fname);     /* 输入文件名 */

    fin = fopen(argv[1], "r");

    if (fin)
    {
//        printf("List object code?(Y/N)");   /* 是否输出虚拟机代码 */
//        scanf("%s", fname);
        listswitch = false;

//        printf("List symbol table?(Y/N)");  /* 是否输出名字表 */
//        scanf("%s", fname);
        tableswitch =false;

        fa1 = fopen("fa1.tmp", "w");
        fprintf(fa1,"Input pl/0 file?   ");
        fprintf(fa1,"%s\n",fname);

        init();     /* 初始化 */

        err = 0;
        cc = cx = ll = 0;   //cc:行缓冲区指针  ,cx:代码分配指针，代码生成模块总在cx所指位置生成新的代码,
        ch = ' ';

        if(-1 != getsym())
        {
            fa = fopen("fa.tmp", "w");
            fas = fopen("fas.tmp", "w");
            addset(nxtlev, declbegsys, statbegsys, symnum);
            nxtlev[period] = true;

            if(-1 == block(0, 0, nxtlev))   /* 调用编译程序 */
            {
                fclose(fa);
                fclose(fa1);
                fclose(fas);
                fclose(fin);
                printf("\n");
                return 0;
            }
            fclose(fa);
            fclose(fa1);
            fclose(fas);

            if (sym != period)
            {
                error(9);
            }

            if (err == 0)
            {
                fa2 = fopen("fa2.tmp", "w");
                interpret();    /* 调用解释执行程序 */
                fclose(fa2);
            }
            else
            {
                printf("Errors in pl/0 program");
            }
        }

        fclose(fin);
    }
    else
    {
        printf("Can't open file!\n");
    }

    printf("\n");
    system("pause");
    return 0;
}

/*
* 初始化
*/
void init()
{
    int i;

    /* 设置单字符符号 */
    for (i=0; i<=255; i++)
    {
        ssym[i] = nul;
    }
    ssym['+'] = plus;
    ssym['-'] = minus;
    ssym['*'] = times;
    ssym['/'] = slash;
    ssym['('] = lparen;
    ssym[')'] = rparen;
    ssym['='] = eql;
    ssym[','] = comma;
    ssym['.'] = period;
    ssym['#'] = neq;
    ssym[';'] = semicolon;
    ssym['['] = lfang;
    ssym[']'] =rfang;

    /* 设置保留字名字,按照字母顺序，便于折半查找 */
    strcpy(&(word[0][0]), "begin");

    strcpy(&(word[1][0]), "break");     //break关键字

    strcpy(&(word[2][0]), "call");
    strcpy(&(word[3][0]), "const");

    strcpy(&(word[4][0]), "continue");    //continue关键字

    strcpy(&(word[5][0]), "do");

    strcpy(&(word[6][0]), "downto");    //downto关键字

    strcpy(&(word[7][0]), "end");

    strcpy(&(word[8][0]),"for");  //for语句关键字

    strcpy(&(word[9][0]), "if");

    strcpy(&(word[10][0]), "malloc");  //malloc关键字

    strcpy(&(word[11][0]), "odd");
    strcpy(&(word[12][0]), "procedure");
    strcpy(&(word[13][0]), "ptr");
    strcpy(&(word[14][0]), "read");
    strcpy(&(word[15][0]), "then");

    strcpy(&(word[16][0]),"to");   //to语句关键字

    strcpy(&(word[17][0]), "var");

    strcpy(&(word[18][0]), "while");

    strcpy(&(word[19][0]), "write");




    /* 设置保留字符号 */
    wsym[0] = beginsym;

    wsym[1] = breaksym; //break关键字

    wsym[2] = callsym;
    wsym[3] = constsym;

    wsym[4] = continuesym;

    wsym[5] = dosym;

    wsym[6] = downtosym;

    wsym[7] = endsym;

    wsym[8] = forsym;

    wsym[9] = ifsym;
    wsym[10] = mallocsym;
    wsym[11] = oddsym;
    wsym[12] = procsym;

    wsym[13] = ptrsym;

    wsym[14] = readsym;
    wsym[15] = thensym;

    wsym[16] = tosym;

    wsym[17] = varsym;
    wsym[18] = whilesym;
    wsym[19] = writesym;



    /* 设置指令名称 */
    strcpy(&(mnemonic[lit][0]), "lit");
    strcpy(&(mnemonic[opr][0]), "opr");
    strcpy(&(mnemonic[lod][0]), "lod");
    strcpy(&(mnemonic[sto][0]), "sto");
    strcpy(&(mnemonic[cal][0]), "cal");
    strcpy(&(mnemonic[inte][0]), "int");
    strcpy(&(mnemonic[jmp][0]), "jmp");
    strcpy(&(mnemonic[jpc][0]), "jpc");
    strcpy(&(mnemonic[dya][0]), "dya");
    strcpy(&(mnemonic[sth][0]), "sth");
    strcpy(&(mnemonic[ldh][0]), "ldh");

    /* 设置符号集 */
    for (i=0; i<symnum; i++)
    {
        declbegsys[i] = false;
        statbegsys[i] = false;
        facbegsys[i] = false;
    }

    /* 设置声明开始符号集 */
    declbegsys[constsym] = true;
    declbegsys[varsym] = true;
    declbegsys[procsym] = true;

    /* 设置语句开始符号集 */
    statbegsys[beginsym] = true;
    statbegsys[callsym] = true;
    statbegsys[ifsym] = true;
    statbegsys[whilesym] = true;
    statbegsys[readsym] = true;
    statbegsys[writesym] = true;

    statbegsys[forsym] = true;  //新增for符号集
    statbegsys[breaksym] = true;  //新增break符号集
    statbegsys[continuesym] = true;
    statbegsys[ptrsym] = true;

    /* 设置因子开始符号集 */
    facbegsys[ident] = true;
    facbegsys[number] = true;
    facbegsys[lparen] = true;

    cur_loop_num=0;   //初始的循环被置为0
}

/*
* 用数组实现集合的集合运算
*/
int inset(int e, bool* s)
{
    return s[e];
}

int addset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]||s2[i];
    }
    return 0;
}

int subset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&(!s2[i]);
    }
    return 0;
}

int mulset(bool* sr, bool* s1, bool* s2, int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        sr[i] = s1[i]&&s2[i];
    }
    return 0;
}

/*
*   出错处理，打印出错位置和错误编码
*/
void error(int n)
{
    char space[81];
    memset(space,32,81);

    space[cc-1]=0; //出错时当前符号已经读完，所以cc-1

    printf("****%s!%d\n", space, n);
    fprintf(fa1,"****%s!%d\n", space, n);
    switch(n) {
        case 1 :
            printf("常量说明赋值符号错误!\n");
            break;
        case 100:
            printf("字符串超出最大长度！\n");
            break;
        case 101:
            printf("格式化输出未找到匹配标识符！\n");
            break;
        case 102:
            printf("无法识别的格式化输出！\n");
            break;
        case 103:
            printf("格式化输入未找到匹配标识符！\n");
            break;
        case 104:
            printf("格式化输入变量类型错误！\n");
            break;
        case 105:
            std::cout<<"错误的格式化输入类型，格式化输入必须为%d或%f"<<std::endl;
            break;
        case 106:
            std::cout<<"for语句后不得出现除赋值语句外的其他语句！"<<std::endl;
            break;
        case 107:
            std::cout<<"for语句缺少to或down to！"<<std::endl;
            break;
        case 108:
            std::cout<<"for语句缺少do！"<<std::endl;
            break;
        case 109:
            std::cout<<"循环嵌套超过最大限制！"<<std::endl;
            break;
        case 110:
            std::cout<<"在非循环语句内使用break语句！"<<std::endl;
            break;
        case 111:
            std::cout<<"在非循环语句内使用continue语句！"<<std::endl;
            break;
        case 112:
            std::cout<<"ptr缺少正确的标识符！"<<std::endl;
            break;
        case 113:
            std::cout<<"ptr赋值符号使用错误!"<<std::endl;
            break;
        case 114:
            std::cout<<"缺少malloc关键字!"<<std::endl;
            break;
        case 115:
            std::cout<<"缺少小括号!"<<std::endl;
            break;
        case 116:
            std::cout<<"栈空间不足！"<<std::endl;
            break;
        case 117:
            std::cout<<"数组调用错误！"<<std::endl;
            break;
    }
    err++;
}

/*
* 漏掉空格，读取一个字符。
*
* 每次读一行，存入line缓冲区，line被getsym取空后再读一行
*
* 被函数getsym调用。
*/
int getch()
{
    if (cc == ll)
    {
        if (feof(fin))
        {
            printf("program incomplete");
            return -1;
        }
        ll=0;
        cc=0;
        printf("%d ", cx);
        fprintf(fa1,"%d ", cx);
        ch = ' ';
        while (ch != 10)
        {
            //fscanf(fin,"%c", &ch)
            //richard
            if (EOF == fscanf(fin,"%c", &ch))
            {
                line[ll] = 0;
                break;
            }
            //end richard
            printf("%c", ch);
            fprintf(fa1, "%c", ch);
            line[ll] = ch;
            ll++;
        }
        printf("\n");
        fprintf(fa1, "\n");
    }
    ch = line[cc];
    cc++;
    return 0;
}

/*
* 词法分析，获取一个符号
*/
int getsym()
{
    int i,j,k;

    /* the original version lacks "\r", thanks to foolevery */
    while (ch==' ' || ch==10 || ch==13 || ch==9)  /* 忽略空格、换行、回车和TAB */
    {
        getchdo;
    }
    if (ch>='a' && ch<='z')
    {           /* 名字或保留字以a..z开头 */
        k = 0;
        do {
            if(k<al)
            {
                a[k] = ch;
                k++;
            }
            getchdo;
        } while (ch>='a' && ch<='z' || ch>='0' && ch<='9');
        a[k] = 0;
        strcpy(id, a);
        i = 0;
        j = norw-1;
        do {    /* 搜索当前符号是否为保留字 */
            k = (i+j)/2;
            if (strcmp(id,word[k]) <= 0)
            {
                j = k - 1;
            }
            if (strcmp(id,word[k]) >= 0)
            {
                i = k + 1;
            }
        } while (i <= j);
        if (i-1 > j)
        {
            sym = wsym[k];
        }
        else
        {
            sym = ident; /* 搜索失败则，是名字或数字 */
        }
    }
    else
    {
        if (ch>='0' && ch<='9')
        {           /* 检测是否为数字：以0..9开头 */
            k = 0;
            num = 0;
            sym = number;
            do {
                num = 10*num + ch - '0';
                k++;
                getchdo;
            } while (ch>='0' && ch<='9'); /* 获取数字的值 */

            if (ch=='.'){                  //浮点数扩增,小数点后为小数部分
                getchdo;
                float temp = 0.1;
                while(ch>='0'&& ch<='9'){       //读取小数部分
                    num = num + temp*(ch - '0');
                    temp = temp*0.1;
                    k++;
                    getchdo;
                }
            }


            k--;
            if (k > nmax)
            {
                error(30);
            }
        }

        else if(ch=='"'){     /* 检测是否为字符串  */
            sym = str;       //将当前词法单元类型置为字符串
            int i = 0;       //cur_str下标
            getchdo;
            while(ch!='"'  && i<strmaxlen){
                cur_str[i]=ch;
                i+=1;
                getchdo;
            }
            if(ch!='"'&&i==strmaxlen){
                error(100);
            }
            getchdo;
            cur_str[i]='\0';    //设置字符串最后一位
        }

        else
        {
            if (ch == ':')      /* 检测赋值符号 */
            {
                getchdo;
                if (ch == '=')
                {
                    sym = becomes;
                    getchdo;
                }
                else
                {
                    sym = nul;  /* 不能识别的符号 */
                }
            }
            else
            {
                if (ch == '<')      /* 检测小于或小于等于符号 */
                {
                    getchdo;
                    if (ch == '=')
                    {
                        sym = leq;
                        getchdo;
                    }
                    else
                    {
                        sym = lss;
                    }
                }
                else
                {
                    if (ch=='>')        /* 检测大于或大于等于符号 */
                    {
                        getchdo;
                        if (ch == '=')
                        {
                            sym = geq;
                            getchdo;
                        }
                        else
                        {
                            sym = gtr;
                        }
                    }
                    else
                    {
                        sym = ssym[ch];     /* 当符号不满足上述条件时，全部按照单字符符号处理 */
                        //getchdo;
                        //richard
                        if (sym != period)
                        {
                            getchdo;
                        }
                        //end richard
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 生成虚拟机代码
*
* x: instruction.f;
* y: instruction.l;
* z: instruction.a;
*/
int gen(enum fct x, int y, float z )
{
    if (cx >= cxmax)
    {
        printf("Program too long"); /* 程序过长 */
        return -1;
    }
    code[cx].f = x;
    code[cx].l = y;
    code[cx].a = z;
    cx++;
    return 0;
}


/*
* 测试当前符号是否合法
*
* 在某一部分（如一条语句，一个表达式）将要结束时时我们希望下一个符号属于某集?
* （该部分的后跟符号），test负责这项检测，并且负责当检测不通过时的补救措施，
* 程序在需要检测时指定当前需要的符号集合和补救用的集合（如之前未完成部分的后跟
* 符号），以及检测不通过时的错误号。
*
* s1:   我们需要的符号
* s2:   如果不是我们需要的，则需要一个补救用的集?
* n:    错误号
*/
int test(bool* s1, bool* s2, int n)
{
    if (!inset(sym, s1))
    {
        error(n);
        /* 当检测不通过时，不停获取符号，直到它属于需要的集合或补救的集合 */
        while ((!inset(sym,s1)) && (!inset(sym,s2)))
        {
            getsymdo;
        }
    }
    return 0;
}

/*
* 编译程序主?
*
* lev:    当前分程序所在层
* tx:     名字表当前尾指针
* fsys:   当前模块后跟符号集?
*/
int block(int lev, int tx, bool* fsys)
{
    int i;

    int dx;                 /* 名字分配到的相对地址 */
    int tx0;                /* 保留初始tx */
    int cx0;                /* 保留初始cx */
    bool nxtlev[symnum];    /* 在下级函数的参数中，符号集合均为值参，但由于使用数组实现，
							传递进来的是指针，为防止下级函数改变上级函数的集合，开辟新的空?
							传递给下级函数*/

    dx = 3;
    tx0 = tx;               /* 记录本层名字的初始位置 */
    table[tx].adr = cx;

    gendo(jmp, 0, 0);

    if (lev > levmax)
    {
        error(32);
    }

    do {

        if (sym == constsym)    /* 收到常量声明符号，开始处理常量声明 */
        {
            getsymdo;

            /* the original do...while(sym == ident) is problematic, thanks to calculous */
            /* do { */
            constdeclarationdo(&tx, lev, &dx);  /* dx的值会被constdeclaration改变，使用指针 */
            while (sym == comma)
            {
                getsymdo;
                constdeclarationdo(&tx, lev, &dx);
            }
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /*漏掉了逗号或者分号*/
            }
            /* } while (sym == ident); */
        }

        if (sym == varsym)      /* 收到变量声明符号，开始处理变量声明 */
        {
            getsymdo;

            /* the original do...while(sym == ident) is problematic, thanks to calculous */
            /* do {  */
            vardeclarationdo(&tx, lev, &dx);
            while (sym == comma)
            {
                getsymdo;
                vardeclarationdo(&tx, lev, &dx);
            }
            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);
            }
            /* } while (sym == ident);  */
        }

        while (sym == procsym) /* 收到过程声明符号，开始处理过程声明 */
        {
            getsymdo;

            if (sym == ident)
            {
                enter(procedur, &tx, lev, &dx); /* 记录过程名字 */
                getsymdo;
            }
            else
            {
                error(4);   /* procedure后应为标识符 */
            }

            if (sym == semicolon)
            {
                getsymdo;
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }

            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
            nxtlev[semicolon] = true;
            if (-1 == block(lev+1, tx, nxtlev))
            {
                return -1;  /* 递归调用 */
            }

            if(sym == semicolon)
            {
                getsymdo;
                memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
                nxtlev[ident] = true;
                nxtlev[procsym] = true;
                testdo(nxtlev, fsys, 6);
            }
            else
            {
                error(5);   /* 漏掉了分号 */
            }
        }
        memcpy(nxtlev, statbegsys, sizeof(bool)*symnum);
        nxtlev[ident] = true;
        testdo(nxtlev, declbegsys, 7);
    } while (inset(sym, declbegsys));   /* 直到没有声明符号 */

    code[table[tx0].adr].a = cx;    /* 开始生成当前过程代码 */
    table[tx0].adr = cx;            /* 当前过程代码地址 */
    table[tx0].size = dx;           /* 声明部分中每增加一条声明都会给dx增加1，声明部分已经结束，dx就是当前过程数据的size */
    cx0 = cx;
    gendo(inte, 0, dx);             /* 生成分配内存代码 */

    if (tableswitch)        /* 输出名字表 */
    {
        printf("TABLE:\n");
        if (tx0+1 > tx)
        {
            printf("    NULL\n");
        }
        for (i=tx0+1; i<=tx; i++)
        {
            switch (table[i].kind)
            {
                case constant:
                    printf("    %d const %s ", i, table[i].name);
                    printf("val=%d\n", table[i].val);
                    fprintf(fas, "    %d const %s ", i, table[i].name);
                    fprintf(fas, "val=%d\n", table[i].val);
                    break;
                case variable:
                    printf("    %d var   %s ", i, table[i].name);
                    printf("lev=%d addr=%d\n", table[i].level, table[i].adr);
                    fprintf(fas, "    %d var   %s ", i, table[i].name);
                    fprintf(fas, "lev=%d addr=%d\n", table[i].level, table[i].adr);
                    break;
                case procedur:
                    printf("    %d proc  %s ", i, table[i].name);
                    printf("lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
                    fprintf(fas,"    %d proc  %s ", i, table[i].name);
                    fprintf(fas,"lev=%d addr=%d size=%d\n", table[i].level, table[i].adr, table[i].size);
                    break;
            }
        }
        printf("\n");
    }

    /* 语句后跟符号为分号或end */
    memcpy(nxtlev, fsys, sizeof(bool)*symnum);  /* 每个后跟符号集和都包含上层后跟符号集和，以便补救 */
    nxtlev[semicolon] = true;
    nxtlev[endsym] = true;
    statementdo(nxtlev, &tx, lev);
    gendo(opr, 0, 0);                       /* 每个过程出口都要使用的释放数据段指令 */
    memset(nxtlev, 0, sizeof(bool)*symnum); /*分程序没有补救集合 */
    testdo(fsys, nxtlev, 8);                /* 检测后跟符号正确性 */
    listcode(cx0);                          /* 输出代码 */
    return 0;
}

/*
* 在名字表中加入一项
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev, int* pdx)
{
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
    table[(*ptx)].kind = k;
    switch (k)
    {
        case constant:  /* 常量名字 */
            if (num > amax)
            {
                error(31);  /* 数越界 */
                num = 0;
            }
            table[(*ptx)].val = num;
            break;
        case variable:  /* 变量名字 */
            table[(*ptx)].level = lev;
            table[(*ptx)].adr = (*pdx);
            (*pdx)++;
            break;
        case procedur:  /*　过程名字　*/
            table[(*ptx)].level = lev;
            break;
    }
}

/*
* 把指针加入到名字表中，重载enter函数
*
* k:      名字种类const,var or procedure
* ptx:    名字表尾指针的指针，为了可以改变名字表尾指针的值
* lev:    名字所在的层次,，以后所有的lev都是这样
* pdx:    dx为当前应分配的变量的相对地址，分配后要增加1
*/
void enter(enum object k, int* ptx, int lev,int size)
{
    (*ptx)++;
    strcpy(table[(*ptx)].name, id); /* 全局变量id中已存有当前名字的名字 */
    table[(*ptx)].kind = k;
    table[(*ptx)].size = size;
    table[(*ptx)].val = num;
    table[(*ptx)].level = lev;
}


/*
* 查找名字的位置.
* 找到则返回在名字表中的位置,否则返回0.
*
* idt:    要查找的名字
* tx:     当前名字表尾指针
*/
int position(char* idt, int tx)
{
    int i;
    strcpy(table[0].name, idt);
    i = tx;
    while (strcmp(table[i].name, idt) != 0)
    {
        i--;
    }
    return i;
}

/*
* 常量声明处理
*/
int constdeclaration(int* ptx, int lev, int* pdx)
{
    if (sym == ident)
    {
        getsymdo;
        if (sym==eql || sym==becomes)
        {
            if (sym == becomes)
            {
                error(1);   /* 把=写成了:= */
            }
            getsymdo;
            if (sym == number)
            {
                enter(constant, ptx, lev, pdx);
                getsymdo;
            }
            else
            {
                error(2);   /* 常量说明=后应是数字 */
            }
        }
        else
        {
            error(3);   /* 常量说明标识后应是= */
        }
    }
    else
    {
        error(4);   /* const后应是标识 */
    }
    return 0;
}

/*
* 变量声明处理
*/
int vardeclaration(int* ptx,int lev,int* pdx)
{
    if (sym == ident)
    {
        enter(variable, ptx, lev, pdx); // 填写名字表
        getsymdo;
    }
    else
    {
        error(4);   /* var后应是标识 */
    }
    return 0;
}

/*
* 输出目标代码清单
*/
void listcode(int cx0)
{
    int i;
    if (listswitch)
    {
        for (i=cx0; i<cx; i++)
        {
            printf("%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
            fprintf(fa,"%d %s %d %d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
        }
    }
}

/*
* 语句处理
*/
int statement(bool* fsys, int* ptx, int lev)
{
    int i, cx1, cx2;
    bool nxtlev[symnum];

    if (sym == ident)   /* 准备按照赋值语句处理 */
    {
        i = position(id, *ptx);
        if (i == 0)
        {
            error(11);  /* 变量未找到 */
        }
        else
        {
            if(table[i].kind != variable&&table[i].kind != ptr)
            {
                error(12);  /* 赋值语句格式错误 */
                i = 0;
            }
            else
            {
                getsymdo;
                if(sym == becomes)    //变量赋值
                {
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    expressiondo(nxtlev, ptx, lev); /* 处理赋值符号右侧表达式 */
                    if(i != 0)
                    {
                        /* expression将执行一系列指令，但最终结果将会保存在栈顶，执行sto命令完成赋值 */
                        gendo(sto, lev-table[i].level, table[i].adr);
                    }
                }
                else if (sym == lfang){   //数组赋值
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    nxtlev[rfang] = true;   //后跟符号为右括号
                    expressiondo(nxtlev, ptx, lev);    //处理赋值语句
                    getsymdo;
                    if(sym!=becomes){   //赋值语句错误
                        error(13);
                    }
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    expressiondo(nxtlev, ptx, lev);
                    gendo(sth,0,i);   //此时，栈顶保存的为需要输入的值，次栈顶保存的为偏移量
                }
                else
                {
                    error(13);  /* 没有检测到赋值符号 */
                }
            }
        }//if (i == 0)
    }
    else
    {
        if (sym == readsym) /* 准备按照read语句处理 */
        {
            getsymdo;
            if (sym != lparen)
            {
                error(34);  /* 格式错误，应是左括号 */
            }
            else
            {
                getsymdo;
                if(sym==str){    //新增格式化read处理
                    getsymdo;    //读出右边第一个单词，应当为一个逗号或右括号
                    for(int str_index=0;str_index<strmaxlen;str_index++){
                        if(cur_str[str_index]=='\0'){    //遇到字符串终结符
                            break;
                        }
                        else if(cur_str[str_index]=='%'){     //遇到格式化输入
                            str_index++;
                            getsymdo;             //获取该格式化输入对应的标识符
                            if(sym!=ident){
                                error(103);    //格式化输入没有找到匹配的ident
                            }

                            i = position(id, *ptx); /* 查找要读的变量 */
                            if (table[i].kind != variable){
                                error(104);
                            }
                            else if(i==0){
                                error(103);
                            }
                            else{
                                if(cur_str[str_index]=='d'){    //如果是整数，则使用整数读入指令
                                    gendo(opr, 0, 16);
                                }
                                else if(cur_str[str_index]=='f'){
                                    gendo(opr,0,19);      //如果是浮点数，则使用浮点数读入指令
                                }
                                else{
                                    error(105);
                                }
                                gendo(sto, lev-table[i].level, table[i].adr);   /* 储存到变量 */

                                getsymdo;    //获取下一个词法单元，应当为逗号或者右括号
                            }
                        }
                        else{   //如果是其余符号，则直接输出
                            gendo(lit,0,cur_str[str_index]);    //把当前字符存在栈顶
                            gendo(opr,0,17);     //将栈顶字符串输出
                        }
                    }
//                    getsym();
                }
                else{     //原read语句处理
                    do {
                        if(sym == comma){
                            getsymdo;   //如果当前符号为逗号，则需要继续读取下一个标识符，方式出错
                        }
//                      getsymdo;
                        if (sym == ident)
                        {
                            i = position(id, *ptx); /* 查找要读的变量 */
                        }
                        else
                        {
                            i=0;
                        }

                        if (i == 0)
                        {
                            error(35);  /* read()中应是声明过的变量名 */
                        }
                        else if (table[i].kind != variable)
                        {
                            error(32);	/* read()参数表的标识符不是变量, thanks to amd */
                        }
                        else
                        {
                            gendo(opr, 0, 19);  /* 生成输入指令，读取值到栈顶 */
                            gendo(sto, lev-table[i].level, table[i].adr);   /* 储存到变量 */
                        }
                        getsymdo;
                    } while (sym == comma); /* 一条read语句可读多个变量 */
                }
            }
            if(sym != rparen)
            {
                error(33);  /* 格式错误，应是右括号 */
                while (!inset(sym, fsys))   /* 出错补救，直到收到上层函数的后跟符号 */
                {
                    getsymdo;
                }
            }
            else
            {
                getsymdo;
            }
        }
        else
        {
            if (sym == writesym)    /* 准备按照write语句处理，与read类似 */
            {
                getsymdo;
                if (sym == lparen)
                {
                    getsymdo;
                    if(sym==str){    //如果为字符串，则表明进行格式化输出，需要遍历str的查看是否有%d形式
                        getsymdo;    //读出字符串右边第一个单词，应当为一个逗号或右括号
//                        int k=0;
                        //遍历原str
                        for(int i=0;i<strmaxlen;i++){
                            if(cur_str[i]=='\0'){    //遇到字符串终结符
                                break;
                            }
                            else if(cur_str[i]=='%'){      //遇到了一个格式化输出

                                getsymdo;             //获取该格式化输出对应的标识符
//                                if(sym!=ident){
//                                    error(101);    //格式化输入没有找到匹配的ident,无法输出
//                                }

                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[rparen] = true;
                                nxtlev[comma] = true;       /*这三行代码的作用代码出错补救措施*/

                                expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                                i++;//读取格式输入下一位
                                if(cur_str[i]=='d'){
                                    gendo(opr, 0, 14);
                                }
                                else if(cur_str[i]=='f'){
                                    gendo(opr,0,18);
                                }
                                else{
                                    error(102);
                                }
                            }
                            else{       //若遇到除%以外的其他信息，则直接输出即可
                                gendo(lit,0,cur_str[i]);    //把当前字符存在栈顶
                                gendo(opr,0,17);     //将栈顶字符串输出
                            }
                        }
                    }

                    else{
                        do {
                            if(sym == comma){
                                getsymdo;
                            }
                            //参数nxtlev代表了当前词单元的follow集

                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[rparen] = true;
                            nxtlev[comma] = true;       /* write的后跟符号为) or , */

                            //上述三行代码的作用为更新follow集

                            expressiondo(nxtlev, ptx, lev); /* 调用表达式处理，此处与read不同，read为给变量赋值 */
                            gendo(opr, 0, 18);  /* 生成输出指令，输出栈顶的值 */
                        } while (sym == comma);
                    }

                    if (sym != rparen)
                    {
                        error(33);  /* write()中应为完整表达式 */
                    }
                    else
                    {
                        getsymdo;
                    }
                }
                gendo(opr, 0, 15);  /* 输出换行 */
            }
            else
            {
                if (sym == callsym) /* 准备按照call语句处理 */
                {
                    getsymdo;
                    if (sym != ident)
                    {
                        error(14);  /* call后应为标识符 */
                    }
                    else
                    {
                        i = position(id, *ptx);
                        if (i == 0)
                        {
                            error(11);  /* 过程未找到 */
                        }
                        else
                        {
                            if (table[i].kind == procedur)
                            {
                                gendo(cal, lev-table[i].level, table[i].adr);   /* 生成call指令 */
                            }
                            else
                            {
                                error(15);  /* call后标识符应为过程 */
                            }
                        }
                        getsymdo;
                    }
                }
                else
                {
                    if (sym == ifsym)   /* 准备按照if语句处理 */
                    {
                        getsymdo;
                        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                        nxtlev[thensym] = true;
                        nxtlev[dosym] = true;   /* 后跟符号为then或do */
                        conditiondo(nxtlev, ptx, lev); /* 调用条件处理（逻辑运算）函数 */
                        if (sym == thensym)
                        {
                            getsymdo;
                        }
                        else
                        {
                            error(16);  /* 缺少then */
                        }
                        cx1 = cx;   /* 保存当前指令地址 */
                        gendo(jpc, 0, 0);   /* 生成条件跳转指令，跳转地址未知，暂时写0 */
                        statementdo(fsys, ptx, lev);    /* 处理then后的语句 */
                        code[cx1].a = cx;   /* 经statement处理后，cx为then后语句执行完的位置，它正是前面未定的跳转地址 */
                    }
                    else
                    {
                        if (sym == beginsym)    /* 准备按照复合语句处理 */
                        {
                            getsymdo;
                            memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                            nxtlev[semicolon] = true;
                            nxtlev[endsym] = true;  /* 后跟符号为分号或end */
                            /* 循环调用语句处理函数，直到下一个符号不是语句开始符号或收到end */
                            statementdo(nxtlev, ptx, lev);

                            while (inset(sym, statbegsys) || sym==semicolon)
                            {
                                if (sym == semicolon)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(10);  /* 缺少分号 */
                                }
                                statementdo(nxtlev, ptx, lev);
                            }
                            if(sym == endsym)
                            {
                                getsymdo;
                            }
                            else
                            {
                                error(17);  /* 缺少end或分号 */
                            }
                        }
                        else
                        {
                            if (sym == whilesym)    /* 准备按照while语句处理 */
                            {
                                cur_loop_num++;
                                cx1 = cx;   /* 保存判断条件操作的位置 */
                                getsymdo;
                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[dosym] = true;   /* 后跟符号为do */
                                conditiondo(nxtlev, ptx, lev);  /* 调用条件处理 */
                                cx2 = cx;   /* 保存循环体的结束的下一个位置 */
                                gendo(jpc, 0, 0);   /* 生成条件跳转，但跳出循环的地址未知 */
                                if (sym == dosym)
                                {
                                    getsymdo;
                                }
                                else
                                {
                                    error(18);  /* 缺少do */
                                }
                                statementdo(fsys, ptx, lev);    /* 循环体 */

                                if(contains_continue[cur_loop_num]==true){    //若有continue语句，则需要回填跳转地址
                                    code[continue_cx].a=cx;
                                }

                                gendo(jmp, 0, cx1); /* 回头重新判断条件 */
                                code[cx2].a = cx;   /* 反填跳出循环的地址，与if类似 */

                                if(contains_break[cur_loop_num]==true){    //若有break语句，则需要回填跳转地址
                                    cur_loop_num--;
                                    code[break_cx].a=cx;
                                }
                                cur_loop_num--;

                            }
                            else if(sym == forsym){     /* 准备按照for语句处理 */
                                cur_loop_num++;//更新循环层数
                                if(cur_loop_num>max_loop){
                                    error(109);
                                }
                                getsymdo;
                                if(sym!=ident){
                                    error(106);    //格式化输入没有找到匹配的ident,无法输出
                                }

                                i=position(id,*ptx);  //记录循环控制变量所在位置

                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[tosym] = true;   //后跟符号为tosym或downtosym
                                nxtlev[downtosym] = true;
                                statementdo(nxtlev, ptx, lev);    //处理赋值语句

                                switch(sym){
                                    case tosym:           //步长为向上增加
                                    cx1=cx;       //保存循环开始点,也就是将控制遍历取出到栈顶这条指令的地址

                                    //进入for循环之前，首先需要进行条件的判断
                                    gendo(lod,lev-table[i].level,table[i].adr);   //将控制变量取出到栈顶

                                    memcpy(nxtlev,fsys,sizeof(bool)*symnum);
                                    nxtlev[dosym]=true;                //表达式后跟do

                                    getsymdo;
                                    expressiondo(nxtlev,ptx,lev);       //处理终止条件表达式，此时控制变量的终值被存储在栈顶，而控制变量被存储在次站顶
                                    gendo(opr,0,13);  //判断是否小于等于终止条件，pascal的for循环包括终止值

                                    cx2=cx;                      /* for循环跳转指令的地址位置 */

                                    gendo(jpc,0,0);        /* 生成条件跳转，但跳出循环的地址未知，先用0代替 */


                                    if(sym!=dosym){
                                        error(108);
                                    }
                                    getsymdo;
                                    statement(fsys,ptx,lev);  //循环体内语句处理

                                    if(contains_continue[cur_loop_num]==true){    //若有continue语句，则需要回填跳转地址
                                        code[continue_cx].a=cx;
                                    }

                                    gendo(lod,lev-table[i].level,table[i].adr); //将循环变量的值取到栈顶
                                    gendo(lit,0,1);     //to步长为1
                                    gendo(opr,0,2);     //计算得到循环遍历新值

                                    gendo(sto,lev-table[i].level,table[i].adr);     //更新循环变量的新值
                                    gendo(jmp,0,cx1);    //进行下一次循环

                                    code[cx2].a=cx;         //回填循环跳出位置
                                    if(contains_break[cur_loop_num]==true){    //若有break语句，则需要回填跳转地址
                                        code[break_cx].a=cx;
                                    }
                                    cur_loop_num--;
                                    break;

                                    case downtosym:           //步长为的向上增加
                                        cx1=cx;       //保存循环开始点

                                        //进入for循环之前，首先需要进行条件的判断
                                        gendo(lod,lev-table[i].level,table[i].adr);   //将控制变量取出到栈顶

                                        memcpy(nxtlev,fsys,sizeof(bool)*symnum);
                                        nxtlev[dosym]=true;                //表达式后跟do

                                        getsymdo;
                                        expressiondo(nxtlev,ptx,lev);       //处理终止条件表达式，此时控制变量的终值被存储在栈顶，而控制变量被存储在次站顶
                                        gendo(opr,0,11);  //判断是否等于终止条件

                                        cx2=cx;                      /* 保存循环体的结束的下一个位置 */

                                        gendo(jpc,0,0);        /* 生成条件跳转，但跳出循环的地址未知，先用0代替 */


                                        if(sym!=dosym){
                                            error(108);
                                        }

                                        getsymdo;
                                        statement(fsys,ptx,lev);  //循环体内语句处理

                                        if(contains_continue[cur_loop_num]==true){    //若有continue语句，则需要回填跳转地址
                                            code[continue_cx].a=cx;
                                        }


                                        gendo(lod,lev-table[i].level,table[i].adr); //将循环变量的值取到栈顶
                                        gendo(lit,0,1);     //to步长为1
                                        gendo(opr,0,3);     //计算得到循环遍历新值

                                        gendo(sto,lev-table[i].level,table[i].adr);     //更新循环变量的新值
                                        gendo(jmp,0,cx1);    //进行下一次循环

                                        code[cx2].a=cx;         //回填循环跳出位置

                                        if(contains_break[cur_loop_num]==true){    //若有break语句，则需要回填跳转地址
                                            code[break_cx].a=cx;
                                        }
                                        cur_loop_num--;
                                        break;
                                }


                            }
                            else if(sym==breaksym){
                                if(cur_loop_num==0){
                                    error(110);
                                }
                                contains_break[cur_loop_num]=true;
                                break_cx=cx;
                                gendo(jmp,0,0);     //生成跳转指令，在循环中回填跳转地址
                                getsymdo;
                            }
                            else if(sym==continuesym){
                                if(cur_loop_num==0){
                                    error(111);
                                }
                                contains_continue[cur_loop_num]=true;
                                continue_cx=cx;
                                gendo(jmp,0,0);     //生成跳转指令，在循环中回填跳转地址
                                getsymdo;
                            }
                             else if(sym==ptrsym){     //动态内存分配
                                char ptr_id[100];
                                int table_index;    //记录指针在名字表中的序号
                                getsymdo;
                                if(sym!=ident){
                                    error(112);
                                }
                                enter(ptr, ptx, lev,0 ); // 填写名字表
                                strcpy(ptr_id,id);     //存储当前的指针id
                                table_index = *ptx;    //存储当前的指针名字表索引
                                getsymdo;
                                if(sym!=becomes){
                                    error(113);
                                }
                                getsymdo;
                                if(sym!=mallocsym){
                                    error(114);
                                }
                                getsymdo;
                                if(sym!=lparen){
                                    error(115);
                                }
                                getsymdo;
                                memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                                nxtlev[rparen] = true;   //后跟符号为右括号
                                expressiondo(nxtlev, ptx, lev);    //处理表达式,计算分配空间大小
                                gendo(dya,0,table_index);
                                if (sym != rparen)
                                {
                                    error(115);
                                }
                                getsymdo;
                            }
                            else
                            {
                                memset(nxtlev, 0, sizeof(bool)*symnum); /* 语句结束无补救集合 */
                                testdo(fsys, nxtlev, 19);   /* 检测语句结束的正确性 */
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

/*
* 表达式处理
*/
int expression(bool* fsys, int* ptx, int lev)
{
    enum symbol addop;  /* 用于保存正负号 */
    bool nxtlev[symnum];

    if(sym==plus || sym==minus) /* 开头的正负号，此时当前表达式被看作一个正的或负的项 */
    {
        addop = sym;    /* 保存开头的正负号 */
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == minus)
        {
            gendo(opr,0,1); /* 如果开头为负号生成取负指令 */
        }
    }
    else    /* 此时表达式被看作项的加减 */
    {
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
    }
    while (sym==plus || sym==minus)
    {
        addop = sym;
        getsymdo;
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[plus] = true;
        nxtlev[minus] = true;
        termdo(nxtlev, ptx, lev);   /* 处理项 */
        if (addop == plus)
        {
            gendo(opr, 0, 2);   /* 生成加法指令 */
        }
        else
        {
            gendo(opr, 0, 3);   /* 生成减法指令 */
        }
    }
    return 0;
}

/*
* 项处理
*/
int term(bool* fsys, int* ptx, int lev)
{
    enum symbol mulop;  /* 用于保存乘除法符号 */
    bool nxtlev[symnum];

    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
    nxtlev[times] = true;
    nxtlev[slash] = true;
    factordo(nxtlev, ptx, lev); /* 处理因子 */
    while(sym==times || sym==slash)
    {
        mulop = sym;
        getsymdo;
        factordo(nxtlev, ptx, lev);
        if(mulop == times)
        {
            gendo(opr, 0, 4);   /* 生成乘法指令 */
        }
        else
        {
            gendo(opr, 0, 5);   /* 生成除法指令 */
        }
    }
    return 0;
}

/*
* 因子处理
*/
int factor(bool* fsys, int* ptx, int lev)
{
    int i;
    bool nxtlev[symnum];
    testdo(facbegsys, fsys, 24);    /* 检测因子的开始符号 */
    /* while(inset(sym, facbegsys)) */  /* 循环直到不是因子开始符号 */
    if(inset(sym,facbegsys))    /* BUG: 原来的方法var1(var2+var3)会被错误识别为因子 */
    {
        if(sym == ident)    /* 因子为常量或变量 */
        {
            i = position(id, *ptx); /* 查找名字 */
            if (i == 0)
            {
                error(11);  /* 标识符未声明 */
            }
            else
            {
                switch (table[i].kind)
                {
                    case constant:  /* 名字为常量 */
                        gendo(lit, 0, table[i].val);    /* 直接把常量的值入栈 */
                        break;
                    case variable:  /* 名字为变量 */
                        gendo(lod, lev-table[i].level, table[i].adr);   /* 找到变量地址并将其值入栈 */
                        break;
                    case procedur:  /* 名字为过程 */
                        error(21);  /* 不能为过程 */
                        break;
                    case ptr:     /*名字为数组*/
                        getsymdo;
                    if(sym!=lfang){
                        error(117);
                    }
                    int base_addr = table[i].val;  //基址
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool) * symnum);
                    nxtlev[rparen] = true;
                    expressiondo(nxtlev, ptx, lev);     //此时偏移量被保存在栈顶
                    gendo(ldh,0,i);
                    break;
                }
            }
            getsymdo;
        }
        else
        {
            if(sym == number)   /* 因子为数 */
            {
                if (num > amax)
                {
                    error(31);
                    num = 0;
                }
                gendo(lit, 0, num);
                getsymdo;
            }
            else
            {
                if (sym == lparen)  /* 因子为表达式 */
                {
                    getsymdo;
                    memcpy(nxtlev, fsys, sizeof(bool)*symnum);
                    nxtlev[rparen] = true;
                    expressiondo(nxtlev, ptx, lev);
                    if (sym == rparen)
                    {
                        getsymdo;
                    }
                    else
                    {
                        error(22);  /* 缺少右括号 */
                    }
                }
                testdo(fsys, facbegsys, 23);    /* 因子后有非法符号 */
            }
        }
    }
    return 0;
}

/*
* 条件处理
*/
int condition(bool* fsys, int* ptx, int lev)
{
    enum symbol relop;
    bool nxtlev[symnum];

    if(sym == oddsym)   /* 准备按照odd运算处理 */
    {
        getsymdo;
        expressiondo(fsys, ptx, lev);
        gendo(opr, 0, 6);   /* 生成odd指令 */
    }
    else
    {
        /* 逻辑表达式处理 */
        memcpy(nxtlev, fsys, sizeof(bool)*symnum);
        nxtlev[eql] = true;
        nxtlev[neq] = true;
        nxtlev[lss] = true;
        nxtlev[leq] = true;
        nxtlev[gtr] = true;
        nxtlev[geq] = true;
        expressiondo(nxtlev, ptx, lev);
        if (sym!=eql && sym!=neq && sym!=lss && sym!=leq && sym!=gtr && sym!=geq)
        {
            error(20);
        }
        else
        {
            relop = sym;
            getsymdo;
            expressiondo(fsys, ptx, lev);
            switch (relop)
            {
                case eql:
                    gendo(opr, 0, 8);
                    break;
                case neq:
                    gendo(opr, 0, 9);
                    break;
                case lss:
                    gendo(opr, 0, 10);
                    break;
                case geq:
                    gendo(opr, 0, 11);
                    break;
                case gtr:
                    gendo(opr, 0, 12);
                    break;
                case leq:
                    gendo(opr, 0, 13);
                    break;
            }
        }
    }
    return 0;
}

/*
* 解释程序
*/
void interpret()
{
    int p, b, t;    /* 指令指针，指令基址，栈顶指针 */
    struct instruction i;   /* 存放当前指令 */
    float  s[stacksize];   /* 栈 */

    printf("start pl0\n");
    t = 0;
    b = 0;
    p = 0;
    s[0] = s[1] = s[2] = 0;
    do {
        i = code[p];    /* 读当前指令 */
        p++;
        switch (i.f)
        {
            case lit:   /* 将a的值取到栈顶 */
                s[t] = i.a;
                t++;
                break;
            case opr:   /* 数学、逻辑运算 */
                switch ((int)i.a)
                {
                    case 0:
                        t = b;
                        p = s[t+2];
                        b = s[t+1];
                        break;
                    case 1:
                        s[t-1] = -s[t-1];
                        break;
                    case 2:
                        t--;
                        s[t-1] = s[t-1]+s[t];
                        break;
                    case 3:
                        t--;
                        s[t-1] = s[t-1]-s[t];
                        break;
                    case 4:
                        t--;
                        s[t-1] = s[t-1]*s[t];
                        break;
                    case 5:
                        t--;
                        s[t-1] = s[t-1]/s[t];
                        break;
                    case 6:
                        s[t-1] = (int)s[t-1]%2;
                        break;
                    case 8:
                        t--;
                        s[t-1] = (s[t-1] == s[t]);
                        break;
                    case 9:
                        t--;
                        s[t-1] = (s[t-1] != s[t]);
                        break;
                    case 10:
                        t--;
                        s[t-1] = (s[t-1] < s[t]);
                        break;
                    case 11:
                        t--;
                        s[t-1] = (s[t-1] >= s[t]);
                        break;
                    case 12:
                        t--;
                        s[t-1] = (s[t-1] > s[t]);
                        break;
                    case 13:
                        t--;
                        s[t-1] = (s[t-1] <= s[t]);
                        break;
                    case 14:            //整数输出
                        std::cout<<s[t-1];
                        fprintf(fa2, "%d",(int) s[t-1]);
                        t--;
                        break;
                    case 15:
                        printf("\n");
                        fprintf(fa2,"\n");
                        break;
                    case 16:                //整数读入
//                        std::cout<<"Integer In"<<std::endl;
                        printf("?");
                        fprintf(fa2, "?");
                        std::cin>>s[t];
                        s[t] = (int) s[t];
//                        scanf("%d", &(s[t]));
                        fprintf(fa2, "%d\n", s[t]);
//                        std::cout<<s[t];
                        t++;
                        break;
                    case 17:                       //char类型输出
                        printf("%c", (int)s[t - 1]);
                        fprintf(fa2, "%c",(int) s[t - 1]);
                        t--;
                        break;
                    case 18:                        //浮点数输出
                        std::cout<<s[t-1];
                        fprintf(fa2, "%f", s[t-1]);
                        t--;
                        break;
                    case 19:                        //浮点数读入
//                    std::cout<<"Float In"<<std::endl;
                        printf("?");
                        fprintf(fa2, "?");
                        scanf("%f", &(s[t]));
                        fprintf(fa2, "%f\n", s[t]);
//                        std::cout<<s[t];
                        t++;
                        break;
                }
                break;
            case lod:   /* 取相对当前过程的数据基地址为a的内存的值到栈顶 */
                s[t] = s[base(i.l,s,b)+(int)i.a];
                t++;
                break;
            case sto:   /* 栈顶的值存到相对当前过程的数据基地址为a的内存 */
                t--;
                s[base(i.l, s, b) + (int)i.a] = s[t];
                break;
            case cal:   /* 调用子过程 */
                s[t] = base(i.l, s, b); /* 将父过程基地址入栈 */
                s[t+1] = b; /* 将本过程基地址入栈，此两项用于base函数 */
                s[t+2] = p; /* 将当前指令指针入栈 */
                b = t;  /* 改变基地址指针值为新过程的基地址 */
                p = i.a;    /* 跳转 */
                break;
            case inte:  /* 分配内存 */
                t += i.a;
                break;
            case jmp:   /* 直接跳转 */
                p = i.a;
                break;
            case jpc:   /* 条件跳转 */
                t--;
                if (s[t] == 0)
                {
                    p = i.a;
                }
                break;
            case dya:   //为指针分配堆空间和地址，此时s[t]存储了应分配大小
                t--;
                table[(int)i.a].size = s[t];
                table[(int)i.a].val = heap_ptr;
                heap_ptr+=s[t];
//                std::cout<<"现在堆顶在："<<heap_ptr<<std::endl;
//                std::cout<<"已经分配了:"<<s[t]<<std::endl;
                if(heap_ptr>heapsize){
                    error(118);    //栈空间不足
                }
                break;
            case sth:
                t--;
                heap[table[(int)i.a].val+(int)s[t-1]] = s[t];
//                std::cout<<"堆中元素的值为："<<heap[9];
                break;
            case ldh:       //当前栈顶存储值为偏移量
                t--;
                s[t] = heap[table[(int)i.a].val+(int)s[t]];   //将堆中相应位置的数据存储到栈顶
                t++;
                break;

        }
    } while (p != 0);
}

/* 通过过程基址求上l层过程的基址 */
int base(int l, float* s, int b)
{
    int b1;
    b1 = b;
    while (l > 0)
    {
        b1 = s[b1];
        l--;
    }
    return b1;
}

