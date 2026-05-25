#include <stdio.h>
#include <stdlib.h>
// #include "ArvoreBinaria.h" // Comentado para rodar tudo em um único arquivo

typedef struct NO {
    int info;
    struct NO *esq;
    struct NO *dir;
} NO;

typedef struct ARV {
    NO *raiz;
} ARV;

// Declaração prévia da função para evitar avisos do compilador
void inserirDireita(NO *no, int valor);

void inserirEsquerda(NO *no, int valor){
    if(no->esq == NULL){
        NO *novo = malloc(sizeof(NO)); 
        novo->info = valor;
        novo->esq = NULL;
        novo->dir = NULL;
        no->esq = novo;
    }
    else {
        if(valor < no->esq->info)
            inserirEsquerda(no->esq, valor);
        else
            inserirDireita(no->esq, valor);
    }
}

void inserirDireita(NO *no, int valor){
    if(no->dir == NULL){
        NO *novo = malloc(sizeof(NO)); 
        novo->info = valor;
        novo->esq = NULL;
        novo->dir = NULL;
        no->dir = novo;
    }
    else {
        if(valor > no->dir->info)
            inserirDireita(no->dir, valor);
        else
            inserirEsquerda(no->dir, valor);
    }
}

void inserir(ARV *arv, int valor){
    if(arv->raiz == NULL){
        NO *novo = malloc(sizeof(NO)); 
        novo->info = valor;
        novo->esq = NULL;
        novo->dir = NULL;
        arv->raiz = novo;
    }
    else {
        if(valor < arv->raiz->info)
            inserirEsquerda(arv->raiz, valor);
        else
            inserirDireita(arv->raiz, valor); // CORREÇÃO: Estava chamando inserirEsquerda aqui
    }
}

void imprimir(NO *raiz){
    if(raiz != NULL){
        imprimir(raiz->esq);
        printf("%d ", raiz->info); // CORREÇÃO: print corrigido para printf e adicionado espaço
        imprimir(raiz->dir);
    }
}


int main() {
    int op, valor;
    ARV arv;
    arv.raiz = NULL; // Inicializa a árvore vazia

    do {
        printf("\n--- MENU DA ARVORE BINARIA ---\n");
        printf("1. Inserir valor\n");
        printf("2. Imprimir arvore (Pre-ordem)\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &op);

        switch(op) {
            case 1:
                printf("Digite o valor para inserir: ");
                scanf("%d", &valor);
                inserir(&arv, valor);
                printf("Valor %d inserido com sucesso!\n", valor);
                break;
            case 2:
                printf("\nElementos da Arvore: ");
                imprimir(arv.raiz);
                printf("\n");
                break;
            case 0:
                printf("Saindo do programa...\n");
                break;
            default:
                printf("Opcao invalida! Tente novamente.\n");
        }
    } while(op != 0);

    return 0;
}