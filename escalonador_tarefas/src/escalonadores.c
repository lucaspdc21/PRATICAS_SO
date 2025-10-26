#include "../include/escalonadores.h"
#include <stdlib.h>
#include <limits.h> 

void ordenar_por_chegada(Processo p[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (p[j].arrival_time < p[i].arrival_time) {
                Processo tmp = p[i];
                p[i] = p[j];
                p[j] = tmp;
            }
        }
    }
}

// First Come, First Served
int* fcfs(Processo* processos, int qtd_task, int* out_max_time) {
    ordenar_por_chegada(processos, qtd_task);
    int tempo = 0;
    int tempo_total_estimado = 0;

    // calcula tempo total aproximado
    int max_arrival = 0;
    for (int i = 0; i < qtd_task; i++) {
        tempo_total_estimado += processos[i].burst_time;
        if (processos[i].arrival_time > max_arrival) {
            max_arrival = processos[i].arrival_time;
        }
    }
    tempo_total_estimado += max_arrival;
    if (tempo_total_estimado == 0) tempo_total_estimado = 1;

    // aloca timeline com valores -1
    int* timeline = (int*) malloc(sizeof(int) * tempo_total_estimado * 2); // Aloca com folga
    for (int t = 0; t < tempo_total_estimado * 2; t++)
        timeline[t] = -1;

    for (int i = 0; i < qtd_task; i++) {
        if (tempo < processos[i].arrival_time)
            tempo = processos[i].arrival_time;

        for (int t = 0; t < processos[i].burst_time; t++) {
            if (tempo >= tempo_total_estimado * 2) { 
                // Realocação de emergência se a estimativa estourar
                int nova_cap = tempo * 2;
                timeline = realloc(timeline, sizeof(int) * nova_cap);
                for(int k = tempo_total_estimado * 2; k < nova_cap; k++) timeline[k] = -1;
                tempo_total_estimado = (nova_cap / 2);
            }
            timeline[tempo] = processos[i].id; // marca qual processo roda nesse tempo
            tempo++;
        }

        processos[i].completion_time = tempo;
        processos[i].turnaround_time = processos[i].completion_time - processos[i].arrival_time;
        processos[i].waiting_time = processos[i].turnaround_time - processos[i].burst_time;
    }
    
    *out_max_time = tempo; // Retorna o tempo final real
    timeline = realloc(timeline, tempo * sizeof(int)); // Realoca para o tamanho exato

    // retorna timeline (vetor de IDs)
    return timeline;
}

// Função auxiliar: encontra o índice do processo disponível com menor burst
int menor_burst_disponivel(Processo p[], int n, int tempo) {
    int idx = -1;
    int menor_burst = INT_MAX; 

    for (int i = 0; i < n; i++) {
        if (!p[i].finished && p[i].arrival_time <= tempo) {
            if (p[i].burst_time < menor_burst) {
                menor_burst = p[i].burst_time;
                idx = i;
            }
        }
    }
    return idx;
}

// Shortest Job First
int* sjf(Processo* processos, int qtd_task, int* out_max_time) {
    for (int i = 0; i < qtd_task; i++) {
        processos[i].remaining_time = processos[i].burst_time;
        processos[i].finished = 0;
    }

    int tempo = 0, concluidos = 0;

    // Estimativa do tempo total para alocação do vetor timeline
    int tempo_estimado = 0;
    int max_arrival = 0;
    for (int i = 0; i < qtd_task; i++) {
        tempo_estimado += processos[i].burst_time;
        if (processos[i].arrival_time > max_arrival)
            max_arrival = processos[i].arrival_time;
    }
    tempo_estimado += max_arrival;
    if (tempo_estimado == 0) tempo_estimado = 1;


    // Aloca timeline (cada posição indica qual processo roda naquele instante)
    int* timeline = (int*)calloc(tempo_estimado * 2, sizeof(int));
    for(int t = 0; t < tempo_estimado * 2; t++) timeline[t] = -1; // Inicia como Idle

    while (concluidos < qtd_task) {
        int idx = menor_burst_disponivel(processos, qtd_task, tempo);
        if (idx == -1) { 
            if (tempo >= tempo_estimado * 2) {
                 int nova_cap = tempo * 2;
                timeline = realloc(timeline, sizeof(int) * nova_cap);
                for(int k = tempo_estimado * 2; k < nova_cap; k++) timeline[k] = -1;
                tempo_estimado = (nova_cap / 2);
            }
            timeline[tempo] = -1; // CPU ociosa
            tempo++; 
            continue; 
        }

        for (int t = 0; t < processos[idx].burst_time; t++) {
             if (tempo >= tempo_estimado * 2) {
                 int nova_cap = tempo * 2;
                timeline = realloc(timeline, sizeof(int) * nova_cap);
                for(int k = tempo_estimado * 2; k < nova_cap; k++) timeline[k] = -1;
                tempo_estimado = (nova_cap / 2);
            }
            timeline[tempo] = processos[idx].id;
            tempo++;
        }

        processos[idx].completion_time = tempo;
        processos[idx].turnaround_time = processos[idx].completion_time - processos[idx].arrival_time;
        processos[idx].waiting_time = processos[idx].turnaround_time - processos[idx].burst_time;
        processos[idx].finished = 1;
        concluidos++;
    }

    *out_max_time = tempo; // Retorna o tempo final real
    timeline = realloc(timeline, tempo * sizeof(int));

    return timeline;
}

// Função auxiliar para calcular a largura da matriz de timeline
static int get_matrix_width_estimate(Processo* p, int n) {
    int sum_burst = 0;
    int max_arrival = 0;
    for(int i = 0; i < n; i++) {
        sum_burst += p[i].burst_time;
        if(p[i].arrival_time > max_arrival) {
            max_arrival = p[i].arrival_time;
        }
    }
    // Estimativa: (soma dos bursts + maior chegada) * 2 (folga)
    int estimate = (sum_burst + max_arrival) * 2;
    return (estimate > 0) ? estimate : 1;
}


// Shortest Remaining Time First
int* srtf(Processo *novo, int qtd_task, int* out_matrix_width, int* out_max_time) {
    int tempo = 0,
    proc_concluidos = 0,
    pid,
    pronto_tras = -1,
    index_maior, menor_restante;
    Processo *pronto[qtd_task];

    for (int i = 0; i < qtd_task; ++i) {
        novo[i].remaining_time = novo[i].burst_time;
        novo[i].finished = 0;
    }

    // Usa estimativa para largura da matriz
    int matrix_width = get_matrix_width_estimate(novo, qtd_task);
    *out_matrix_width = matrix_width;
    int* timeline = calloc(qtd_task * matrix_width, sizeof(int));

    while (proc_concluidos < qtd_task) {
        // Adiciona os processos do tempo atual no pronto
        for (int i = 0; i < qtd_task; ++i) {
            if (novo[i].finished != 1 && novo[i].arrival_time == tempo)
                pronto[++pronto_tras] = &novo[i];
        }

        // Checa se precisa realocar
        if (tempo >= matrix_width) {
            int nova_width = matrix_width * 2;
            int* new_timeline = calloc(qtd_task * nova_width, sizeof(int));
            for(int p = 0; p < qtd_task; p++) {
                for(int t = 0; t < matrix_width; t++) {
                    new_timeline[p * nova_width + t] = timeline[p * matrix_width + t];
                }
            }
            free(timeline);
            timeline = new_timeline;
            matrix_width = nova_width;
            *out_matrix_width = matrix_width;
        }

        // Marcando processos prontos, mas não executados
        for (int i = 0; i <= pronto_tras; i++) {
            int pid = pronto[i]->id;
            timeline[pid * matrix_width + tempo] = 1;
        }

        // Seleciona processo com menor tempo restante
        index_maior = -1, menor_restante = INT_MAX;
        for (int i = 0; i <= pronto_tras; ++i) {
            if (pronto[i]->remaining_time < menor_restante) {
                menor_restante = pronto[i]->remaining_time;
                index_maior = i;
            }
        }

        // Nenhum processo pronto, CPU ociosa
        if (index_maior == -1) {
            tempo++;
            continue;
        }

        // Executa uma unidade de tempo
        pronto[index_maior]->remaining_time--;
        pid = pronto[index_maior]->id;
        timeline[pid * matrix_width + tempo] = 2;
        tempo++;

        // Se processo terminou
        if (pronto[index_maior]->remaining_time == 0) {
            pronto[index_maior]->completion_time = tempo;
            pronto[index_maior]->finished = 1;
            pronto[index_maior] = pronto[pronto_tras--];
            proc_concluidos++;
        }
    }

    // Calcula turnaround e waiting time
    for (int i = 0; i < qtd_task; i++) {
        novo[i].turnaround_time = novo[i].completion_time - novo[i].arrival_time;
        novo[i].waiting_time = novo[i].turnaround_time - novo[i].burst_time;
    }
    
    *out_max_time = tempo;
    // Realocar para tamanho exato
    if (tempo < matrix_width) {
        int* new_timeline = calloc(qtd_task * tempo, sizeof(int));
         for(int p = 0; p < qtd_task; p++) {
            for(int t = 0; t < tempo; t++) {
                new_timeline[p * tempo + t] = timeline[p * matrix_width + t];
            }
        }
        free(timeline);
        timeline = new_timeline;
        *out_matrix_width = tempo;
    }

    return timeline;
}

// Escalonamento por Prioridade Cooperativo (não preemptivo)
int* prioc(Processo* novo, int qtd_task, int* out_matrix_width, int* out_max_time) {
    int tempo = 0, tempo_ant = 0;
    int pid, tempo_proc, proc_concluidos = 0;
    int pronto_tras = -1, index_maior, maior_prioridade;

    Processo* pronto[qtd_task];

    // Inicializa tempos restantes e flags
    for (int i = 0; i < qtd_task; ++i) {
        novo[i].remaining_time = novo[i].burst_time;
        novo[i].finished = 0;
    }

    // Estima tamanho inicial da matriz timeline
    int matrix_width = get_matrix_width_estimate(novo, qtd_task);
    *out_matrix_width = matrix_width;
    int* timeline = calloc(qtd_task * matrix_width, sizeof(int));

    while (proc_concluidos < qtd_task) {

        // 🔹 Adiciona processos que já chegaram e ainda não foram finalizados
        for (int i = 0; i < qtd_task; ++i) {
            if (!novo[i].finished && novo[i].arrival_time <= tempo) {
                // Evita duplicar processo na fila pronto
                int ja_presente = 0;
                for (int j = 0; j <= pronto_tras; j++) {
                    if (pronto[j] == &novo[i]) {
                        ja_presente = 1;
                        break;
                    }
                }
                if (!ja_presente)
                    pronto[++pronto_tras] = &novo[i];
            }
        }

        // 🔹 Checa se precisa realocar a timeline
        if (tempo >= matrix_width) {
            int nova_width = matrix_width * 2;
            int* new_timeline = calloc(qtd_task * nova_width, sizeof(int));
            for (int p = 0; p < qtd_task; p++) {
                for (int t = 0; t < matrix_width; t++) {
                    new_timeline[p * nova_width + t] = timeline[p * matrix_width + t];
                }
            }
            free(timeline);
            timeline = new_timeline;
            matrix_width = nova_width;
            *out_matrix_width = matrix_width;
        }

        // 🔹 Marca processos prontos (em espera)
        for (int i = 0; i <= pronto_tras; i++) {
            pid = pronto[i]->id;
            for (int j = tempo_ant; j < tempo; j++) {
                if (timeline[pid * matrix_width + j] == 0)
                    timeline[pid * matrix_width + j] = 1;
            }
        }

        // 🔹 Seleciona o processo com maior prioridade
        index_maior = -1;
        maior_prioridade = -1;
        for (int i = 0; i <= pronto_tras; ++i) {
            if (pronto[i]->prioridade > maior_prioridade) {
                maior_prioridade = pronto[i]->prioridade;
                index_maior = i;
            }
        }

        // 🔹 Nenhum processo pronto → CPU ociosa
        if (index_maior == -1) {
            tempo++;
            continue;
        }

        // 🔹 Executa o processo selecionado (sem preempção)
        if (pronto[index_maior]->remaining_time <= 0)
            pronto[index_maior]->remaining_time = 0;

        tempo_proc = tempo + pronto[index_maior]->remaining_time;

        // 🔹 Realoca se necessário
        if (tempo_proc >= matrix_width) {
            int nova_width = tempo_proc * 2;
            int* new_timeline = calloc(qtd_task * nova_width, sizeof(int));
            for (int p = 0; p < qtd_task; p++) {
                for (int t = 0; t < matrix_width; t++) {
                    new_timeline[p * nova_width + t] = timeline[p * matrix_width + t];
                }
            }
            free(timeline);
            timeline = new_timeline;
            matrix_width = nova_width;
            *out_matrix_width = matrix_width;
        }

        // 🔹 Marca execução contínua
        pid = pronto[index_maior]->id;
        for (int i = tempo; i < tempo_proc; ++i) {
            pronto[index_maior]->remaining_time--;
            timeline[pid * matrix_width + i] = 2;
        }

        // 🔹 Atualiza estado do processo
        tempo = tempo_proc;
        tempo_ant = tempo;
        pronto[index_maior]->completion_time = tempo;
        pronto[index_maior]->finished = 1;

        // Remove da lista de prontos
        if (pronto_tras >= 0) {
            pronto[index_maior] = pronto[pronto_tras--];
        } else {
            pronto_tras = -1;
        }

        proc_concluidos++;
    }

    // 🔹 Calcula turnaround e waiting time
    for (int i = 0; i < qtd_task; i++) {
        novo[i].turnaround_time = novo[i].completion_time - novo[i].arrival_time;
        novo[i].waiting_time = novo[i].turnaround_time - novo[i].burst_time;
    }

    *out_max_time = tempo;

    // 🔹 Reduz timeline para tamanho exato
    if (tempo < matrix_width) {
        int* new_timeline = calloc(qtd_task * tempo, sizeof(int));
        for (int p = 0; p < qtd_task; p++) {
            for (int t = 0; t < tempo; t++) {
                new_timeline[p * tempo + t] = timeline[p * matrix_width + t];
            }
        }
        free(timeline);
        timeline = new_timeline;
        *out_matrix_width = tempo;
    }

    return timeline;
}


// Prioridade preemptivo
int* priop(Processo* novo, int qtd_task, int* out_matrix_width, int* out_max_time) {
    int tempo = 0,
    proc_concluidos = 0,
    pid,
    pronto_tras = -1,
    index_maior, maior_prioridade;
    Processo *pronto[qtd_task];

    for (int i = 0; i < qtd_task; ++i) {
        novo[i].remaining_time = novo[i].burst_time;
        novo[i].finished = 0;
    }
    
    // Usa estimativa para largura da matriz
    int matrix_width = get_matrix_width_estimate(novo, qtd_task);
    *out_matrix_width = matrix_width;
    int* timeline = calloc(qtd_task * matrix_width, sizeof(int));


    while (proc_concluidos < qtd_task) {
        // Adiciona os processos do tempo atual no pronto
        for (int i = 0; i < qtd_task; ++i) {
            if (novo[i].finished != 1 && novo[i].arrival_time == tempo)
                pronto[++pronto_tras] = &novo[i];
        }

        // Checa se precisa realocar
        if (tempo >= matrix_width) {
            int nova_width = matrix_width * 2;
            int* new_timeline = calloc(qtd_task * nova_width, sizeof(int));
            for(int p = 0; p < qtd_task; p++) {
                for(int t = 0; t < matrix_width; t++) {
                    new_timeline[p * nova_width + t] = timeline[p * matrix_width + t];
                }
            }
            free(timeline);
            timeline = new_timeline;
            matrix_width = nova_width;
            *out_matrix_width = matrix_width;
        }

        // Marcando processos prontos, mas não executados
        for (int i = 0; i <= pronto_tras; i++) {
            int pid = pronto[i]->id;
            timeline[pid * matrix_width + tempo] = 1;
        }

        // Seleciona processo com maior prioridade
        index_maior = -1, maior_prioridade = -1;
        for (int i = 0; i <= pronto_tras; ++i) {
            if (pronto[i]->prioridade > maior_prioridade) {
                maior_prioridade = pronto[i]->prioridade;
                index_maior = i;
            }
        }

        // Nenhum processo pronto, CPU ociosa
        if (index_maior == -1) {
            tempo++;
            continue;
        }

        // Executa uma unidade de tempo
        pronto[index_maior]->remaining_time--;
        pid = pronto[index_maior]->id;
        timeline[pid * matrix_width + tempo] = 2;
        tempo++;

        // Se processo terminou
        if (pronto[index_maior]->remaining_time == 0) {
            pronto[index_maior]->completion_time = tempo;
            pronto[index_maior]->finished = 1;
            pronto[index_maior] = pronto[pronto_tras--];
            proc_concluidos++;
        }
    }

    // Calcula turnaround e waiting time
    for (int i = 0; i < qtd_task; i++) {
        novo[i].turnaround_time = novo[i].completion_time - novo[i].arrival_time;
        novo[i].waiting_time = novo[i].turnaround_time - novo[i].burst_time;
    }

    *out_max_time = tempo;
    // realocar para tamanho exato
    if (tempo < matrix_width) {
        int* new_timeline = calloc(qtd_task * tempo, sizeof(int));
         for(int p = 0; p < qtd_task; p++) {
            for(int t = 0; t < tempo; t++) {
                new_timeline[p * tempo + t] = timeline[p * matrix_width + t];
            }
        }
        free(timeline);
        timeline = new_timeline;
        *out_matrix_width = tempo;
    }

    return timeline;
}

/*
 * Algoritmo de Round-Robin clássico (sem prioridade).
 * Recebe vetor de Processos, quantidade de processos e quantum.
 * O vetor de Processos deve ter remaining_time = burst_time.
 * Retorna vetor com tempos de conclusão de cada processo.
 * Também coleta a timeline (histórico das trocas de contexto).
 */
// A struct Evento agora está em escalonadores.h

// Definição das variáveis globais da timeline
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

    // Zera a timeline global (caso seja uma segunda execução)
    if(timeline_rr) free(timeline_rr);
    timeline_rr = NULL;
    timeline_rr_sz = 0; 
    timeline_rr_cap = 0;

    // Enfileira processos com chegada zero
    for (int i = 0; i < qtd_proc; i++) {
        processos[i].finished = 0; // Garante que finished está zerado
        processos[i].remaining_time = processos[i].burst_time; // Garante remaining_time
        if (processos[i].arrival_time == 0) {
            fila[tras++] = i;
            em_fila[i] = 1;
        }
    }

    while (processos_finalizados < qtd_proc) {
        if (frente == tras) {
            // Avança tempo até alguém chegar
            int prox_chegada = INT_MAX;
            for (int i = 0; i < qtd_proc; i++)
                if (!processos[i].finished && !em_fila[i] && processos[i].arrival_time > tempo_atual)
                    if (processos[i].arrival_time < prox_chegada)
                        prox_chegada = processos[i].arrival_time;
            
            if (prox_chegada == INT_MAX) {
                // Não há mais processos para chegar e a fila está vazia
                // Isso não deveria acontecer se processos_finalizados < qtd_proc
                break; 
            }

            tempo_atual = prox_chegada;
            
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
        int tempo_fim_exec = tempo_atual + tempo_exec;

        // Entrada de processos durante esse quantum
        for (int i = 0; i < qtd_proc; i++)
            if (!em_fila[i] && !processos[i].finished && 
                processos[i].arrival_time > tempo_atual && // Chegou *depois* do início
                processos[i].arrival_time <= tempo_fim_exec) { // Chegou *antes* do fim
                fila[tras++] = i;
                em_fila[i] = 1;
            }
        
        tempo_atual = tempo_fim_exec;
        p->remaining_time -= tempo_exec;


        if (p->remaining_time == 0) {
            p->completion_time = tempo_atual;
            p->turnaround_time = p->completion_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            p->finished = 1;
            processos_finalizados++;
            completion_times[idx] = p->completion_time;
        } else {
            // Reinfileira o processo
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

// Definição das variáveis globais da timeline
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

    // Zera a timeline global (caso seja uma segunda execução)
    if(timeline_rrd) free(timeline_rrd);
    timeline_rrd = NULL;
    timeline_rrd_sz = 0; 
    timeline_rrd_cap = 0;
    int ultimo_processo = -1;

    int* prioridade_dinamica = malloc(qtd_proc * sizeof(int));
    for (int i = 0; i < qtd_proc; i++) {
        prioridade_dinamica[i] = processos[i].prioridade;
        processos[i].finished = 0; // Garante que finished está zerado
        processos[i].remaining_time = processos[i].burst_time; // Garante remaining_time
    }


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
            int prox_chegada = INT_MAX;
            for (int i = 0; i < qtd_proc; i++)
                if (!processos[i].finished && !em_fila[i] && processos[i].arrival_time > time)
                     if (processos[i].arrival_time < prox_chegada)
                        prox_chegada = processos[i].arrival_time;
            
            if (prox_chegada == INT_MAX) break;

            time = prox_chegada;
            
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
        int tempo_fim_exec = time + exec_time;

        // Entrada de processos durante quantum
        for (int i = 0; i < qtd_proc; i++)
            if (!em_fila[i] && !processos[i].finished && 
                processos[i].arrival_time > time &&
                processos[i].arrival_time <= tempo_fim_exec) {
                fila[fila_sz++] = (FilaElem){.idx = i, .prioridade = prioridade_dinamica[i], .chegada = processos[i].arrival_time};
                em_fila[i] = 1;
            }
        
        time = tempo_fim_exec;
        p->remaining_time -= exec_time;


        // Aging dos demais processos na fila (exceto o que acabou de rodar)
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
            // Atualiza prioridade do processo que rodou e o joga pro fim
            // (para ser re-ordenado no próximo qsort)
            fila[0].prioridade = prioridade_dinamica[idx]; 
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
