#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

// ESTRUTURAS

typedef struct treeNode{
	char valor[15];
	struct treeNode *left;
	struct treeNode *right;
}treeNode;

// Lista usada para guardar as árvores das expressões
typedef struct listNode{
	treeNode *root;
	struct listNode *next;
} listNode;

// PROTÓTIPOS
// Funções de manipulação dos dados
void infixToPosfix(char *infix, char *posfix);
treeNode* buildPosfixTree(char *posfix);
int getPrecedence(char *op);
treeNode* createNode(char *value);

// Funções de avaliação
double avaliateTree(treeNode *root);
int isOperator(char *token);

// Funções para exibição
void printInOrder(treeNode *root);
void printTreeVisual(treeNode *root, int space);
void freeTree(treeNode *root);
void freeList(listNode *top);

int main(void){
	int choice = 0;
	char bufferInfix[500];
	listNode *topList = NULL;
	listNode *endList = NULL;
	
	while (1){
		printf("\n MENU \n");
		printf("1 - Adicionar uma nova expressao\n");
		printf("2 - Mostrar estruturas das arvores\n");
		printf("3 - Encerrar o programa\n");
		printf("Escolha uma das opcoes: ");
		scanf("%d", &choice);	
		while(getchar() != '\n'); 

		switch(choice){
			case 1:
				printf("Digite a expressao: ");
				fgets(bufferInfix, sizeof(bufferInfix), stdin);
				bufferInfix[strcspn(bufferInfix, "\n")] = '\0';
				
				char posfix[500] = "";
				infixToPosfix(bufferInfix, posfix);
				treeNode *root = buildPosfixTree(posfix);
				listNode *newNode = (listNode*)malloc(sizeof(listNode));
				newNode->root = root;
				newNode->next = NULL;
				
				if (topList == NULL) {
					topList = newNode;
					endList = newNode;
				} else {
					endList->next = newNode;
					endList = newNode;
				}
				
				printf("[OK] Expressao adicionada com sucesso\n");
				break;
			
			case 2:
				if (topList == NULL) {
					printf("Nenhuma expressao foi adicionada ainda.\n");
				} else {
					printf("\nEXPRESSOES\n");
					listNode *actual = topList;
					int counter = 1;
					while (actual != NULL){
						printf("--------------------------------------------------\n");
						printf("Expressao %d: \n", counter++);
						
						printf("\nEstrutura da Arvore:\n");
						printTreeVisual(actual->root, 0);
						
						printf("\n\nNotacao Infixa: ");
						printInOrder(actual->root);
						
						printf("\nResultado: %.2f\n", avaliateTree(actual->root));
						printf("--------------------------------------------------\n");
						
						actual = actual->next;
					}
				}
				break;
				
			case 3:
				freeList(topList);
				return 0;
				
			default:
				printf("Escolha uma opcao valida!\n");	
		}
	}
	return 0;
}

treeNode* createNode(char *nodeValue){
	treeNode* new = (treeNode*)malloc(sizeof(treeNode));
	strcpy(new->valor, nodeValue);
	new->left = NULL;
	new->right = NULL;
	return new;
}

int getPrecedence(char *op) {
    if (strcmp(op, "sqrt") == 0 || strcmp(op, "~") == 0) return 4;
    if (strcmp(op, "^") == 0 || strcmp(op, "$") == 0) return 3;    
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 2;    
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 1;   
    return 0;
}

void infixToPosfix(char *infix, char *posfix){
	char pileOp[100][15];
	int topOp = -1;
	int i = 0;
	int isUnary = 1;
	char number[15];
	
	posfix[0] = '\0';
	
	while(infix[i] != '\0'){
		if(isspace(infix[i])){
			i++;
			continue;
		}
		
		if (isdigit(infix[i])){
			int k = 0;
			while (isdigit(infix[i]) || infix[i] == '.'){
				number[k++] = infix[i++];
			}
			number[k] = '\0';
			strcat(posfix, number);
			strcat(posfix, " ");
			isUnary = 0;
			continue;
		}
		
		if (strncmp(&infix[i], "sqrt", 4) == 0){
			strcpy(pileOp[++topOp], "sqrt");
			i = i + 4;
			isUnary = 0;
			continue;
		}
		
		if (infix[i] == '('){
			strcpy(pileOp[++topOp], "(");
			isUnary = 1;
		} else if (infix[i] == ')'){
			while(topOp >= 0 && strcmp(pileOp[topOp], "(") != 0){
				strcat(posfix, pileOp[topOp--]);
				strcat(posfix, " ");
			}
			if (topOp >= 0){
				topOp--;
				isUnary = 0;
			}
		} else {
			char opStr[2] = {infix[i], '\0'};
			
			if (infix[i] == '-' && isUnary){
				strcpy(opStr, "~");
			}
			
			while (topOp >= 0 && strcmp(pileOp[topOp], "(") != 0 && 
			getPrecedence(pileOp[topOp]) >= getPrecedence(opStr)){
				strcat(posfix, pileOp[topOp--]);
				strcat(posfix, " ");
			} 
			strcpy(pileOp[++topOp], opStr);
			isUnary = 1;
		}
		i++;
	}
	
	while(topOp >= 0){
		strcat(posfix, pileOp[topOp--]);
		strcat(posfix, " ");
	}
}

treeNode* buildPosfixTree(char *posfix){
	treeNode* nodePile[100];
	int nodeTop = -1;
	
	char copy[500];
	strcpy(copy, posfix);
	
	char *token = strtok(copy, " ");
	
	while (token != NULL){
		if(strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
		strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
		strcmp(token,"^") == 0 || strcmp(token,"$") == 0){
			treeNode* new = createNode(token);
			new->right = nodePile[nodeTop--];
			new->left = nodePile[nodeTop--];
			nodePile[++nodeTop] = new;
		} else if (strcmp(token, "sqrt") == 0 || strcmp(token, "~") == 0){
			treeNode* new = createNode(token);
			new->left = nodePile[nodeTop--];
			nodePile[++nodeTop] = new;
		} else {
			treeNode* new = createNode(token);
			nodePile[++nodeTop] = new;
		}
		
		token = strtok(NULL, " ");
	}
	
	return nodeTop >= 0 ? nodePile[nodeTop] : NULL;
}

int isOperator(char *token) {
    if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
        strcmp(token, "*") == 0 || strcmp(token, "/") == 0 ||
        strcmp(token, "^") == 0 || strcmp(token, "$") == 0 ||
        strcmp(token, "sqrt") == 0 || strcmp(token, "~") == 0) {
        return 1;
    }
    return 0;
}

double avaliateTree(treeNode *root) {
    if (root == NULL) return 0;

    if (!isOperator(root->valor)) {
        return atof(root->valor);
    }

    double leftVal = 0, rightVal = 0;

    if (root->left) leftVal = avaliateTree(root->left);
    if (root->right) rightVal = avaliateTree(root->right);

    if (strcmp(root->valor, "+") == 0) return leftVal + rightVal;
    if (strcmp(root->valor, "-") == 0) return leftVal - rightVal;
    if (strcmp(root->valor, "*") == 0) return leftVal * rightVal;
    if (strcmp(root->valor, "/") == 0) return leftVal / rightVal;
    if (strcmp(root->valor, "^") == 0 || strcmp(root->valor, "$") == 0) return pow(leftVal, rightVal);
    if (strcmp(root->valor, "~") == 0) return -leftVal;
    if (strcmp(root->valor, "sqrt") == 0) return sqrt(leftVal);

    return 0;
}

void printInOrder(treeNode *root) {
    if (root != NULL) {
        if (isOperator(root->valor)) printf("(");
        
        if (strcmp(root->valor, "sqrt") == 0) {
            printf("sqrt(");
            printInOrder(root->left);
            printf(")");
        } else if (strcmp(root->valor, "~") == 0) {
            printf("-");
            printInOrder(root->left);
        } else {
            printInOrder(root->left);
            printf("%s", root->valor);
            printInOrder(root->right);
        }
        
        if (isOperator(root->valor)) printf(")");
    }
}

void printTreeVisual(treeNode *root, int space) {
	int i = 0;
	
    if (root == NULL) return;
    
    space += 6;
    
    printTreeVisual(root->right, space);
    
    printf("\n");
    for (i; i < space; i++) {
        printf(" ");
    }
    printf("%s\n", root->valor);
    
    printTreeVisual(root->left, space);
}

void freeTree(treeNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

void freeList(listNode *top) {
    listNode *temp;
    while (top != NULL) {
        temp = top;
        top = top->next;
        freeTree(temp->root);
        free(temp);
    }
}
