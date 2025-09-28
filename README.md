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

---

## Funcionalidades Opcionais

- [x] **Detectar comandos em background**
  - [x] Identificar o símbolo `&` no final da linha de comando.
  - [x] Ajustar `args[]` para remover o `&` antes de executar o comando.

- [x] **Armazenar PIDs de processos em background**
  - [x] Adicionar PID do processo filho à lista `bg_processes[]`.
  - [x] Manter contador `bg_count` para controlar o número de processos em background.

- [ ] **Comando jobs**
  - [ ] Listar todos os processos em background ativos.
  - [ ] Mostrar número do job, PID e status (Running).

- [ ] **Comando wait**
  - [ ] Aguardar o término de todos os processos em background.
  - [ ] Bloquear o shell até que todos os processos filhos terminem.
  - [ ] Exibir mensagens quando processos terminam: `[1]+ Done`.

- [ ] **Limpeza automática de processos terminados**
  - [ ] Remover PIDs de processos já finalizados da lista `bg_processes[]`.
  - [ ] Atualizar `bg_count` de forma adequada.
