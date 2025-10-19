// First Come, First Served
int** fcfs(int** tasks, int qtd_task);

// Shortest Job First
int** sjf(int** tasks, int qtd_task);

// Shortest Remaining Time First
int** srtf(int** tasks, int qtd_task);

// Prioridade cooperativo
int** prioc(int** tasks, int qtd_taskd);

// Prioridade preemptivo
int** priop(int** tasks, int qtd_taskd);

// Round-Robin clássico (sem prioridade)
int** rr(int** tasks, int qtd_taskd, int quantum);

// Round-Robin com prioridade e aging
int** rr_d(int** tasks, int qtd_taskd, int quantum, int alfa);
