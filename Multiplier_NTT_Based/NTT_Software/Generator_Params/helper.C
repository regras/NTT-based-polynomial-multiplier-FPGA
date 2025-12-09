#include <stdio.h>
#include <stdlib.h>

// Função auxiliar: Algoritmo Euclidiano Estendido
int egcd(int a, int b, int *x, int *y) {
    if (a == 0) {
        *x = 0;
        *y = 1;
        return b;
    }

    int x1, y1;
    int g = egcd(b % a, a, &x1, &y1);

    *x = y1 - (b / a) * x1;
    *y = x1;

    return g;
}

// Função principal: Inverso modular de a mod m
int modinv(int a, int m) {
    int x, y;
    int g = egcd(a, m, &x, &y);

    if (g != 1) {
        // Inverso não existe
        fprintf(stderr, "Erro: inverso modular não existe para a=%d e m=%d\n", a, m);
        exit(EXIT_FAILURE);
    } else {
        // Ajuste para resultado positivo
        int res = (x % m + m) % m;
        return res;
    }
}
