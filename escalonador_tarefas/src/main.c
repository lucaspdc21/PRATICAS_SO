#include "cJSON.h"
#include "escalonadores.h"
#include <stdio.h>

int main() {
    // TODO
    // Lembrar de ler o arquivo de config.txt
    // Exemplo:
    //    void ler_config(int *quantum, int *aging) {
    //        FILE *fp = fopen("config.txt", "r");
    //        if (!fp) {
    //            printf("Erro ao abrir config.txt\n");
    //            exit(1);
    //        }
    //        char linha[100];
    //        fgets(linha, sizeof(linha), fp);
    //        fclose(fp);
    //
    //        // Esperado: quantum:2 aging:1
    //        sscanf(linha, "quantum:%d aging:%d", quantum, aging);

    // Fluxo:
    // 1. Ler JSON de entrada
    // 2. Preencher array de Processo, quantum, aging etc.
    // 3. Selecionar algoritmo e chamar função
    // 4. Preencher as métricas de saída
    // 5. Gerar JSON de saída
    // 6. Salvar arquivo ou imprimir
}
