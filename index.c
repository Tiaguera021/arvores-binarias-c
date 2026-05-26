#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

typedef enum { TIPO_VALOR, TIPO_BINARIO, TIPO_UNARIO } TipoNodo;

typedef struct Nodo {
    TipoNodo tipo;
    char operador;
    int valor_numerico;
    struct Nodo* esquerda;
    struct Nodo* direita;
} Nodo;

Nodo* criar_binario(char op, Nodo* esq, Nodo* dir) {
    Nodo* novo = (Nodo*)malloc(sizeof(Nodo));
    novo->tipo = TIPO_BINARIO;
    novo->operador = op;
    novo->esquerda = esq;
    novo->direita = dir;
    return novo;
}

Nodo* criar_unario(char op, Nodo* filho) {
    Nodo* novo = (Nodo*)malloc(sizeof(Nodo));
    novo->tipo = TIPO_UNARIO;
    novo->operador = op;
    novo->esquerda = filho;
    novo->direita = NULL;
    return novo;
}

Nodo* criar_valor(int val) {
    Nodo* novo = (Nodo*)malloc(sizeof(Nodo));
    novo->tipo = TIPO_VALOR;
    novo->valor_numerico = val;
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

void pular_espacos(const char** str) {
    while (isspace(**str)) (*str)++;
}

Nodo* analisar_expressao(const char** str);

Nodo* analisar_fator(const char** str) {
    pular_espacos(str);
    
    if (**str == '(') {
        (*str)++; 
        Nodo* nodo = analisar_expressao(str);
        pular_espacos(str);
        if (**str == ')') (*str)++;
        return nodo;
    }
    
    if (strncmp(*str, "sqrt", 4) == 0) {
        *str += 4;
        pular_espacos(str);
        if (**str == '(') {
            (*str)++;
            Nodo* interno = analisar_expressao(str);
            pular_espacos(str);
            if (**str == ')') (*str)++;
            return criar_unario('s', interno);
        }
    }

    if (**str == '-') {
        (*str)++;
        return criar_unario('n', analisar_fator(str));
    }
    
    int valor = 0;
    int lidos = 0;
    sscanf(*str, "%d%n", &valor, &lidos);
    *str += lidos;
    return criar_valor(valor);
}

Nodo* analisar_expoente(const char** str) {
    Nodo* esquerdo = analisar_fator(str);
    pular_espacos(str);
    
    while (**str == '$' || **str == '^') {
        char op = **str;
        (*str)++;
        Nodo* direito = analisar_fator(str);
        esquerdo = criar_binario(op, esquerdo, direito);
        pular_espacos(str);
    }
    return esquerdo;
}

Nodo* analisar_termo(const char** str) {
    Nodo* esquerdo = analisar_expoente(str);
    pular_espacos(str);
    
    while (**str == '*' || **str == '/') {
        char op = **str;
        (*str)++;
        esquerdo = criar_binario(op, esquerdo, analisar_expoente(str));
        pular_espacos(str);
    }
    return esquerdo;
}

Nodo* analisar_expressao(const char** str) {
    Nodo* esquerdo = analisar_termo(str);
    pular_espacos(str);
    
    while (**str == '+' || **str == '-') {
        char op = **str;
        (*str)++;
        esquerdo = criar_binario(op, esquerdo, analisar_termo(str));
        pular_espacos(str);
    }
    return esquerdo;
}

float avaliar_arvore(Nodo* raiz) {
    if (raiz == NULL) return 0;
    
    if (raiz->tipo == TIPO_VALOR) {
        return (float)raiz->valor_numerico;
    }
    
    if (raiz->tipo == TIPO_UNARIO) {
        float val = avaliar_arvore(raiz->esquerda);
        if (raiz->operador == 'n') return -val;
        if (raiz->operador == 's') return sqrt(val);
    }
    
    float esq = avaliar_arvore(raiz->esquerda);
    float dir = avaliar_arvore(raiz->direita);
    
    switch (raiz->operador) {
        case '+': return esq + dir;
        case '-': return esq - dir;
        case '*': return esq * dir;
        case '/': 
            if (dir == 0) return 0;
            return esq / dir;
        case '$':
        case '^': return pow(esq, dir);
        default: return 0;
    }
}

// Escreve os nós e suas conexões no formato Graphviz
void exportar_dot_recursivo(Nodo* raiz, FILE* arquivo) {
    if (raiz == NULL) return;

    // Define o rótulo do nó atual usando o endereço de memória como ID único
    if (raiz->tipo == TIPO_VALOR) {
        fprintf(arquivo, "  \"%p\" [label=\"%d\"];\n", (void*)raiz, raiz->valor_numerico);
    } else if (raiz->tipo == TIPO_UNARIO) {
        fprintf(arquivo, "  \"%p\" [label=\"%s\"];\n", (void*)raiz, raiz->operador == 's' ? "sqrt" : "-");
    } else {
        fprintf(arquivo, "  \"%p\" [label=\"%c\"];\n", (void*)raiz, raiz->operador);
    }

    // Desenha as setas para os filhos esquerdo e direito
    if (raiz->esquerda != NULL) {
        fprintf(arquivo, "  \"%p\" -> \"%p\";\n", (void*)raiz, (void*)raiz->esquerda);
        exportar_dot_recursivo(raiz->esquerda, arquivo);
    }
    if (raiz->direita != NULL) {
        fprintf(arquivo, "  \"%p\" -> \"%p\";\n", (void*)raiz, (void*)raiz->direita);
        exportar_dot_recursivo(raiz->direita, arquivo);
    }
}


void gerar_arquivo_graphviz(Nodo* raiz, int indice) {
    char nome_arquivo[64];
    sprintf(nome_arquivo, "arvore_%d.dot", indice);
    
    FILE* arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        printf("Falha critica: Não foi possivel criar o arquivo %s\n", nome_arquivo);
        return;
    }
    
    fprintf(arquivo, "digraph ArvoreExpressao {\n");
    fprintf(arquivo, "  node [shape=circle, fontname=\"Helvetica\"];\n");
    exportar_dot_recursivo(raiz, arquivo);
    fprintf(arquivo, "}\n");
    
    fclose(arquivo);
    printf(">> Arquivo grafico '%s' gerado com sucesso.\n", nome_arquivo);
}

void liberar_arvore(Nodo* raiz) {
    if (raiz != NULL) {
        liberar_arvore(raiz->esquerda);
        liberar_arvore(raiz->direita);
        free(raiz);
    }
}

int main() {
    char lote_expressoes[100][256];
    int quantidade = 0;
    
    printf("Digite as expressoes matematicas (pressione ENTER em uma linha vazia para processar):\n");
    
    while (quantidade < 100) {
        printf("> ");
        if (fgets(lote_expressoes[quantidade], 256, stdin) == NULL) break;
        
        lote_expressoes[quantidade][strcspn(lote_expressoes[quantidade], "\n")] = 0;
        if (strlen(lote_expressoes[quantidade]) == 0) break;
        
        quantidade++;
    }
    
    printf("\nProcessando o lote de %d expressoes...\n\n", quantidade);
    
    for (int i = 0; i < quantidade; i++) {
        const char* ponteiro = lote_expressoes[i];
        Nodo* raiz = analisar_expressao(&ponteiro);
        
        printf("Expressao %d: %s\n", i + 1, lote_expressoes[i]);
        
        float resultado = avaliar_arvore(raiz);
        printf("Resultado Final: %.2f\n", resultado);
        
        // Substitui a impressão por conectores de texto pela exportação estruturada
        gerar_arquivo_graphviz(raiz, i + 1);
        printf("\n");
        
        liberar_arvore(raiz);
    }
    
    return 0;
}