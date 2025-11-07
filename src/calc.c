#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

/*
Name: Mehmet Kaan ULU
Student ID: 231ADB102

*/

typedef struct { const char *s; size_t i; } Lexer;

static void skip_ws(Lexer *L){ while(L->s[L->i] && isspace((unsigned char)L->s[L->i])) L->i++; }
static int peek(Lexer *L){ skip_ws(L); return (unsigned char)L->s[L->i]; }
static int eat(Lexer *L, int c){ skip_ws(L); if(L->s[L->i]==c){ L->i++; return 1; } return 0; }

static int number(Lexer *L, double *out){
    skip_ws(L);
    size_t start = L->i;
    int seen = 0, dot = 0;
    if(L->s[L->i]=='+' || L->s[L->i]=='-'){ L->i++; }
    while(isdigit((unsigned char)L->s[L->i]) || L->s[L->i]=='.'){
        if(L->s[L->i]=='.'){ if(dot) break; dot=1; }
        seen=1; L->i++;
    }
    if(!seen){ L->i=start; return 0; }
    char buf[128];
    size_t n = L->i - start; if(n>=sizeof(buf)) n = sizeof(buf)-1;
    memcpy(buf, L->s + start, n); buf[n]='\0';
    char *endp=NULL;
    *out = strtod(buf,&endp);
    if(endp==buf){ L->i=start; return 0; }
    return 1;
}

static int errflag=0;

static double parse_expr(Lexer *L);

static double parse_factor(Lexer *L){
    if(eat(L,'+')) return parse_factor(L);
    if(eat(L,'-')) return -parse_factor(L);
    if(eat(L,'(')){
        double v = parse_expr(L);
        if(!eat(L,')')){ errflag=1; return 0.0; }
        return v;
    }
    double v;
    if(number(L,&v)) return v;
    errflag=1; return 0.0;
}

static double parse_term(Lexer *L){
    double v = parse_factor(L);
    for(;;){
        if(eat(L,'*')) v *= parse_factor(L);
        else if(eat(L,'/')){
            double r = parse_factor(L);
            if(r==0.0){ errflag=2; return 0.0; }
            v /= r;
        } else break;
    }
    return v;
}

static double parse_expr(Lexer *L){
    double v = parse_term(L);
    for(;;){
        if(eat(L,'+')) v += parse_term(L);
        else if(eat(L,'-')) v -= parse_term(L);
        else break;
    }
    return v;
}

static int is_int_like(double x){
    double y = round(x);
    return fabs(x - y) < 1e-9 && fabs(y) < 9e15;
}

static void print_number(double x){
    if(is_int_like(x)){
        long long k = (long long)llround(x);
        printf("%lld\n", k);
    }else{
        char buf[64];
        snprintf(buf,sizeof(buf),"%.12g", x);
        printf("%s\n", buf);
    }
}

int main(int argc, char **argv){
    if(argc<2){
        fprintf(stderr,"usage: %s \"expr\"\n", argv[0]);
        return 1;
    }
    Lexer L = (Lexer){ .s = argv[1], .i = 0 };
    errflag = 0;
    double val = parse_expr(&L);
    if(errflag==2){ fprintf(stderr,"division by zero\n"); return 2; }
    if(errflag || peek(&L)!=0){ fprintf(stderr,"syntax error near pos %zu\n", L.i+1); return 3; }
    print_number(val);
    return 0;
}
