#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CLAUSULAS 100
#define MAX_LITERAIS 20
#define MAX_VARIAVEIS 50

// Estrutura que representa a fórmula em CNF
typedef struct {
    int mat[MAX_CLAUSULAS][MAX_LITERAIS]; // Matriz: linhas = clausulas, colunas = literais
    int qtd_literais[MAX_CLAUSULAS];       // Quantidade de literais em cada clausula
    int num_clausulas;                     // Total de clausulas
    int num_variaveis;                     // Total de variaveis
} FormulaCNF;

// Estado das variáveis: 1 (TRUE), 0 (FALSE), -1 (INDEFINIDO)
int interpretacao[MAX_VARIAVEIS + 1];

// ==============================================================================
// 1. LEITURA DO ARQUIVO DIMACS (.cnf)
// ==============================================================================
bool carregar_dimacs_cnf(const char* caminho_arquivo, FormulaCNF* f) {
    FILE* file = fopen(caminho_arquivo, "r");
    if (!file) {
        printf("Erro ao abrir o arquivo '%s'!\n", caminho_arquivo);
        return false;
    }

    char buffer[256];
    f->num_clausulas = 0;
    f->num_variaveis = 0;
    int total_esperado = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        // Ignora comentarios e linhas vazias
        if (buffer[0] == 'c' || buffer[0] == '\n' || buffer[0] == '\r') {
            continue;
        }

        // Linha de definicao: p cnf <variaveis> <clausulas>
        if (buffer[0] == 'p') {
            sscanf(buffer, "p cnf %d %d", &f->num_variaveis, &total_esperado);
            continue;
        }

        // Le os literais da clausula (terminam com 0)
        int pos = 0;
        char* token = strtok(buffer, " \t\n\r");
        while (token != NULL) {
            int literal = atoi(token);
            if (literal == 0) break; // 0 indica fim da clausula no formato DIMACS

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

// ==============================================================================
// 2. AVALIAÇÃO DA FÓRMULA (I => F  ou  I => ~F)
// ==============================================================================
// Retorna:
//  1: Todas as clausulas satisfeitas (SAT)
//  0: Pelo menos uma clausula falsa (PODA por contradicao)
// -1: Indefinido (precisa de mais atribuicoes)
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
            } else {
                // Literal positivo com var=1 OU literal negativo com var=0
                if ((lit > 0 && valor == 1) || (lit < 0 && valor == 0)) {
                    clausula_satisfeita = true;
                    break;
                }
            }
        }

        // Se a clausula ja foi satisfeita por algum literal, passa para a proxima
        if (clausula_satisfeita) continue;

        // Se nao foi satisfeita e nao tem literais indefinidos, ela virou FALSA
        // Uma clausula falsa torna toda a conjuncao (AND) falsa -> PODA!
        if (!tem_indefinido) return 0;

        todas_satisfeitas = false;
    }

    if (todas_satisfeitas) return 1;

    return -1; // Indefinido
}

// Imprime indentacao para visualizar a profundidade da arvore recursiva
void imprimir_indentacao(int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }
}

// ==============================================================================
// 3. SAT SOLVER RECURSIVO (Árvore de Decisão Binária com Poda e Backtracking)
// ==============================================================================
bool sat_solver(FormulaCNF* f, int nivel) {
    int status = avalia_formula(f);

    // Caso 1: Formula satisfeita (I => F)
    if (status == 1) {
        imprimir_indentacao(nivel);
        printf("[SAT] Formula satisfeita!\n");
        return true;
    }

    // Caso 2: Poda por contradicao (I => ~F)
    if (status == 0) {
        imprimir_indentacao(nivel);
        printf("[PODA] Formula falsa!\n");
        return false;
    }

    // Escolhe a primeira variavel ainda nao atribuida
    int x = -1;
    for (int i = 1; i <= f->num_variaveis; i++) {
        if (interpretacao[i] == -1) {
            x = i;
            break;
        }
    }

    if (x == -1) return false;

    // Ramo 1: Tenta x = TRUE (1)
    imprimir_indentacao(nivel);
    printf("? Testando x%d = TRUE...\n", x);
    interpretacao[x] = 1;
    if (sat_solver(f, nivel + 1)) return true;

    // Ramo 2: Tenta x = FALSE (0) [Backtracking]
    imprimir_indentacao(nivel);
    printf("? Testando x%d = FALSE...\n", x);
    interpretacao[x] = 0;
    if (sat_solver(f, nivel + 1)) return true;

    // Desfaz a atribuicao (Backtracking)
    interpretacao[x] = -1;
    return false;
}

// ==============================================================================
// 4. FUNÇÃO PRINCIPAL
// ==============================================================================
int main(int argc, char* argv[]) {
    char nome_arquivo[256];
    FormulaCNF formula;

    // Permite passar por argumento ou digitar no terminal
    if (argc > 1) {
        strcpy(nome_arquivo, argv[1]);
    } else {
        printf("Insira o nome ou caminho do arquivo .cnf: ");
        if (scanf("%255s", nome_arquivo) != 1) {
            printf("Erro na leitura do arquivo.\n");
            return 1;
        }
    }

    printf("\n--- Lendo arquivo DIMACS: '%s' ---\n", nome_arquivo);
    if (!carregar_dimacs_cnf(nome_arquivo, &formula)) {
        return 1;
    }

    printf("Variaveis: %d | Clausulas: %d\n", formula.num_variaveis, formula.num_clausulas);

    // Inicializa todas as variaveis como -1 (INDEFINIDO)
    for (int i = 1; i <= formula.num_variaveis; i++) {
        interpretacao[i] = -1;
    }

    printf("\n--- Processo de Decisao ---\n");
    bool eh_sat = sat_solver(&formula, 0);

    printf("\n--- Resultado Final ---\n");
    if (eh_sat) {
        printf("SATISFATIVEL! (SAT)\n");
        printf("Valores encontrados:\n");
        for (int i = 1; i <= formula.num_variaveis; i++) {
            if (interpretacao[i] == 1) {
                printf("  x%d = TRUE\n", i);
            } else if (interpretacao[i] == 0) {
                printf("  x%d = FALSE\n", i);
            } else {
                printf("  x%d = LIVRE (tanto faz)\n", i);
            }
        }
    } else {
        printf("INSATISFATIVEL! (UNSAT)\n");
    }

    return 0;
}
