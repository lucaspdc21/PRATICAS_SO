#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h> 
#include <fcntl.h>   
#include <unistd.h>   

#include <microhttpd.h>
#include "../libs/cJSON.h" 
#include "../include/escalonadores.h"

#define PORT 8888
#define STATIC_DOC_ROOT "public"

static int find_proc_index_by_id(Processo* processos, int qtd_proc, int id) {
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].id == id) {
            return i;
        }
    }
    return -1;
}

static int get_max_completion_time(Processo* processos, int qtd_proc) {
    int max_time = 0;
    for (int i = 0; i < qtd_proc; i++) {
        if (processos[i].completion_time > max_time) {
            max_time = processos[i].completion_time;
        }
    }
    return max_time;
}

static int build_timeline_simple(cJSON* out_timeline, int* timeline_data, int max_time, Processo* processos, int qtd_proc) {
    int context_switches = 0;
    int last_pid = -2; 
    for (int t = 0; t < max_time; t++) {
        int current_pid = timeline_data[t];
        if (current_pid != last_pid && current_pid != -1) {
            context_switches++;
        }
        cJSON* entry = cJSON_CreateObject();
        char time_str[32];
        sprintf(time_str, "%d-%d", t, t + 1);
        cJSON_AddStringToObject(entry, "time", time_str);
        if (current_pid == -1) {
            cJSON_AddStringToObject(entry, "exec", "Idle");
        } else {
            int proc_idx = find_proc_index_by_id(processos, qtd_proc, current_pid);
            cJSON_AddStringToObject(entry, "exec", (proc_idx != -1) ? processos[proc_idx].original_id_str : "??");
        }
        cJSON_AddItemToArray(out_timeline, entry);
        last_pid = current_pid;
    }
    return context_switches;
}

static int build_timeline_matrix(cJSON* out_timeline, int* timeline_data, int max_time, int matrix_width, Processo* processos, int qtd_proc) {
    int context_switches = 0;
    int last_pid = -2;
    for (int t = 0; t < max_time; t++) {
        int current_pid = -1; 
        for (int p_idx = 0; p_idx < qtd_proc; p_idx++) {
            if (timeline_data[p_idx * matrix_width + t] == 2) {
                current_pid = processos[p_idx].id;
                break;
            }
        }
        if (current_pid != last_pid && current_pid != -1) {
            context_switches++;
        }
        cJSON* entry = cJSON_CreateObject();
        char time_str[32];
        sprintf(time_str, "%d-%d", t, t + 1);
        cJSON_AddStringToObject(entry, "time", time_str);
        if (current_pid == -1) {
            cJSON_AddStringToObject(entry, "exec", "Idle");
        } else {
             int proc_idx = find_proc_index_by_id(processos, qtd_proc, current_pid);
            cJSON_AddStringToObject(entry, "exec", (proc_idx != -1) ? processos[proc_idx].original_id_str : "??");
        }
        cJSON_AddItemToArray(out_timeline, entry);
        last_pid = current_pid;
    }
    return context_switches;
}

static int build_timeline_event(cJSON* out_timeline, Evento* events, int event_count, int max_time, Processo* processos, int qtd_proc) {
    int context_switches = event_count; 
    for (int t = 0; t < max_time; t++) {
        int current_pid = -1; 
        int running_event_idx = -1;
        for (int i = 0; i < event_count; i++) {
            if (events[i].tempo_inicio <= t) {
                running_event_idx = i;
            } else {
                break;
            }
        }
        if (running_event_idx != -1) {
            int pid = events[running_event_idx].pid;
            int proc_idx = find_proc_index_by_id(processos, qtd_proc, pid);
            if (proc_idx != -1 && t < processos[proc_idx].completion_time) {
                current_pid = pid;
            }
        }
        cJSON* entry = cJSON_CreateObject();
        char time_str[32];
        sprintf(time_str, "%d-%d", t, t + 1);
        cJSON_AddStringToObject(entry, "time", time_str);
        if (current_pid == -1) {
            cJSON_AddStringToObject(entry, "exec", "Idle");
        } else {
            int proc_idx = find_proc_index_by_id(processos, qtd_proc, current_pid);
            cJSON_AddStringToObject(entry, "exec", (proc_idx != -1) ? processos[proc_idx].original_id_str : "??");
        }
        cJSON_AddItemToArray(out_timeline, entry);
    }
    return context_switches;
}


char* run_simulation_from_json(const char* input_buffer) {
    cJSON* root = cJSON_Parse(input_buffer);
    if (!root) {
        return strdup("{\"error\":\"Erro ao parsear JSON\"}");
    }

    cJSON* j_algo = cJSON_GetObjectItemCaseSensitive(root, "algorithm");
    cJSON* j_config = cJSON_GetObjectItemCaseSensitive(root, "config");
    cJSON* j_processes = cJSON_GetObjectItemCaseSensitive(root, "processes");

    if (!j_algo || !j_processes || !cJSON_IsString(j_algo) || !cJSON_IsArray(j_processes)) {
        cJSON_Delete(root);
        return strdup("{\"error\":\"JSON de entrada inválido\"}");
    }

    int qtd_task = cJSON_GetArraySize(j_processes);
    if (qtd_task == 0) {
        cJSON_Delete(root);
        return strdup("{\"error\":\"Nenhum processo enviado\"}");
    }

    Processo* processos = malloc(qtd_task * sizeof(Processo));
    if(!processos) {
        cJSON_Delete(root);
        return strdup("{\"error\":\"Falha ao alocar memória\"}");
    }

    cJSON* j_proc = NULL;
    int i = 0;
    cJSON_ArrayForEach(j_proc, j_processes) {
        cJSON* j_id = cJSON_GetObjectItem(j_proc, "id");
        cJSON* j_arrival = cJSON_GetObjectItem(j_proc, "arrival");
        cJSON* j_duration = cJSON_GetObjectItem(j_proc, "duration");
        cJSON* j_priority = cJSON_GetObjectItem(j_proc, "priority");

        processos[i].id = i; 
        processos[i].arrival_time = j_arrival ? j_arrival->valueint : 0;
        processos[i].burst_time = j_duration ? j_duration->valueint : 0;
        processos[i].remaining_time = processos[i].burst_time;
        processos[i].prioridade = j_priority ? j_priority->valueint : 0;
        processos[i].finished = 0;
        processos[i].original_id_str = j_id ? strdup(j_id->valuestring) : strdup("?");
        i++;
    }

    cJSON* out_root = cJSON_CreateObject();
    cJSON* out_results = cJSON_CreateObject();
    cJSON* out_timeline = cJSON_CreateArray();
    cJSON* out_processes = cJSON_CreateArray();

    cJSON_AddStringToObject(out_root, "algorithm", j_algo->valuestring);

    char* algo_name = j_algo->valuestring;
    int context_switches = 0;
    int max_time = 0;

    if (strcmp(algo_name, "FCFS") == 0) {
        int* timeline_data = fcfs(processos, qtd_task, &max_time);
        context_switches = build_timeline_simple(out_timeline, timeline_data, max_time, processos, qtd_task);
        free(timeline_data);
    } 
    else if (strcmp(algo_name, "SJF") == 0) {
        int* timeline_data = sjf(processos, qtd_task, &max_time);
        context_switches = build_timeline_simple(out_timeline, timeline_data, max_time, processos, qtd_task);
        free(timeline_data);
    } 
    else if (strcmp(algo_name, "SRTF") == 0) {
        int matrix_width = 0;
        int* timeline_data = srtf(processos, qtd_task, &matrix_width, &max_time);
        context_switches = build_timeline_matrix(out_timeline, timeline_data, max_time, matrix_width, processos, qtd_task);
        free(timeline_data);
    } 
    else if (strcmp(algo_name, "PRIOC") == 0) {
        int matrix_width = 0;
        int* timeline_data = prioc(processos, qtd_task, &matrix_width, &max_time);
        context_switches = build_timeline_matrix(out_timeline, timeline_data, max_time, matrix_width, processos, qtd_task);
        free(timeline_data);
    } 
    else if (strcmp(algo_name, "PRIOP") == 0) {
        int matrix_width = 0;
        int* timeline_data = priop(processos, qtd_task, &matrix_width, &max_time);
        context_switches = build_timeline_matrix(out_timeline, timeline_data, max_time, matrix_width, processos, qtd_task);
        free(timeline_data);
    } 
    else if (strcmp(algo_name, "Round Robin") == 0) {
        int quantum = 1;
        int aging = 0;
        if(j_config) {
            cJSON* j_quantum = cJSON_GetObjectItem(j_config, "quantum");
            if(j_quantum) quantum = j_quantum->valueint;
            cJSON* j_aging = cJSON_GetObjectItem(j_config, "aging");
            if(j_aging) aging = j_aging->valueint;
        }
        
        cJSON_AddNumberToObject(out_root, "quantum", quantum);
        cJSON_AddNumberToObject(out_root, "aging", aging); // Adiciona aging ao resultado
        int* completion_times = NULL;

        if (aging > 0) {
            completion_times = rr_d(processos, qtd_task, quantum, aging);
            max_time = get_max_completion_time(processos, qtd_task);
            context_switches = build_timeline_event(out_timeline, timeline_rrd, timeline_rrd_sz, max_time, processos, qtd_task);
            if(timeline_rrd) free(timeline_rrd);
            timeline_rrd = NULL; timeline_rrd_sz = 0; timeline_rrd_cap = 0;
        } else {
            completion_times = rr(processos, qtd_task, quantum);
            max_time = get_max_completion_time(processos, qtd_task);
            context_switches = build_timeline_event(out_timeline, timeline_rr, timeline_rr_sz, max_time, processos, qtd_task);
            if(timeline_rr) free(timeline_rr);
            timeline_rr = NULL; timeline_rr_sz = 0; timeline_rr_cap = 0;
        }
        free(completion_times);
    } 
    else {
        cJSON_AddStringToObject(out_root, "error", "Algoritmo desconhecido");
    }

    double total_turnaround = 0;
    double total_waiting = 0;

    for (int k = 0; k < qtd_task; k++) {
        total_turnaround += processos[k].turnaround_time;
        total_waiting += processos[k].waiting_time;

        cJSON* out_proc = cJSON_CreateObject();
        cJSON_AddStringToObject(out_proc, "id", processos[k].original_id_str);
        cJSON_AddNumberToObject(out_proc, "turnaround", processos[k].turnaround_time);
        cJSON_AddNumberToObject(out_proc, "waiting", processos[k].waiting_time);
        cJSON_AddItemToArray(out_processes, out_proc);
    }

    if (qtd_task > 0) {
        cJSON_AddNumberToObject(out_results, "turnaround_avg", total_turnaround / qtd_task);
        cJSON_AddNumberToObject(out_results, "waiting_avg", total_waiting / qtd_task);
    } else {
        cJSON_AddNumberToObject(out_results, "turnaround_avg", 0);
        cJSON_AddNumberToObject(out_results, "waiting_avg", 0);
    }
    cJSON_AddNumberToObject(out_results, "context_switches", context_switches);

    cJSON_AddItemToObject(out_root, "results", out_results);
    cJSON_AddItemToObject(out_root, "timeline", out_timeline);
    cJSON_AddItemToObject(out_root, "processes", out_processes);

    char* out_string = cJSON_Print(out_root);
    
    // Limpeza
    cJSON_Delete(root);
    cJSON_Delete(out_root);
    for(int k=0; k < qtd_task; k++) {
        free(processos[k].original_id_str);
    }
    free(processos);

    return out_string;
}


// Estrutura para guardar o corpo da requisição POST
struct connection_info_struct {
  char *data;
  size_t data_size;
};

static enum MHD_Result serve_static_file(struct MHD_Connection *connection, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", STATIC_DOC_ROOT, filename);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        return MHD_NO; // Arquivo não encontrado
    }

    struct stat sbuf;
    if (fstat(fd, &sbuf) == -1) {
        close(fd);
        return MHD_NO;
    }

    struct MHD_Response *response = MHD_create_response_from_fd(sbuf.st_size, fd);
    if (!response) {
        close(fd);
        return MHD_NO;
    }

    // Determina o Content-Type
    const char *content_type = "text/plain";
    if (strstr(filename, ".html")) content_type = "text/html; charset=utf-8";
    else if (strstr(filename, ".css")) content_type = "text/css";
    else if (strstr(filename, ".js")) content_type = "application/javascript";

    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, content_type);
    
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

// Iterador para processar dados POST
static enum MHD_Result
iterate_post_data (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
               const char *filename, const char *content_type,
               const char *transfer_encoding, const char *data, uint64_t off,
               size_t size)
{
  struct connection_info_struct *con_info = coninfo_cls;

  // Estamos interessados apenas nos dados brutos, não em chaves/valores
  if (size > 0) {
    con_info->data = realloc(con_info->data, con_info->data_size + size + 1);
    if (!con_info->data)
      return MHD_NO;
    memcpy(&con_info->data[con_info->data_size], data, size);
    con_info->data_size += size;
    con_info->data[con_info->data_size] = '\0';
  }
  return MHD_YES;
}

// Callback de limpeza da conexão
static void
request_completed (void *cls, struct MHD_Connection *connection,
                   void **con_cls, enum MHD_RequestTerminationCode toe)
{
  struct connection_info_struct *con_info = *con_cls;
  if (NULL == con_info)
    return;
  if (con_info->data)
    free (con_info->data);
  free (con_info);
  *con_cls = NULL;
}


// Manipulador principal de conexões
static enum MHD_Result answer_to_connection(void *cls, struct MHD_Connection *connection,
                                            const char *url, const char *method,
                                            const char *version, const char *upload_data,
                                            size_t *upload_data_size, void **con_cls) {
    if (0 == strcmp(method, MHD_HTTP_METHOD_POST) && 0 == strcmp(url, "/simulate")) {
        struct connection_info_struct *con_info = *con_cls;

        if (NULL == con_info) {
            // Nova conexão POST
            con_info = calloc(1, sizeof(struct connection_info_struct));
            if (NULL == con_info) return MHD_NO;
            *con_cls = (void *) con_info;
            return MHD_YES;
        }

        if (*upload_data_size != 0) {
            // Processa o chunk de dados
            // (Para JSON, esperamos que venha de uma vez, mas tratamos como chunk)
             con_info->data = realloc(con_info->data, con_info->data_size + *upload_data_size + 1);
             if (!con_info->data) return MHD_NO;
             memcpy(&con_info->data[con_info->data_size], upload_data, *upload_data_size);
             con_info->data_size += *upload_data_size;
             con_info->data[con_info->data_size] = '\0';
             *upload_data_size = 0; // Consumimos os dados
             return MHD_YES;
        } else {
            // POST finalizado, processa a simulação
            char *response_json_str = run_simulation_from_json(con_info->data);
            
            struct MHD_Response *response = MHD_create_response_from_buffer(
                                                strlen(response_json_str),
                                                (void *)response_json_str,
                                                MHD_RESPMEM_MUST_FREE); // MHD libera a string
            
            MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "application/json");
            enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
            return ret;
        }
    }

    if (0 == strcmp(method, MHD_HTTP_METHOD_GET)) {
        if (0 == strcmp(url, "/")) {
            return serve_static_file(connection, "index.html");
        }
        if (0 == strcmp(url, "/style.css")) {
            return serve_static_file(connection, "style.css");
        }
        if (0 == strcmp(url, "/script.js")) {
            return serve_static_file(connection, "script.js");
        }
    }


    const char *page = "<html><body>Recurso não encontrado</body></html>";
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
    enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
}

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL,
                              MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Erro ao iniciar o servidor libmicrohttpd\n");
        return 1;
    }

    printf("Servidor rodando em http://localhost:%d\n", PORT);
    printf("Coloque seus arquivos (index.html, style.css, script.js) na pasta '%s'\n", STATIC_DOC_ROOT);
    printf("Pressione Enter para sair...\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
