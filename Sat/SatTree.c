#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CLAUSULAS 100
#define MAX_LITERAIS 20
#define MAX_VARIAVEIS 50

typedef struct {
    int mat[MAX_CLAUSULAS][MAX_LITERAIS];
    int qtd_literais[MAX_CLAUSULAS];
    int num_clausulas;
    int num_variaveis;
} FormulaCNF;

int interpretacao[MAX_VARIAVEIS + 1];

typedef struct No {
    int var;
    int valor;
    int status;
    struct No* esq;
    struct No* dir;
} No;

No* criar_no(int var, int valor, int status) {
    No* novo = (No*) malloc(sizeof(No));
    novo->var = var;
    novo->valor = valor;
    novo->status = status;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}

// Imprime espaços no terminal para refletir a profundidade da árvore
void imprimir_indentacao(int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }
}

// LEITURA DIMACS E AVALIAÇÃO COM VALIDAÇÃO DE ERRO DE SINTAXE
bool carregar_dimacs_cnf(const char* caminho_arquivo, FormulaCNF* f) {
    FILE* file = fopen(caminho_arquivo, "r");
    if (!file) return false;

    char buffer[256];
    f->num_clausulas = 0;
    f->num_variaveis = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (buffer[0] == 'c' || buffer[0] == '\n' || buffer[0] == '\r') continue;
        if (buffer[0] == 'p') {
            sscanf(buffer, "p cnf %d %d", &f->num_variaveis, &f->num_clausulas);
            
            // Valida se o número de variáveis ultrapassa a capacidade suportada
            if (f->num_variaveis > MAX_VARIAVEIS) {
                printf("[ERRO SINTATICO] O arquivo declara %d variaveis, mas o limite maximo e %d!\n", 
                       f->num_variaveis, MAX_VARIAVEIS);
                fclose(file);
                return false;
            }

            f->num_clausulas = 0; // Reinicia contador real de cláusulas lidas
            continue;
        }

        int pos = 0;
        char* token = strtok(buffer, " \t\n\r");
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break;

            int var = abs(literal);

            // VALIDAÇÃO DE ERRO DE SINTAXE (Garantia do limite da variável)
            if (var > f->num_variaveis) {
                printf("[ERRO SINTATICO] Variavel x%d encontrada na clausula, mas o cabecalho declarou apenas %d variaveis!\n", 
                       var, f->num_variaveis);
                fclose(file);
                return false; // Interrompe a leitura por erro de malformação do arquivo
            }

            f->mat[f->num_clausulas][pos++] = literal;
            token = strtok(NULL, " \t\n\r");
        }
        if (pos > 0) {
            f->qtd_literais[f->num_clausulas] = pos;
            f->num_clausulas++;
        }
    }
    fclose(file);
    return true;
}

int avalia_formula(FormulaCNF* f) {
    bool todas_satisfeitas = true;

    for (int i = 0; i < f->num_clausulas; i++) {
        bool clausula_satisfeita = false;
        bool tem_indefinido = false;

        for (int j = 0; j < f->qtd_literais[i]; j++) {
            int lit = f->mat[i][j];
            int var = abs(lit);
            int valor = interpretacao[var];

            if (valor == -1) {
                tem_indefinido = true;
            } else if ((lit > 0 && valor == 1) || (lit < 0 && valor == 0)) {
                clausula_satisfeita = true;
                break;
            }
        }

        if (clausula_satisfeita) continue;
        if (!tem_indefinido) return 0; // PODA!
        todas_satisfeitas = false;
    }

    if (todas_satisfeitas) return 1; // SAT!
    return -1; // INDEFINIDO
}


// FUNÇÃO RECURSIVA

void sat_solver_arvore(FormulaCNF* f, No* pai, int var_atual, int valor_atribuido, bool* encontrou_solucao, int nivel) {
    if (*encontrou_solucao) return;

    interpretacao[var_atual] = valor_atribuido;

    imprimir_indentacao(nivel);
    printf("(Nivel %d) Testando x%d = %s...\n", nivel, var_atual, valor_atribuido == 1 ? "TRUE" : "FALSE");

    int status = avalia_formula(f);

    No* no_atual = criar_no(var_atual, valor_atribuido, status);

    if (valor_atribuido == 1) {
        pai->esq = no_atual;
    } else {
        pai->dir = no_atual;
    }

    if (status == 1) {
        imprimir_indentacao(nivel);
        printf("[SAT] Formula satisfeita!\n");
        *encontrou_solucao = true;
        return;
    }

    if (status == 0) {
        imprimir_indentacao(nivel);
        printf("[PODA] Contradicao encontrada!\n");
        return;
    }

    int proxima_var = -1;
    for (int i = 1; i <= f->num_variaveis; i++) {
        if (interpretacao[i] == -1) {
            proxima_var = i;
            break;
        }
    }

    if (proxima_var == -1) return;

    // Ramo da Esquerda: TRUE (1)
    sat_solver_arvore(f, no_atual, proxima_var, 1, encontrou_solucao, nivel + 1);

    // Ramo da Direita: FALSE (0) [Backtracking]
    if (!(*encontrou_solucao)) {
        sat_solver_arvore(f, no_atual, proxima_var, 0, encontrou_solucao, nivel + 1);
    }

    // Desfaz atribuição para backtracking
    if (!(*encontrou_solucao)) {
        interpretacao[proxima_var] = -1;
    }
}

void liberar_arvore(No* raiz) {
    if (raiz == NULL) return;
    liberar_arvore(raiz->esq);
    liberar_arvore(raiz->dir);
    free(raiz);
}

// ------------------------------------------------------------------------------
// FUNÇÃO PRINCIPAL
// ------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    char nome_arquivo[256];
    FormulaCNF formula;

    if (argc > 1) {
        strcpy(nome_arquivo, argv[1]);
    } else {
        printf("Insira o nome do arquivo .cnf: ");
        if (scanf("%255s", nome_arquivo) != 1) return 1;
    }

    if (!carregar_dimacs_cnf(nome_arquivo, &formula)) {
        printf("Execucao interrompida devido a falha na leitura ou erro de sintaxe no arquivo.\n");
        return 1;
    }

    for (int i = 1; i <= formula.num_variaveis; i++) interpretacao[i] = -1;

    printf("\n--- Executando SAT Solver ---\n");

    No* raiz = criar_no(0, -1, -1);
    bool eh_sat = false;
    int primeira_var = 1;

    // Dispara a busca testando a primeira variável como TRUE (1) no nível 0
    sat_solver_arvore(&formula, raiz, primeira_var, 1, &eh_sat, 0);

    // Se o ramo TRUE falhou, testa a primeira variável como FALSE (0) no nível 0
    if (!eh_sat) {
        sat_solver_arvore(&formula, raiz, primeira_var, 0, &eh_sat, 0);
    }

    printf("\n--- Resultado Final ---\n");
    if (eh_sat) {
        printf("SATISFATIVEL! (SAT)\n");
        printf("Valores encontrados:\n");
        for (int i = 1; i <= formula.num_variaveis; i++) {
            printf("  x%d = %s\n", i, interpretacao[i] == 1 ? "TRUE" : (interpretacao[i] == 0 ? "FALSE" : "LIVRE"));
        }
    } else {
        printf("INSATISFATIVEL! (UNSAT)\n");
    }

    liberar_arvore(raiz);
    return 0;
}