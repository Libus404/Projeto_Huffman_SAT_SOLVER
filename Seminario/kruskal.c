/* Kruskal: árvore geradora mínima (conecta tudo gastando o mínimo). */
#include <stdio.h>
#include <stdlib.h>

#define V 6
#define E 9

typedef struct { int a, b, peso; } Ponte;

int chefe[V];                                      /* Union-Find */

int achar(int x) { return chefe[x] == x ? x : (chefe[x] = achar(chefe[x])); }

int menorPrimeiro(const void *p, const void *q) {
    return ((const Ponte *)p)->peso - ((const Ponte *)q)->peso;
}

int main(void) {
    const char *nome[V] = {"A", "B", "C", "D", "E", "F"};
    Ponte pontes[E] = {
        {0,1,7}, {0,2,9}, {0,5,14}, {1,2,10}, {1,3,15},
        {2,3,11}, {2,5,2}, {3,4,6}, {4,5,9}};

    for (int i = 0; i < V; i++) chefe[i] = i;      /* cada um é seu próprio grupo */
    qsort(pontes, E, sizeof(Ponte), menorPrimeiro); /* 1) ordena por peso */

    int total = 0;
    for (int i = 0; i < E; i++) {                  /* 2) tenta cada ponte */
        int ra = achar(pontes[i].a), rb = achar(pontes[i].b);
        if (ra != rb) {                            /* grupos diferentes = sem ciclo */
            chefe[ra] = rb;                        /* 3) une os grupos */
            total += pontes[i].peso;
            printf("%s - %s (peso %d)\n", nome[pontes[i].a], nome[pontes[i].b], pontes[i].peso);
        }
    }
    printf("Custo total: %d\n", total);
    return 0;
}
