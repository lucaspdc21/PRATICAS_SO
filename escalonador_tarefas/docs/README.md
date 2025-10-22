# Simulador de Escalonamento de Processos

Este projeto consiste em um simulador de escalonamento de processos, escrito em C. O objetivo é implementar e comparar diferentes algoritmos de escalonamento, fornecendo métricas de desempenho para cada um.

A entrada e a saída dos dados são feitas exclusivamente em arquivos no formato JSON para facilitar o intercâmbio de informações estruturadas e a análise dos resultados. Para manipulação de JSON em C, utilizamos a biblioteca [cJSON](https://github.com/DaveGamble/cJSON).

## Estrutura do projeto
```mermaid
.
├── cJSON.c
├── cJSON.h
├── escalonadores.c
├── escalonadores.h
├── main.c
└── Makefile
```
---

## Checklist do Projeto

### Obrigatórios
- [ ] Suporte aos seguintes algoritmos de escalonamento:
    - [ ] FCFS (First Come, First Served)
    - [ ] Shortest Job First 
    - [ ] Shortest Remaining Time First 
    - [ ] Por prioridade, sem preempção 
    - [ ] Por prioridade, com preempção por prioridade 
    - [x] Round-Robin com quantum, sem prioridade 
    - [x] Round-Robin com prioridade e envelhecimento 
- [ ] Leitura de processos a partir de um arquivo JSON (instante de criação, duração, prioridade)
- [ ] Saída dos resultados em arquivo JSON contendo:
    - [ ] Tempo médio de vida (turnaround time, tt)
    - [ ] Tempo médio de espera (waiting time, tw)
    - [ ] Número de trocas de contexto 
    - [ ] Diagrama de tempo da execução
- [ ] Configuração por arquivo texto em formato:
    ```quantum:2 aging:1```
- [ ] Utilização e correta integração da biblioteca cJSON para leitura e escrita de JSON 
- [ ]Código devidamente comentado 
- [ ] Documento explicativo das decisões de implementação (estruturas usadas, tratamento de desempates, etc.)

### Opcionais/Bônus
- [ ] Interface visual para acompanhamento ou animação da simulação (+2.5 pontos bônus)
- [ ] Descrever, no documento, eventuais estruturas de dados ou padrões de projeto utilizados 
- [ ] Documentar exemplos de entrada/saída JSON 
- [ ] Explicação detalhada de cada algoritmo implementado 

---

## Observações importantes
- Em caso de empate na escolha do processo a ser executado, adotar a seguinte prioridade:
    1. Processo já em execução (evita troca de contexto)
    2. Processo com menor tempo restante de processamento 
    3. Escolha aleatória, se persistir o empate
- Para Round-Robin com prioridade e envelhecimento, o envelhecimento ocorre a cada quantum e não há preempção por prioridade.