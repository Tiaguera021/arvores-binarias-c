#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_EXPR   1024
#define MAX_TOKENS 512
#define MAX_EXPRS  64

typedef enum { OPERANDO, OPERADOR, FUNC_SQRT } TipoNo;

typedef struct No {
    TipoNo tipo;
    char op;
    double valor;
    char rotulo[32];
    struct No *esq;
    struct No *dir;
} No;

No *novoOperando(double val) {
    No *n = (No *)malloc(sizeof(No));
    n->tipo = OPERANDO;
    n->valor = val;
    n->esq = n->dir = NULL;
    snprintf(n->rotulo, sizeof(n->rotulo), "%.6g", val);
    return n;
}

No *novoOperador(char op) {
    No *n = (No *)malloc(sizeof(No));
    n->tipo = OPERADOR;
    n->op = op;
    n->esq = n->dir = NULL;
    if (op == 'u')
        snprintf(n->rotulo, sizeof(n->rotulo), "-(unario)");
    else
        snprintf(n->rotulo, sizeof(n->rotulo), "%c", op);
    return n;
}

No *novoSqrt() {
    No *n = (No *)malloc(sizeof(No));
    n->tipo = FUNC_SQRT;
    n->op = 's';
    n->esq = n->dir = NULL;
    snprintf(n->rotulo, sizeof(n->rotulo), "sqrt");
    return n;
}

typedef enum {
    TOK_NUM, TOK_MAIS, TOK_MENOS, TOK_MUL, TOK_DIV,
    TOK_POT, TOK_SQRT, TOK_ABRE, TOK_FECHA, TOK_FIM
} TipoToken;

typedef struct {
    TipoToken tipo;
    double numval;
} Token;

Token tokens[MAX_TOKENS];
int totalTokens = 0;
int pos = 0;

int tokenizar(const char *expr) {
    int i = 0;
    totalTokens = 0;
    int n = strlen(expr);

    while (i < n) {
        if (isspace(expr[i])) { i++; continue; }

        if (strncmp(&expr[i], "sqrt", 4) == 0 && (i + 4 >= n || !isalnum(expr[i + 4]))) {
            tokens[totalTokens++].tipo = TOK_SQRT;
            i += 4;
            continue;
        }

        if (isdigit(expr[i])) {
            char buf[64]; int k = 0;
            while (i < n && isdigit(expr[i]))
                buf[k++] = expr[i++];
            buf[k] = '\0';
            tokens[totalTokens].tipo = TOK_NUM;
            tokens[totalTokens].numval = atof(buf);
            totalTokens++;
            continue;
        }

        switch (expr[i]) {
            case '+': tokens[totalTokens++].tipo = TOK_MAIS;  break;
            case '-': tokens[totalTokens++].tipo = TOK_MENOS; break;
            case '*': tokens[totalTokens++].tipo = TOK_MUL;   break;
            case '/': tokens[totalTokens++].tipo = TOK_DIV;   break;
            case '$': tokens[totalTokens++].tipo = TOK_POT;   break;
            case '^': tokens[totalTokens++].tipo = TOK_POT;   break;
            case '(': tokens[totalTokens++].tipo = TOK_ABRE;  break;
            case ')': tokens[totalTokens++].tipo = TOK_FECHA; break;
            default:
                fprintf(stderr, "Caractere desconhecido: '%c'\n", expr[i]);
                return 0;
        }
        i++;
    }
    tokens[totalTokens++].tipo = TOK_FIM;
    return 1;
}

No *parseExpressao();
No *parseTermo();
No *parsePotencia();
No *parseUnario();
No *parsePrimario();

No *parseExpressao() {
    No *esq = parseTermo();
    while (tokens[pos].tipo == TOK_MAIS || tokens[pos].tipo == TOK_MENOS) {
        char op = (tokens[pos].tipo == TOK_MAIS) ? '+' : '-';
        pos++;
        No *dir = parseTermo();
        No *raiz = novoOperador(op);
        raiz->esq = esq;
        raiz->dir = dir;
        esq = raiz;
    }
    return esq;
}

No *parseTermo() {
    No *esq = parsePotencia();
    while (tokens[pos].tipo == TOK_MUL || tokens[pos].tipo == TOK_DIV) {
        char op = (tokens[pos].tipo == TOK_MUL) ? '*' : '/';
        pos++;
        No *dir = parsePotencia();
        No *raiz = novoOperador(op);
        raiz->esq = esq;
        raiz->dir = dir;
        esq = raiz;
    }
    return esq;
}

No *parsePotencia() {
    No *base = parseUnario();
    if (tokens[pos].tipo == TOK_POT) {
        pos++;
        No *exp = parsePotencia();
        No *raiz = novoOperador('^');
        raiz->esq = base;
        raiz->dir = exp;
        return raiz;
    }
    return base;
}

No *parseUnario() {
    if (tokens[pos].tipo == TOK_MENOS) {
        pos++;
        No *operando = parseUnario();
        No *raiz = novoOperador('u');
        raiz->dir = operando;
        return raiz;
    }
    return parsePrimario();
}

No *parsePrimario() {
    if (tokens[pos].tipo == TOK_NUM) {
        No *n = novoOperando(tokens[pos].numval);
        pos++;
        return n;
    }
    if (tokens[pos].tipo == TOK_SQRT) {
        pos++;
        if (tokens[pos].tipo != TOK_ABRE) {
            fprintf(stderr, "Esperado '(' apos sqrt\n");
            return NULL;
        }
        pos++;
        No *arg = parseExpressao();
        if (tokens[pos].tipo != TOK_FECHA) {
            fprintf(stderr, "Esperado ')' apos argumento de sqrt\n");
            return NULL;
        }
        pos++;
        No *raiz = novoSqrt();
        raiz->dir = arg;
        return raiz;
    }
    if (tokens[pos].tipo == TOK_ABRE) {
        pos++;
        No *n = parseExpressao();
        if (tokens[pos].tipo != TOK_FECHA) {
            fprintf(stderr, "Esperado ')'\n");
            return NULL;
        }
        pos++;
        return n;
    }
    fprintf(stderr, "Token inesperado\n");
    return NULL;
}

No *construirArvore(const char *expr) {
    if (!tokenizar(expr)) return NULL;
    pos = 0;
    No *raiz = parseExpressao();
    if (tokens[pos].tipo != TOK_FIM) {
        fprintf(stderr, "Expressao mal formada\n");
        return NULL;
    }
    return raiz;
}

double avaliar(No *raiz) {
    if (!raiz) return 0.0;
    if (raiz->tipo == OPERANDO) return raiz->valor;

    if (raiz->tipo == FUNC_SQRT) {
        double v = avaliar(raiz->dir);
        if (v < 0) { fprintf(stderr, "Aviso: raiz de numero negativo\n"); return NAN; }
        return sqrt(v);
    }

    if (raiz->op == 'u') return -avaliar(raiz->dir);

    double l = avaliar(raiz->esq);
    double r = avaliar(raiz->dir);
    switch (raiz->op) {
        case '+': return l + r;
        case '-': return l - r;
        case '*': return l * r;
        case '/':
            if (r == 0) { fprintf(stderr, "Erro: divisao por zero\n"); return NAN; }
            return l / r;
        case '^': return pow(l, r);
    }
    return 0.0;
}


void imprimirArvoreBonita(No *raiz, const char *prefixo, int isEsq, int isRoot) {
    if (!raiz) return;

    char novoPrefixo[512];

    if (raiz->dir) {
        snprintf(novoPrefixo, sizeof(novoPrefixo), "%s%s", prefixo, isRoot ? "" : (isEsq ? "|   " : "    "));
        imprimirArvoreBonita(raiz->dir, novoPrefixo, 0, 0);
    }

    printf("%s", prefixo);
    if (isRoot) {
        printf("[%s]\n", raiz->rotulo);
    } else {
        printf("%s[%s]\n", isEsq ? "\\-- " : "/-- ", raiz->rotulo);
    }

    if (raiz->esq) {
        snprintf(novoPrefixo, sizeof(novoPrefixo), "%s%s", prefixo, isRoot ? "" : (isEsq ? "    " : "|   "));
        imprimirArvoreBonita(raiz->esq, novoPrefixo, 1, 0);
    }
}

void infixa(No *raiz) {
    if (!raiz) return;
    int addParen = (raiz->tipo == OPERADOR && raiz->op != 'u' && raiz->esq);
    if (addParen) printf("(");
    infixa(raiz->esq);
    if (raiz->tipo == OPERANDO) printf("%s", raiz->rotulo);
    else if (raiz->op == 'u') { printf("-("); infixa(raiz->dir); printf(")"); return; }
    else if (raiz->tipo == FUNC_SQRT) { printf("sqrt("); infixa(raiz->dir); printf(")"); return; }
    else printf(" %c ", raiz->op == '^' ? '$' : raiz->op);
    infixa(raiz->dir);
    if (addParen) printf(")");
}

void prefixa(No *raiz) {
    if (!raiz) return;
    printf("%s ", raiz->rotulo);
    prefixa(raiz->esq);
    prefixa(raiz->dir);
}

void posfixa(No *raiz) {
    if (!raiz) return;
    posfixa(raiz->esq);
    posfixa(raiz->dir);
    printf("%s ", raiz->rotulo);
}

void liberarArvore(No *raiz) {
    if (!raiz) return;
    liberarArvore(raiz->esq);
    liberarArvore(raiz->dir);
    free(raiz);
}

int main(void) {
    char exprs[MAX_EXPRS][MAX_EXPR];
    No *arvores[MAX_EXPRS];
    int total = 0;

    printf("==================================\n");
    printf("  Arvore de Expressoes Aritmeticas\n");
    printf("==================================\n");
    printf("  Operadores: + - * / $ (potencia) sqrt() e - unario\n");
    printf("  Digite as expressoes uma por linha.\n");
    printf("  Linha em branco para finalizar.\n");
    printf("==================================\n\n");

    while (total < MAX_EXPRS) {
        printf("Expressao %d: ", total + 1);
        if (!fgets(exprs[total], MAX_EXPR, stdin)) break;
        exprs[total][strcspn(exprs[total], "\n")] = '\0';
        if (strlen(exprs[total]) == 0) break;

        arvores[total] = construirArvore(exprs[total]);
        if (!arvores[total]) {
            printf("  [!] Expressao invalida, tente novamente.\n");
            continue;
        }
        total++;
    }

    if (total == 0) {
        printf("Nenhuma expressao informada.\n");
        return 0;
    }

    printf("  RESULTADOS\n");
   

    for (int i = 0; i < total; i++) {
        printf("\n-------------------------------------------------------\n");
        printf("Expressao %d: %s\n", i + 1, exprs[i]);
        printf("-------------------------------------------------------\n");

        printf("\n>> Arvore no Terminal:\n\n");
        imprimirArvoreBonita(arvores[i], "", 0, 1);
        printf("\n");

        printf(">> Notacao prefixa  : ");
        prefixa(arvores[i]);
        printf("\n");

        printf(">> Notacao posfixa  : ");
        posfixa(arvores[i]);
        printf("\n");

        printf(">> Notacao infixa   : ");
        infixa(arvores[i]);
        printf("\n");

        double resultado = avaliar(arvores[i]);
        if (isnan(resultado))
            printf(">> Resultado        : INDETERMINADO (erro)\n");
        else
            printf(">> Resultado        : %.6g\n", resultado);
    }

    printf("\n=======================================================\n");

    for (int i = 0; i < total; i++) liberarArvore(arvores[i]);

    return 0;
}