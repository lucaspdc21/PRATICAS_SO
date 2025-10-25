document.addEventListener('DOMContentLoaded', () => {

    const simForm = document.getElementById('simForm');
    const addProcessForm = document.getElementById('addProcessForm');
    const processTbody = document.querySelector('#processInputTable tbody');
    const algorithmSelect = document.getElementById('algorithm');
    const quantumGroup = document.getElementById('quantum-group');
    const agingGroup = document.getElementById('aging-group');
    const resultsDiv = document.getElementById('simulationResults');
    const resultOutputPre = document.getElementById('resultOutput');

    addProcessForm.addEventListener('submit', function(event) {
        event.preventDefault();
        
        const id = document.getElementById('p_id').value;
        const arrival = parseInt(document.getElementById('p_arrival').value);
        const duration = parseInt(document.getElementById('p_duration').value);
        const priority = parseInt(document.getElementById('p_priority').value);

        if (!id || isNaN(arrival) || isNaN(duration) || isNaN(priority)) {
            alert('Por favor, preencha todos os campos corretamente.');
            return;
        }

        const row = processTbody.insertRow();
        row.innerHTML = `
            <td data-label="id">${id}</td>
            <td data-label="arrival">${arrival}</td>
            <td data-label="duration">${duration}</td>
            <td data-label="priority">${priority}</td>
            <td><button class="remove-proc">Remover</button></td>
        `;
        
        // Limpa o formulário
        addProcessForm.reset();
        document.getElementById('p_id').focus();
    });

    processTbody.addEventListener('click', function(event) {
        if (event.target.classList.contains('remove-proc')) {
            const row = event.target.closest('tr');
            processTbody.removeChild(row);
        }
    });

    function toggleRRFields() {
        const isRR = algorithmSelect.value === 'Round Robin';
        quantumGroup.style.display = isRR ? 'block' : 'none';
        agingGroup.style.display = isRR ? 'block' : 'none';
    }
    algorithmSelect.addEventListener('change', toggleRRFields);
    toggleRRFields(); // Executa na inicialização

    simForm.addEventListener('submit', async function(event) {
        event.preventDefault(); // Impede o envio tradicional do formulário
        resultsDiv.style.display = 'none';
        resultOutputPre.textContent = 'Simulando...';

        // 1. Coletar dados dos processos da tabela
        const processes = [];
        const rows = processTbody.querySelectorAll('tr');
        if (rows.length === 0) {
            alert('Adicione pelo menos um processo antes de simular.');
            return;
        }

        rows.forEach(row => {
            const cells = row.querySelectorAll('td');
            processes.push({
                id: cells[0].textContent,
                arrival: parseInt(cells[1].textContent),
                duration: parseInt(cells[2].textContent),
                priority: parseInt(cells[3].textContent)
            });
        });

        // 2. Coletar dados da configuração
        const algorithm = algorithmSelect.value;
        const quantum = parseInt(document.getElementById('quantum').value);
        const aging = parseInt(document.getElementById('aging').value);

        // 3. Montar o JSON da requisição
        const requestData = {
            algorithm: algorithm,
            config: {
                quantum: quantum,
                aging: aging
            },
            processes: processes
        };

        // 4. Enviar para o servidor C (libmicrohttpd)
        try {
            const response = await fetch('/simulate', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(requestData)
            });

            if (!response.ok) {
                throw new Error(`Erro do servidor: ${response.statusText}`);
            }

            const data = await response.json();

            // 5. Exibir os resultados
            if (data.error) {
                resultOutputPre.textContent = `Erro na simulação: ${data.error}`;
            } else {
                resultsDiv.style.display = 'block';
                resultOutputPre.textContent = JSON.stringify(data, null, 2);
                displaySimulationResults(data); 
            }

        } catch (error) {
            console.error('Erro ao enviar simulação:', error);
            resultOutputPre.textContent = `Erro de conexão: ${error.message}`;
        }
    });
});

function displaySimulationResults(data) {
    const generalResultsDiv = document.getElementById('generalResults');
    const processesTableBody = document.querySelector('#processesTable tbody');
    const resultOutputPre = document.getElementById('resultOutput');

    resultOutputPre.textContent = JSON.stringify(data, null, 2);

    generalResultsDiv.innerHTML = `
        <p><strong>Algoritmo:</strong> ${data.algorithm}</p>
        <p><strong>Quantum:</strong> ${data.quantum > 0 ? data.quantum : 'N/A'}</p>
        <p><strong>Aging:</strong> ${data.aging > 0 ? data.aging : 'N/A'}</p>
        <p><strong>Turnaround Médio:</strong> ${data.results.turnaround_avg.toFixed(2)}</p>
        <p><strong>Tempo de Espera Médio:</strong> ${data.results.waiting_avg.toFixed(2)}</p>
        <p><strong>Trocas de Contexto:</strong> ${data.results.context_switches}</p>
    `;

    drawGanttChart(data.timeline, data.processes);

    processesTableBody.innerHTML = ''; 
    data.processes.forEach(process => {
        const row = processesTableBody.insertRow();
        row.insertCell().textContent = process.id;
        row.insertCell().textContent = process.turnaround;
        row.insertCell().textContent = process.waiting;
    });
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function mergeTimelineBlocks(timeline) {
    if (!timeline || timeline.length === 0) return [];

    const merged = [];
    let currentBlock = null;

    timeline.forEach(item => {
        const [start] = item.time.split('-').map(Number);
        
        if (!currentBlock || item.exec !== currentBlock.id) {
            if (currentBlock) {
                merged.push(currentBlock);
            }
            currentBlock = {
                id: item.exec,
                start: start,
                end: start + 1
            };
        } else {
            currentBlock.end = start + 1;
        }
    });
    if (currentBlock) {
        merged.push(currentBlock);
    }
    return merged;
}

async function drawGanttChart(timeline, processesData) {
    const container = document.getElementById('ganttChart');
    const tooltip = document.getElementById('ganttTooltip');
    container.innerHTML = '';

    if (!timeline || timeline.length === 0) {
        container.textContent = "Nenhum dado na timeline para exibir.";
        return;
    }

    const mergedBlocks = mergeTimelineBlocks(timeline);
    const processIDs = [...new Set(processesData.map(p => p.id))].sort();
    
    // Inclui "Idle" se existir
    if (mergedBlocks.some(b => b.id === 'Idle') && !processIDs.includes('Idle')) {
        processIDs.push('Idle');
    }

    const maxTime = Math.max(...mergedBlocks.map(b => b.end));

    const colors = ["#2196F3", "#FF9800", "#9C27B0", "#E91E63", "#4CAF50", "#f44336", "#00BCD4", "#795548"];
    const processColorMap = new Map();
    processIDs.forEach((id, index) => {
        if (id === 'Idle') {
            processColorMap.set('Idle', '#BDBDBD'); 
        } else {
            processColorMap.set(id, colors[index % colors.length]);
        }
    });

    container.style.gridTemplateColumns = `auto repeat(${maxTime}, minmax(40px, 1fr))`;
    container.style.gridTemplateRows = `auto repeat(${processIDs.length}, 35px)`;
    
    const corner = document.createElement('div');
    corner.className = 'gantt-corner';
    container.appendChild(corner);

    for (let i = 0; i < maxTime; i++) {
        const timeHeader = document.createElement('div');
        timeHeader.className = 'gantt-header-time';
        timeHeader.textContent = i;
        timeHeader.style.gridColumn = i + 2;
        container.appendChild(timeHeader);
    }

    processIDs.forEach((pid, index) => {
        const processLabel = document.createElement('div');
        processLabel.className = 'gantt-process-label';
        processLabel.textContent = pid;
        processLabel.style.gridRow = index + 2;
        container.appendChild(processLabel);
    });

    for (const block of mergedBlocks) {
        const processIndex = processIDs.indexOf(block.id);
        if (processIndex === -1) continue; 

        const blockDiv = document.createElement('div');
        blockDiv.className = 'gantt-block';
        if (block.id === 'Idle') {
            blockDiv.classList.add('gantt-block-idle');
        }
        
        blockDiv.textContent = block.id === 'Idle' ? '' : block.id;
        blockDiv.style.gridRow = processIndex + 2;
        blockDiv.style.gridColumn = `${block.start + 2} / ${block.end + 2}`;
        blockDiv.style.backgroundColor = processColorMap.get(block.id);
        
        blockDiv.addEventListener('mousemove', (e) => {
            tooltip.style.display = 'block';
            tooltip.style.left = `${e.pageX + 10}px`;
            tooltip.style.top = `${e.pageY + 10}px`;
            tooltip.innerHTML = `
                <strong>Processo:</strong> ${block.id}<br>
                <strong>Início:</strong> ${block.start}<br>
                <strong>Fim:</strong> ${block.end}<br>
                <strong>Duração:</strong> ${block.end - block.start}
            `;
        });
        blockDiv.addEventListener('mouseout', () => {
            tooltip.style.display = 'none';
        });

        container.appendChild(blockDiv);
        
        window.getComputedStyle(blockDiv).opacity; 

        blockDiv.style.opacity = '1';
        blockDiv.style.transform = 'scaleY(1)';
        
        await sleep(50); 
    }
}