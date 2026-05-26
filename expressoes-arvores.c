#include<stdio.h>
#include<stdlib.h>

int main(void){
	int choice = 0;
	
	printf("Escolha uma das opcoes:");
	printf("1 - Adicionar uma nova expressao");
	printf("2 - Mostrar estruturas das arvores");
	printf("3 - Encerrar o programa");
	scanf("%d", &choice);
	
	switch(choice){
		case 1:
			break;
		
		case 2:
			break;
			
		case 3:
			return 0;
	}
}
