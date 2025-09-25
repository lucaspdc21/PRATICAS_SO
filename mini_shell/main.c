#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CMD_LEN 256
#define MAX_ARGS 32

// Array para armazenar PIDs de processos em background
pid_t bg_processes[10];
int bg_count = 0;
pid_t last_child_pid = 0; // Armazena PID do último processo filho

void parse_command(char *input, char **args, int *background) {
    // Implementar parsing do comando
    // Dividir a string em argumentos
    // Verificar se termina com &

    *background = 0;

    // Inicializa todos os argumentos como NULL
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = NULL;
    }
    
    int argc = 0;
    char *token = strtok(input, " ");

    // Percorrendo os tokens 
    while (token != NULL && argc < MAX_ARGS - 1) {
        // Ignora tokens vazios (caso de múltiplos espaços)
        if (strlen(token) > 0) {
            args[argc++] = token;
        }
        token = strtok(NULL, " ");
    }

    // Verifica se o último argumento é "&"
    if (argc > 0 && strcmp(args[argc - 1], "&") == 0) {
        *background = 1;
        args[argc - 1] = NULL; // Remove o "&" dos argumentos
    }
}


void add_background_process(int pid) {
    bg_count < 10 ? (bg_processes[bg_count++] = pid) : (fprintf(stderr, "Limite de processos em background atingido\n"));
}

void execute_command(char **args, int background) {
    // Implementar execução
    // Usar fork() e execvp()
    // Gerenciar background se necessário

    // Realizando o fork
    int retval;
    retval = fork ();

    if (retval < 0) {
        // Erro no fork
        perror("Fork falhou");
        exit(1);
    }

    else {
        if (retval > 0) { // Processo pai
            last_child_pid = retval;

            if (background) {
                add_background_process(retval); // Adiciona na lista de bg
                printf("Processo background (PID: %d)\n", retval);
            } else {
                waitpid(retval, NULL, 0);
            }

        } else { // Processo filho (retval = 0)
            // Executa comando externo
            if (execvp(args[0], args) == -1) {
                perror("Erro no execvp");
                exit(1); // garante que o filho não continue o shell
            }
        }
    //exit(0);
    }
}

int is_internal_command(char **args) {
    // Verificar se é comando interno
    // exit, pid, jobs, wait

    int is_internal_command(char **args) {

        if (args[0] == NULL) return 0;

        // Comando interno exit
        if (strcmp(args[0], "exit") == 0) {
            return 1;
        }

        // Comando interno pid
        if (strcmp(args[0], "pid") == 0) {
            return 1;
        }

        return 0; // não é comando interno
    }
}

void handle_internal_command(char **args) {
    // TODO: Executar comandos internos

    if (strcmp(args[0], "exit") == 0) {
        printf("Saindo do shell...\n");
        exit(0);  // encerra o shell
    }

    else if (strcmp(args[0], "pid") == 0) {
        printf("PID do shell: %d\n", getpid());
        if (last_child_pid != 0) {
            printf("PID do último processo filho: %d\n", last_child_pid);
        } else {
            printf("Nenhum processo filho executado ainda.\n");
        }
    }
}


int main() {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int background;

    printf("Mini-Shell iniciado (PID: %d)\n", getpid());
    printf("Digite 'exit' para sair\n\n");

    while (1) {
        printf("minishell> ");
        fflush(stdout);

        // Ler entrada do usuário
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        // Remover quebra de linha
        input[strcspn(input, "\n")] = 0;

        // Ignorar linhas vazias
        if (strlen(input) == 0) {
            continue;
        }

        // Fazer parsing do comando
        parse_command(input, args, &background);

        // Executar comando
        if (is_internal_command(args)) {
            handle_internal_command(args);
        } else {
            execute_command(args, background);
        }
    }

    printf("Shell encerrado!\n");
    return 0;
}