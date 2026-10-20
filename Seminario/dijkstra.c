/* Dijkstra: menor caminho de um ponto de partida até todos os outros. */
#include <stdio.h>
#include <limits.h>

#define N 6
#define INFINITO INT_MAX

/* imprime o caminho voltando pelos "pais" (recursão) */
void trilha(int pai[], const char *nome[], int v) {
    if (pai[v] != -1) { trilha(pai, nome, pai[v]); printf(" -> "); }
    printf("%s", nome[v]);
}

int main(void) {
    const char *nome[N] = {"A", "B", "C", "D", "E", "F"};
    /* mapa[i][j] = distância entre i e j (0 = sem estrada) */
    int mapa[N][N] = {
        { 0,  7,  9,  0,  0, 14}, 
        { 7,  0, 10, 15,  0,  0},
        { 9, 10,  0, 11,  0,  2},
        { 0, 15, 11,  0,  6,  0},
        { 0,  0,  0,  6,  0,  9},
        {14,  0,  2,  0,  9,  0}};

    int gasto[N], pai[N], fechado[N] = {0};
    for (int i = 0; i < N; i++) { gasto[i] = INFINITO; pai[i] = -1; }
    gasto[0] = 0;                                  /* começa em A */

    for (int rodada = 0; rodada < N; rodada++) {
        int u = -1;                                /* vértice aberto mais barato */
        for (int i = 0; i < N; i++)
            if (!fechado[i] && (u < 0 || gasto[i] < gasto[u])) u = i;
        if (gasto[u] == INFINITO) break;
        fechado[u] = 1;

        for (int v = 0; v < N; v++)                /* relaxamento das arestas */
            if (mapa[u][v] && gasto[u] + mapa[u][v] < gasto[v]) {
                gasto[v] = gasto[u] + mapa[u][v];
                pai[v] = u;
            }
    }

    for (int v = 0; v < N; v++) {
        printf("A ate %s | custo %2d | ", nome[v], gasto[v]);
        trilha(pai, nome, v);
        printf("\n");
    }
    return 0;
}
