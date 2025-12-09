#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h> // Para garantir que os inteiros sejam grandes o suficiente

// Definições de constantes conforme solicitado
#define Q 12289 // O módulo (q)
#define N 256   // O numero de coeficientes (N)

// Prototipo da função para gerar e escrever no arquivo
void gerar_e_escrever(const char *nome_arquivo);

int main() {
    // Inicializa o gerador de números aleatórios com a hora atual.
    // Isso garante que os dois arquivos tenham sequências diferentes a cada execução.
    srand(time(NULL)); 

    printf("Gerando coeficientes aleatorios com Q=%d e N=%d.\n", Q, N);

    // Gera o primeiro arquivo
    gerar_e_escrever("coeficientes_a.txt");

    // Gera o segundo arquivo
    gerar_e_escrever("coeficientes_b.txt");

    printf("Arquivos 'coeficientes_a.txt' e 'coeficientes_b.txt' gerados com sucesso!\n");

    return 0;
}

/**
 * Gera N coeficientes aleatorios no intervalo [0, Q-1] e escreve-os no arquivo.
 * @param nome_arquivo O nome do arquivo a ser criado.
 */
void gerar_e_escrever(const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "w");

    if (arquivo == NULL) {
        perror("Erro ao abrir/criar o arquivo");
        return;
    }

    // Loop para gerar N coeficientes
    for (int i = 0; i < N; i++) {
        // Geracao do numero aleatorio: [0, Q-1]
        // rand() % Q garante que o valor esteja no intervalo [0, 12288]
        int coeficiente = rand() % Q; 
        
        // Escreve o coeficiente no arquivo. 
        // Note que separamos por espaco para facilitar a leitura futura (e.g., com fscanf).
        fprintf(arquivo, "%d ", coeficiente);

        // Opcional: Quebra de linha a cada 10 coeficientes para melhor visualização
        if ((i + 1) % 10 == 0) {
            fprintf(arquivo, "\n");
        }
    }

    fclose(arquivo);
}