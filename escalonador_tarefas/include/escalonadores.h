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
    
    // Campo para armazenar o ID original em string 
    char* original_id_str;
} Processo;

// Estrutura de evento para timeline (usada por RR e RR_D)
typedef struct {
    int tempo_inicio;
    int pid;
} Evento;

// Timelines globais populadas por rr() e rr_d()
extern Evento* timeline_rr;
extern int timeline_rr_sz;
extern int timeline_rr_cap;

extern Evento* timeline_rrd;
extern int timeline_rrd_sz;
extern int timeline_rrd_cap;


// First Come, First Served
int* fcfs(Processo* processos, int qtd_task, int* out_max_time);

// Shortest Job First
int* sjf(Processo* processos, int qtd_task, int* out_max_time);

// Shortest Remaining Time First
int* srtf(Processo *novo, int qtd_task, int* out_matrix_width, int* out_max_time);

// Prioridade cooperativo
int* prioc(Processo* novo, int qtd_task, int* out_matrix_width, int* out_max_time);

// Prioridade preemptivo
int* priop(Processo* novo, int qtd_task, int* out_matrix_width, int* out_max_time);

// Round-Robin clássico (sem prioridade)
int* rr(Processo* processos, int qtd_proc, int quantum);

// Round-Robin com prioridade e envelhecimento
int* rr_d(Processo* processos, int qtd_proc, int quantum, int aging);

#endif