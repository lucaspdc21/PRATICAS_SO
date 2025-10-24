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
 * Recebe vetor de Processos, quantidade de processos e quantum.
 * O vetor de Processos deve ter remaining_time = burst_time.
 * Retorna vetor com tempos de conclusão de cada processo.
 * Também coleta a timeline (histórico das trocas de contexto).
 */
typedef struct {
    int tempo_inicio;
    int pid;
} Evento;

Evento* timeline_rr = NULL;
int timeline_rr_sz = 0;
int timeline_rr_cap = 0;

void rr_add_event(int tempo, int pid) {
    if (timeline_rr_cap == 0) {
        timeline_rr_cap = 128;
        timeline_rr = malloc(timeline_rr_cap * sizeof(Evento));
    } else if (timeline_rr_sz == timeline_rr_cap) {
        timeline_rr_cap *= 2;
        timeline_rr = realloc(timeline_rr, timeline_rr_cap * sizeof(Evento));
    }
    timeline_rr[timeline_rr_sz].tempo_inicio = tempo;
    timeline_rr[timeline_rr_sz].pid = pid;
    timeline_rr_sz++;
}

int* rr(Processo* processos, int qtd_proc, int quantum) {
    int* completion_times = malloc(qtd_proc * sizeof(int));
    int processos_finalizados = 0;
    int tempo_atual = 0;
    int* fila = malloc(qtd_proc * sizeof(int));
    int frente = 0, tras = 0;
    int* em_fila = calloc(qtd_proc, sizeof(int));
    int ultimo_processo = -1;

    timeline_rr_sz = 0; // zera a timeline global

    // Enfileira processos com chegada zero
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].arrival_time == 0) {
            fila[tras++] = i;
            em_fila[i] = 1;
        }
    }

    while (processos_finalizados < qtd_proc) {
        if (frente == tras) {
            // Avança tempo até alguém chegar
            int prox = -1;
            for (int i = 0; i < qtd_proc; i++)
                if (!processos[i].finished && processos[i].arrival_time > tempo_atual &&
                    (prox == -1 || processos[i].arrival_time < processos[prox].arrival_time))
                    prox = i;
            tempo_atual = processos[prox].arrival_time;
            for (int i = 0; i < qtd_proc; i++)
                if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= tempo_atual) {
                    fila[tras++] = i;
                    em_fila[i] = 1;
                }
            continue;
        }

        int idx = fila[frente++];
        Processo *p = &processos[idx];

        // Registra na timeline se houve troca de contexto
        if (ultimo_processo != idx) {
            rr_add_event(tempo_atual, p->id);
            ultimo_processo = idx;
        }

        // Executa quantum ou até o fim
        int tempo_exec = (p->remaining_time > quantum) ? quantum : p->remaining_time;
        tempo_atual += tempo_exec;
        p->remaining_time -= tempo_exec;

        // Entrada de processos durante esse quantum
        for (int i = 0; i < qtd_proc; i++)
            if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= tempo_atual) {
                fila[tras++] = i;
                em_fila[i] = 1;
            }

        if (p->remaining_time == 0) {
            p->completion_time = tempo_atual;
            p->turnaround_time = p->completion_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            p->finished = 1;
            processos_finalizados++;
            completion_times[idx] = p->completion_time;
        } else {
            fila[tras++] = idx;
        }
    }

    free(fila);
    free(em_fila);
    return completion_times;
}

/*
 * Algoritmo de Round-Robin com prioridade e envelhecimento.
 * Recebe vetor de Processos, quantidade de processos, quantum e valor de aging.
 * O vetor de Processos deve ter remaining_time = burst_time e prioridade preenchida.
 * Fila ordenada por prioridade dinâmica, atualizada a cada quantum.
 * Não há preempção por prioridade (apenas rotina de RR).
 * Retorna vetor com tempos de conclusão de cada processo.
 * Também coleta o diagrama de tempo (timeline).
 */
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

Evento* timeline_rrd = NULL;
int timeline_rrd_sz = 0;
int timeline_rrd_cap = 0;

void rrd_add_event(int tempo, int pid) {
    if (timeline_rrd_cap == 0) {
        timeline_rrd_cap = 128;
        timeline_rrd = malloc(timeline_rrd_cap * sizeof(Evento));
    } else if (timeline_rrd_sz == timeline_rrd_cap) {
        timeline_rrd_cap *= 2;
        timeline_rrd = realloc(timeline_rrd, timeline_rrd_cap * sizeof(Evento));
    }
    timeline_rrd[timeline_rrd_sz].tempo_inicio = tempo;
    timeline_rrd[timeline_rrd_sz].pid = pid;
    timeline_rrd_sz++;
}

int* rr_d(Processo* processos, int qtd_proc, int quantum, int aging) {
    int time = 0;
    int* completion_times = malloc(qtd_proc * sizeof(int));
    int processos_finalizados = 0;

    timeline_rrd_sz = 0; // zera a timeline global
    int ultimo_processo = -1;

    int* prioridade_dinamica = malloc(qtd_proc * sizeof(int));
    for (int i = 0; i < qtd_proc; i++)
        prioridade_dinamica[i] = processos[i].prioridade;

    int* em_fila = calloc(qtd_proc, sizeof(int));
    FilaElem* fila = malloc(qtd_proc * sizeof(FilaElem));
    int fila_sz = 0;

    // Inicializa fila com processos que chegam em t=0
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].arrival_time == 0) {
            fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
            em_fila[i] = 1;
        }
    }

    while (processos_finalizados < qtd_proc) {
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

        qsort(fila, fila_sz, sizeof(FilaElem), cmp_fila);

        int idx = fila[0].idx;
        Processo* p = &processos[idx];

        // Registra diagrama de tempo
        if (ultimo_processo != idx) {
            rrd_add_event(time, p->id);
            ultimo_processo = idx;
        }

        int exec_time = (p->remaining_time > quantum) ? quantum : p->remaining_time;
        time += exec_time;
        p->remaining_time -= exec_time;

        // Entrada de processos durante quantum
        for (int i = 0; i < qtd_proc; i++)
            if (!em_fila[i] && !processos[i].finished && processos[i].arrival_time <= time) {
                fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
                em_fila[i] = 1;
            }

        // Aging dos demais processos na fila
        for (int f = 1; f < fila_sz; f++) {
            int proc_idx = fila[f].idx;
            prioridade_dinamica[proc_idx] -= aging;
            if (prioridade_dinamica[proc_idx] < 0) prioridade_dinamica[proc_idx] = 0;
            fila[f].prioridade = prioridade_dinamica[proc_idx];
        }

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
            // Reinfileira o processo
            FilaElem atual = fila[0];
            for (int f = 0; f < fila_sz-1; f++) fila[f] = fila[f+1];
            fila[fila_sz-1] = atual;
        }
    }
    free(prioridade_dinamica);
    free(fila);
    free(em_fila);
    return completion_times;
}
