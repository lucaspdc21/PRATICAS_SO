#include "escalonadores.h"
#include <stdlib.h>

// Estrutura auxiliar para guardar prioridade dinâmica durante a execução
typedef struct {
    int idx;
    int prioridade;
    int chegada; // tempo de chegada original (para desempate)
} FilaElem;

// Função de comparação de prioridades (menor valor = maior prioridade)
int cmp_fila(const void* a, const void* b) {
    FilaElem *fa = (FilaElem*)a, *fb = (FilaElem*)b;
    if (fa->prioridade != fb->prioridade)
        return fa->prioridade - fb->prioridade; // menor prioridade vem antes
    return fa->chegada - fb->chegada; // desempate por ordem de chegada
}

// First Come, First Served
int* fcfs(int* tasks, int qtd_task) {
    // TODO
}

// Shortest Job First
int* sjf(int* tasks, int qtd_task) {
    // TODO
}

// Shortest Remaining Time First
int* srtf(int* tasks, int qtd_task) {
    // TODO
}

// Prioridade cooperativo
int* prioc(int* tasks, int qtd_taskd) {
    // TODO
}

// Prioridade preemptivo
int* priop(int* tasks, int qtd_taskd) {
    // TODO
}

/*
 * Algoritmo de Round-Robin clássico (sem prioridade).
 * Recebe um vetor de processos, quantidade de processos e o quantum.
 * O vetor de processos deve estar inicializado (inclusive remaining_time = burst_time).
 * Retorna um vetor com os tempos de conclusão de cada processo.
 */
int* rr(Processo* processos, int qtd_proc, int quantum) {
    int* completion_times = malloc(qtd_proc * sizeof(int));
    int processos_finalizados = 0;
    int tempo_atual = 0;
    int* fila = malloc(qtd_proc * sizeof(int));
    int frente = 0, tras = 0;
    int* em_fila = calloc(qtd_proc, sizeof(int));
    int ultimo_em_execucao = -1;

    // Enfileira processos que chegam no tempo zero
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].arrival_time == 0) {
            fila[tras++] = i;
            em_fila[i] = 1;
        }
    }

    while (processos_finalizados < qtd_proc) {
        if (frente == tras) {
            // Nenhum processo pronto, avança tempo ao próximo arrival
            int proximo_arrival = -1;
            for (int i = 0; i < qtd_proc; i++) {
                if (!processos[i].finished && processos[i].arrival_time > tempo_atual) {
                    if (proximo_arrival == -1 || processos[i].arrival_time < proximo_arrival)
                        proximo_arrival = processos[i].arrival_time;
                }
            }
            tempo_atual = proximo_arrival;
            for (int i = 0; i < qtd_proc; i++) {
                if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= tempo_atual) {
                    fila[tras++] = i;
                    em_fila[i] = 1;
                }
            }
            continue;
        }

        int idx = fila[frente++];
        Processo *p = &processos[idx];

        // Executa quantum ou até o fim
        int tempo_exec = (p->remaining_time > quantum) ? quantum : p->remaining_time;
        tempo_atual += tempo_exec;
        p->remaining_time -= tempo_exec;

        // Verifica se novos processos chegaram nesse intervalo e enfileira
        for (int i = 0; i < qtd_proc; i++) {
            if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= tempo_atual) {
                fila[tras++] = i;
                em_fila[i] = 1;
            }
        }

        if (p->remaining_time == 0) {
            p->completion_time = tempo_atual;
            p->turnaround_time = p->completion_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            p->finished = 1;
            processos_finalizados++;
            completion_times[idx] = p->completion_time;
        } else {
            fila[tras++] = idx; // Reenfileira processo
        }

        ultimo_em_execucao = idx;
    }

    free(fila);
    free(em_fila);
    return completion_times;
}

/*
* Algoritmo de Round-Robin com prioridade e envelhecimento.
* Recebe um vetor de processos, quantidade de processos, quantum e valor de aging.
* O vetor de processos deve estar inicializado (inclusive remaining_time = burst_time e prioridade estática preenchida).
* A fila de execução é ordenada por prioridade dinâmica, atualizada a cada quantum por aging.
* Não há preempção por prioridade: o processo roda até o fim do quantum, depois a fila é reordenada.
* Retorna um vetor com os tempos de conclusão de cada processo.
*/
int* rr_d(Processo* processos, int qtd_proc, int quantum, int aging) {
    int time = 0;
    int* completion_times = malloc(qtd_proc * sizeof(int));
    int processos_finalizados = 0;

    // Vetor de prioridades dinâmicas (inicio com prioridade estática fornecida)
    int* prioridade_dinamica = malloc(qtd_proc * sizeof(int));
    for (int i = 0; i < qtd_proc; i++)
        prioridade_dinamica[i] = /* Precisa carregar da entrada (ex: processos[i].prioridade), */ 1;

    int* em_fila = calloc(qtd_proc, sizeof(int));
    FilaElem* fila = malloc(qtd_proc * sizeof(FilaElem));
    int fila_sz = 0;

    // Inicializa a fila com processos que chegam no tempo zero
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].arrival_time == 0) {
            fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
            em_fila[i] = 1;
        }
    }

    while (processos_finalizados < qtd_proc) {
        // Caso a fila esteja vazia, avança tempo para o próximo processo
        if (fila_sz == 0) {
            int proximo = -1;
            for (int i = 0; i < qtd_proc; i++)
                if (!processos[i].finished && processos[i].arrival_time > time &&
                    (proximo == -1 || processos[i].arrival_time < processos[proximo].arrival_time))
                    proximo = i;
            time = processos[proximo].arrival_time;
            for (int i = 0; i < qtd_proc; i++)
                if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= time) {
                    fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
                    em_fila[i] = 1;
                }
            continue;
        }

        // Ordena a fila por prioridade dinâmica (e chegada)
        qsort(fila, fila_sz, sizeof(FilaElem), cmp_fila);

        // Seleciona o primeiro processo da fila
        int idx = fila[0].idx;
        Processo *p = &processos[idx];

        // Executa quantum ou até o fim
        int exec_time = (p->remaining_time > quantum) ? quantum : p->remaining_time;
        time += exec_time;
        p->remaining_time -= exec_time;

        // Enfileira processos que chegaram durante esse quantum
        for (int i = 0; i < qtd_proc; i++) {
            if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= time) {
                fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
                em_fila[i] = 1;
            }
        }

        // Aging: todos os processos esperando na fila (exceto o executado) "sobem" em prioridade
        for (int f = 1; f < fila_sz; f++) { // começa de 1 pois 0 é o processo que acabou de rodar
            int proc_idx = fila[f].idx;
            prioridade_dinamica[proc_idx] -= aging;
            if (prioridade_dinamica[proc_idx] < 0) prioridade_dinamica[proc_idx] = 0; // Garante prioridade positiva
            fila[f].prioridade = prioridade_dinamica[proc_idx]; // Atualiza na fila
        }

        // Processo terminou?
        if (p->remaining_time == 0) {
            p->completion_time = time;
            p->turnaround_time = p->completion_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            p->finished = 1;
            processos_finalizados++;
            completion_times[idx] = p->completion_time;

            // Remove processo da fila
            for (int f = 0; f < fila_sz-1; f++) fila[f] = fila[f+1];
            fila_sz--;
        } else {
            // Move processo para o final da fila (regra do RR)
            FilaElem atual = fila[0];
            for (int f = 0; f < fila_sz-1; f++) fila[f] = fila[f+1];
            fila[fila_sz-1] = atual; // re-insere no final
            // prioridade já foi atualizada
        }
    }

    free(prioridade_dinamica);
    free(fila);
    free(em_fila);
    return completion_times;
}
