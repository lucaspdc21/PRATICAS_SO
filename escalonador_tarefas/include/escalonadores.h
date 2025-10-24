#include<limits.h>
#ifndef ESCALONADORES_H
#define ESCALONADORES_H

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    int prioridade;
    int finished;
} Processo;

// First Come, First Served
int* fcfs(Processo* processos, int qtd_task);

// Shortest Job First
int* sjf(Processo* processos, int qtd_task);

// Shortest Remaining Time First
int* srtf(Processo* processos, int qtd_task);

// Prioridade cooperativo
int* prioc(Processo* processos, int qtd_taskd);

// Prioridade preemptivo
int* priop(Processo* processos, int qtd_taskd);

// Round-Robin clássico (sem prioridade)
int* rr(Processo* processos, int qtd_proc, int quantum);

// Round-Robin com prioridade e envelhecimento
int* rr_d(Processo* processos, int qtd_proc, int quantum, int aging);

#endif
