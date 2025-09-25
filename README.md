# Checklist de Implementação do Mini-Shell

## Funcionalidades Obrigatórias

- [x] **parse_command()**
  - [x] Dividir a string de entrada em argumentos usando `strtok()`.
  - [x] Detectar `&` no final para indicar execução em background.

- [x] **execute_command()**
  - [x] Criar processo filho com `fork()`.
  - [x] No filho: executar comando externo com `execvp(args[0], args)`.
  - [x] No pai: usar `wait()` ou `waitpid()` se o comando **não** for em background.
  - [x] Suportar processos em background adicionando o PID em uma lista.

- [x] **is_internal_command()**
  - [x] Verificar se o comando digitado é interno:
    - [x] `exit`
    - [x] `pid`

- [x] **handle_internal_command()**
  - [x] `exit`: encerra o shell.
  - [x] `pid`: mostra o PID do shell e do último processo filho executado.
