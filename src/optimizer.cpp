/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   optimizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 01:08:45 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/06 16:35:20 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/optimizer.hpp"

GraspOptimizer::GraspOptimizer(const std::vector<Process>& procs, 
                               const std::map<std::string, int>& stocks,
                               int max_t)
    : processes(procs), initial_stocks(stocks), max_time(max_t)
{
    std::srand(std::time(nullptr));
    best_solution.makespan = __INT_MAX__;
}

bool GraspOptimizer::hasResourcesFor(const Process& proc, 
                                     const std::map<std::string, int>& stocks) const
{
    // TODO: Recorrer proc.requisites
	for (auto& p : proc.requisites)
	{
		for (auto& s : stocks)
		{
			if (s.first == p.first)
				if (s.second < p.second)
					return false;
		}
	}
	return true;
    // TODO: Para cada recurso, verificar que stocks[recurso] >= cantidad_necesaria
    // TODO: Si alguno no tiene suficiente, return false
    // TODO: Si todos están ok, return true
}

void GraspOptimizer::consumeResources(std::map<std::string, int>& stocks, 
                                      const Process& proc) const
{
    // TODO: Recorrer proc.requisites
	for (auto& p : proc.requisites)
		stocks[p.first] -= p.second;
    // TODO: Para cada recurso: stocks[recurso] -= cantidad
}

void GraspOptimizer::produceResources(std::map<std::string, int>& stocks, 
                                      const Process& proc) const
{
    // TODO: Recorrer proc.produces
	for (auto& p : proc.requisites)
	{
		if (stocks.count(p.first) == 0)
			stocks.insert(std::make_pair(p.first, 0));
		stocks[p.first] += p.second;
	}
    // TODO: Para cada recurso: stocks[recurso] += cantidad
    // TODO: Si el recurso no existe en stocks, crearlo
}

const Process* GraspOptimizer::findProcess(const std::string& name) const
{
    // TODO: Recorrer processes
	for (auto& p : this->processes)
	{
		if (p.name == name)
			return (&p);
	}
	return (nullptr);
    // TODO: Si encuentras uno con proc.name == name, return &proc
    // TODO: Si no lo encuentras, return nullptr
}

int GraspOptimizer::calculateMakespan(const Solution& solution) const
{
    // TODO: Si solution.schedule está vacío, return 0
    // TODO: Encontrar el finish_time máximo de todas las actividades
    // TODO: Return ese tiempo máximo
	if (solution.schedule.empty())
		return (0);
	int max = solution.schedule.begin()->finish_time;
	for (auto& s : solution.schedule)
	{
		if (s.finish_time > max)
			max = s.finish_time;
	}
	return (max);
}

bool GraspOptimizer::areDependenciesSatisfied(
    const Process& proc,
    const std::vector<bool>& scheduled,
    const std::map<std::string, int>& stocks) const
{
    // Para cada recurso que necesita este proceso
    for (const auto& [recurso_necesario, cantidad] : proc.requisites) {
        
        // ¿Hay stock inicial de este recurso?
        if (stocks.count(recurso_necesario) && stocks.at(recurso_necesario) > 0) {
            continue;  // OK, hay stock, no necesita dependencia
        }
        
        // No hay stock, así que ALGUIEN debe haberlo producido
        bool alguien_lo_produjo = false;
        
        // Buscar quién produce este recurso
        for (size_t i = 0; i < processes.size(); i++) {
            
            // ¿Este proceso produce el recurso que necesitamos?
            if (processes[i].produces.count(recurso_necesario)) {
                
                // ¿Y además YA fue programado?
                if (scheduled[i] == true) {
                    alguien_lo_produjo = true;
                    break;  // Ya encontramos a alguien, suficiente
                }
            }
        }
        
        // Si nadie ha producido este recurso todavía
        if (!alguien_lo_produjo) {
            return false;  // No se pueden satisfacer las dependencias
        }
    }
    
    return true;  // Todas las dependencias están OK
}

std::vector<const Process*> GraspOptimizer::getEligibleProcesses(
    const std::map<std::string, int>& current_stocks,
    const std::vector<bool>& scheduled) const
{
    std::vector<const Process*> eligible;
    
    // TODO: Recorrer processes con índice i
	for (size_t i = 0; i < processes.size(); i++)
	{
		if (scheduled[i] || !hasResourcesFor(processes[i], current_stocks) || !areDependenciesSatisfied(processes[i], scheduled, current_stocks))
			continue;

		eligible.emplace_back(processes[i]);
	}
    // TODO: Si scheduled[i] == true, skip (ya está programado)
    // TODO: Si !hasResourcesFor(processes[i], current_stocks), skip
    // TODO: Si !areDependenciesSatisfied(processes[i], scheduled, current_stocks), skip
    // TODO: Si pasa todo, añadir &processes[i] al vector eligible
    
    return eligible;
}

int GraspOptimizer::calculateSlack(const Process& proc,
                                   const std::map<std::string, int>& current_stocks,
                                   int current_time) const
{
    int time_available = max_time - current_time;
    
    // Estimación simple: 
    // tiempo_necesario = delay del proceso + estimación de procesos posteriores
    int num_outputs = proc.produces.size();
    int estimated_subsequent = proc.delay * (num_outputs > 0 ? num_outputs : 1);
    
    int slack = time_available - estimated_subsequent;
    
    return slack;
}

double GraspOptimizer::calculateRank(const Process& proc) const
{
    double rank = 0.0;
    
    // TODO: Cuando integres con simulator, recibir target_resources como parámetro
    std::vector<std::string> target_resources;  // Vacío por ahora
    
    // 1. Valor por lo que produce
    for (const auto& [resource, qty] : proc.produces) {
        double value = qty;
        
        // BONUS CRÍTICO: si produce un recurso objetivo
        bool is_target = false;
        for (const auto& target : target_resources) {
            if (resource == target) {
                rank += 100000.0 * qty;  // PRIORIDAD ABSOLUTA
                is_target = true;
                break;
            }
        }
        
        // Si NO es objetivo, aplicar heurística de demanda
        if (!is_target) {
            // Contar cuántos procesos necesitan este recurso
            int num_processes_need_it = 0;
            for (const auto& p : processes) {
                if (p.requisites.count(resource)) {
                    num_processes_need_it++;
                }
            }
            
            // Bonus por demanda: recursos muy demandados son valiosos
            // porque son productos intermedios necesarios
            double demand_bonus = 1.0 + num_processes_need_it * 0.5;
            
            // Bonus adicional si tiene MUCHA demanda (>5 procesos)
            if (num_processes_need_it > 5) {
                demand_bonus *= 2.0;  // Es un cuello de botella
            }
            
            value *= demand_bonus;
        }
        
        rank += value;
    }
    
    // 2. Penalización por lo que consume (más suave)
    for (const auto& [resource, qty] : proc.requisites) {
        rank -= qty * 0.3;
    }
    
    // 3. Pequeño bonus por delay
    rank += proc.delay * 0.1;
    
    return rank;
}

double GraspOptimizer::calculatePriority(const Process& proc, 
                                        const std::map<std::string, int>& current_stocks,
                                        int current_time,
                                        PriorityRule rule) const
{
    switch (rule) {
        case LFT:  // Latest Finish Time (menor es mejor)
            // Prioridad = -1 * (max_time - current_time - proc.delay)
            // Cuanto menos tiempo quede, más urgente
            return -(max_time - current_time - proc.delay);
            
        case MTS:  // Minimum Total Slack (menor slack = más urgente)
            return -calculateSlack(proc, current_stocks, current_time);
            
        case GRPW: // Greatest Rank (mayor rank = más urgente)
            return calculateRank(proc);
            
        case SPT:  // Shortest Processing Time (menor delay = primero)
            return -proc.delay;
            
        case RANDOM:
            return rand() % 1000;  // Aleatorio entre 0-999
            
        default:
            return 0.0;
    }
}

const Process* GraspOptimizer::selectFromRCL(
    const std::vector<const Process*>& eligible,
    const std::map<std::string, int>& current_stocks,
    int current_time,
    PriorityRule rule,
    double alpha) const
{
    if (eligible.empty()) {
        return nullptr;
    }
    
    // 1. Calcular prioridad de cada proceso elegible
    std::vector<std::pair<const Process*, double>> candidates;
    
    for (const auto* proc : eligible) {
        double priority = calculatePriority(*proc, current_stocks, current_time, rule);
        candidates.push_back({proc, priority});
    }
    
    // 2. Ordenar por prioridad (de mayor a menor)
    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;  // Mayor prioridad primero
        });
    
    // 3. Construir la RCL (Restricted Candidate List)
    // RCL contiene los mejores α% de candidatos
    
    double best_priority = candidates[0].second;
    double worst_priority = candidates.back().second;
    
    // Threshold: solo candidatos con prioridad >= threshold entran en RCL
    double threshold = worst_priority + alpha * (best_priority - worst_priority);
    
    std::vector<const Process*> rcl;
    for (const auto& [proc, priority] : candidates) {
        if (priority >= threshold) {
            rcl.push_back(proc);
        }
    }
    
    // 4. Elegir ALEATORIAMENTE uno de la RCL
    int random_index = rand() % rcl.size();
    return rcl[random_index];
}

Solution GraspOptimizer::constructGreedySolution(PriorityRule rule, double alpha)
{
    Solution solution;
    
    // Estado de la construcción
    std::vector<bool> scheduled(processes.size(), false);
    std::map<std::string, int> current_stocks = initial_stocks;
    int current_time = 0;
    
    // Lista de procesos en ejecución (start_time, process_index)
    std::vector<std::pair<int, size_t>> running;
    
    int scheduled_count = 0;
    int total_processes = processes.size();
    
    // Mientras haya procesos por programar
    while (scheduled_count < total_processes && current_time < max_time) {
        
        // 1. Terminar procesos que finalizan en este ciclo
        std::vector<std::pair<int, size_t>> still_running;
        
        for (const auto& [start_time, proc_idx] : running) {
            int finish_time = start_time + processes[proc_idx].delay;
            
            if (finish_time <= current_time) {
                // Este proceso terminó
                produceResources(current_stocks, processes[proc_idx]);
                
                // Añadir al schedule
                solution.schedule.push_back(
                    ScheduledActivity(processes[proc_idx].name, start_time, finish_time)
                );
            } else {
                // Sigue ejecutándose
                still_running.push_back({start_time, proc_idx});
            }
        }
        
        running = still_running;
        
        // 2. Obtener procesos elegibles
        std::vector<const Process*> eligible = getEligibleProcesses(current_stocks, scheduled);
        
        // 3. Si hay elegibles, programar UNO
        if (!eligible.empty()) {
            const Process* selected = selectFromRCL(eligible, current_stocks, current_time, rule, alpha);
            
            if (selected != nullptr) {
                // Encontrar el índice del proceso seleccionado
                size_t proc_idx = 0;
                for (size_t i = 0; i < processes.size(); i++) {
                    if (&processes[i] == selected) {
                        proc_idx = i;
                        break;
                    }
                }
                
                // Consumir recursos inmediatamente
                consumeResources(current_stocks, *selected);
                
                // Marcar como programado
                scheduled[proc_idx] = true;
                scheduled_count++;
                
                // Añadir a la lista de ejecución
                running.push_back({current_time, proc_idx});
            }
        }
        
        // 4. Avanzar el tiempo
        current_time++;
    }
    
    // Esperar a que terminen los procesos que aún están corriendo
    while (!running.empty()) {
        std::vector<std::pair<int, size_t>> still_running;
        
        for (const auto& [start_time, proc_idx] : running) {
            int finish_time = start_time + processes[proc_idx].delay;
            
            if (finish_time <= current_time) {
                produceResources(current_stocks, processes[proc_idx]);
                solution.schedule.push_back(
                    ScheduledActivity(processes[proc_idx].name, start_time, finish_time)
                );
            } else {
                still_running.push_back({start_time, proc_idx});
            }
        }
        
        running = still_running;
        
        if (!running.empty()) {
            current_time++;
        }
    }
    
    // Calcular makespan y stocks finales
    solution.makespan = calculateMakespan(solution);
    solution.final_stocks = current_stocks;
    
    return solution;
}

Solution GraspOptimizer::solve(int iterations, double alpha_param)
{
    num_iterations = iterations;
    alpha = alpha_param;
    
    std::cout << "Iniciando GRASP con " << iterations << " iteraciones (alpha=" << alpha << ")...\n";
    
    // Array de reglas a probar
    PriorityRule rules[] = {LFT, MTS, GRPW, SPT, RANDOM};
    int num_rules = 5;
    
    for (int iter = 0; iter < num_iterations; iter++) {
        // 1. Elegir una regla (rotar entre todas)
        PriorityRule current_rule = rules[iter % num_rules];
        
        // 2. FASE CONSTRUCTIVA: Construir solución greedy randomizada
        Solution candidate = constructGreedySolution(current_rule, alpha);
        
        // 3. FASE DE MEJORA: Aplicar búsqueda local
        localSearch(candidate);
        
        // 4. Actualizar mejor solución si es mejor
        if (candidate.makespan < best_solution.makespan) {
            best_solution = candidate;
            std::cout << "  Iteración " << iter << ": Nueva mejor solución (makespan=" 
                      << best_solution.makespan << ")\n";
        }
        
        // Mostrar progreso cada 10%
        if ((iter + 1) % (iterations / 10) == 0) {
            std::cout << "  Progreso: " << (iter + 1) << "/" << iterations 
                      << " (" << ((iter + 1) * 100 / iterations) << "%)\n";
        }
    }
    
    std::cout << "GRASP completado. Mejor makespan encontrado: " << best_solution.makespan << "\n";
    
    return best_solution;
}
