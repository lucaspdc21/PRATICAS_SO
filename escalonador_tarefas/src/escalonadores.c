#include "escalonadores.h"
#include <stdlib.h>

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

// Round-Robin com prioridade e aging
int* rr_d(int* tasks, int qtd_taskd, int quantum, int alfa) {
    // TODO
}
