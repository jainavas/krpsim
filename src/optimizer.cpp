/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   optimizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 01:08:45 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 01:15:33 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/optimizer.hpp"

void DependencyGraph::analyze_full_chain(const std::string& target,
                                        int target_quantity,
                                        const std::map<std::string, int>& current_stocks,
                                        const std::vector<Process>& processes)
{
    // Limpiar análisis anterior
    resource_info.clear();
    process_info.clear();
    all_processes = &processes;
    
    // Fase 1: BFS inverso con cantidades
    analyze_quantities_backward(target, target_quantity, current_stocks, processes);
    
    // Fase 2: Análisis temporal forward (camino crítico)
    analyze_time_forward(processes);
    
    // Fase 3: Análisis temporal backward
    analyze_time_backward(target, processes);
    
    // Fase 4: Calcular prioridades finales (con tiempo incluido)
    calculate_final_priorities();
}

void DependencyGraph::analyze_quantities_backward(const std::string& target,
                                                  int target_quantity,
                                                  const std::map<std::string, int>& current_stocks,
                                                  const std::vector<Process>& processes)
{
    std::queue<std::pair<std::string, int>> q;  // (recurso, cantidad)
    std::map<std::string, int> visited_quantities;
    
    q.push({target, target_quantity});
    visited_quantities[target] = target_quantity;
    
    resource_info[target].distance_to_target = 0;
    resource_info[target].total_needed = target_quantity;
    
    while (!q.empty()) {
        auto [current_resource, current_qty] = q.front();
        q.pop();
        
        int current_dist = resource_info[current_resource].distance_to_target;
        
        // Buscar TODOS los procesos que producen este recurso
        std::vector<const Process*> producers;
        for (const auto& proc : processes) {
            if (proc.produces.count(current_resource)) {
                producers.push_back(&proc);
                resource_info[current_resource].produced_by.push_back(proc.name);
            }
        }
        
        if (producers.empty()) {
            // Es un recurso base (del stock inicial)
            resource_info[current_resource].available_in_stock = 
                current_stocks.count(current_resource) ? 
                current_stocks.at(current_resource) : 0;
            continue;
        }
        
        // Elegir el mejor productor
        const Process* best_producer = choose_best_producer(producers);
        
        // Calcular cuántas veces necesitamos ejecutar este proceso
        int produced_per_run = best_producer->produces.at(current_resource);
        int times_needed = (current_qty + produced_per_run - 1) / produced_per_run;
        
        // Propagar necesidades a los requisitos
        for (const auto& [req, req_qty] : best_producer->requisites) {
            int total_req_needed = req_qty * times_needed;
            
            if (visited_quantities.count(req)) {
                visited_quantities[req] += total_req_needed;
                resource_info[req].total_needed += total_req_needed;
            } else {
                visited_quantities[req] = total_req_needed;
                resource_info[req].distance_to_target = current_dist + 1;
                resource_info[req].total_needed = total_req_needed;
                q.push({req, total_req_needed});
            }
        }
        
        // Considerar subproductos
        for (const auto& [prod, prod_qty] : best_producer->produces) {
            if (prod != current_resource) {
                int generated = prod_qty * times_needed;
                if (resource_info.count(prod)) {
                    resource_info[prod].total_needed -= generated;
                    if (resource_info[prod].total_needed < 0)
                        resource_info[prod].total_needed = 0;
                }
            }
        }
    }
    
    // Calcular availability ratios
    calculate_availability_and_priorities(current_stocks);
}

void DependencyGraph::analyze_time_forward(const std::vector<Process>& processes)
{
    auto sorted_processes = topological_sort(processes);
    
    for (const auto& proc_name : sorted_processes) {
        const Process* proc = find_process(processes, proc_name);
        if (!proc) continue;
        
        ProcessInfo& info = process_info[proc_name];
        info.name = proc_name;
        
        // Earliest start = cuando todos los requisitos están disponibles
        int earliest_start = 0;
        
        for (const auto& [req, qty] : proc->requisites) {
            if (resource_info.count(req)) {
                int req_time = resource_info[req].earliest_available_time;
                earliest_start = std::max(earliest_start, req_time);
            }
        }
        
        info.earliest_start_time = earliest_start;
        info.earliest_finish_time = earliest_start + proc->delay;
        
        // Actualizar cuando estarán disponibles los outputs
        for (const auto& [prod, qty] : proc->produces) {
            if (!resource_info.count(prod)) {
                resource_info[prod].earliest_available_time = info.earliest_finish_time;
            } else {
                resource_info[prod].earliest_available_time = 
                    std::min(resource_info[prod].earliest_available_time,
                            info.earliest_finish_time);
            }
            
            // Calcular tiempo para producir cantidad necesaria
            if (resource_info[prod].total_needed > 0 && qty > 0) {
                int times = (resource_info[prod].total_needed + qty - 1) / qty;
                resource_info[prod].time_to_produce_needed = 
                    info.earliest_finish_time * times;
            }
            
            resource_info[prod].time_to_produce = proc->delay;
        }
    }
}

void DependencyGraph::analyze_time_backward(const std::string& target,
                                           const std::vector<Process>& processes)
{
    // El objetivo debe estar listo en su earliest_finish_time
    int project_deadline = resource_info.count(target) ? 
                          resource_info[target].earliest_available_time : 10000;
    
    // Ordenar procesos en orden inverso
    auto sorted = topological_sort(processes);
    std::reverse(sorted.begin(), sorted.end());
    
    for (const auto& proc_name : sorted) {
        const Process* proc = find_process(processes, proc_name);
        if (!proc) continue;
        
        ProcessInfo& info = process_info[proc_name];
        
        // Latest finish = el mínimo de los latest start de procesos que usan sus outputs
        int latest_finish = project_deadline;
        
        for (const auto& [prod, qty] : proc->produces) {
            // Buscar procesos que usan este producto
            for (const auto& other_proc : processes) {
                if (other_proc.requisites.count(prod)) {
                    if (process_info.count(other_proc.name)) {
                        latest_finish = std::min(latest_finish,
                            process_info[other_proc.name].latest_start_time);
                    }
                }
            }
        }
        
        info.latest_finish_time = latest_finish;
        info.latest_start_time = latest_finish - proc->delay;
        
        // Calcular holgura (slack)
        info.slack = info.latest_start_time - info.earliest_start_time;
        
        // Es crítico si no tiene holgura
        info.is_critical = (info.slack <= 0);
        
        // Marcar recursos del camino crítico
        if (info.is_critical) {
            for (const auto& [prod, qty] : proc->produces) {
                resource_info[prod].is_on_critical_path = true;
            }
            for (const auto& [req, qty] : proc->requisites) {
                resource_info[req].is_on_critical_path = true;
            }
        }
    }
    
    // Calcular longitud de camino crítico para cada recurso
    calculate_critical_path_lengths(target);
}

void DependencyGraph::calculate_critical_path_lengths(const std::string& target)
{
    std::queue<std::string> q;
    std::set<std::string> visited;
    
    if (resource_info.count(target)) {
        resource_info[target].critical_path_length = 0;
        q.push(target);
    }
    
    while (!q.empty()) {
        std::string resource = q.front();
        q.pop();
        
        if (visited.count(resource)) continue;
        visited.insert(resource);
        
        int current_length = resource_info[resource].critical_path_length;
        
        // Buscar procesos que producen este recurso
        for (const auto& proc_name : resource_info[resource].produced_by) {
            if (!process_info.count(proc_name)) continue;
            
            const Process* proc = find_process(*all_processes, proc_name);
            if (!proc) continue;
            
            int new_length = current_length + proc->delay;
            
            // Propagar a los requisitos
            for (const auto& [req, qty] : proc->requisites) {
                if (!resource_info.count(req) || 
                    resource_info[req].critical_path_length < new_length) {
                    resource_info[req].critical_path_length = new_length;
                    q.push(req);
                }
            }
        }
    }
}

std::vector<std::string> DependencyGraph::topological_sort(
    const std::vector<Process>& processes) const
{
    std::map<std::string, std::vector<std::string>> graph;
    std::map<std::string, int> in_degree;
    
    // Inicializar
    for (const auto& proc : processes) {
        in_degree[proc.name] = 0;
        graph[proc.name] = {};
    }
    
    // Construir aristas
    for (const auto& proc : processes) {
        for (const auto& [req, qty] : proc.requisites) {
            for (const auto& other : processes) {
                if (other.produces.count(req)) {
                    graph[other.name].push_back(proc.name);
                    in_degree[proc.name]++;
                }
            }
        }
    }
    
    // Kahn's algorithm
    std::queue<std::string> q;
    for (const auto& [proc, degree] : in_degree) {
        if (degree == 0) q.push(proc);
    }
    
    std::vector<std::string> result;
    while (!q.empty()) {
        std::string proc = q.front();
        q.pop();
        result.push_back(proc);
        
        for (const auto& neighbor : graph[proc]) {
            in_degree[neighbor]--;
            if (in_degree[neighbor] == 0) {
                q.push(neighbor);
            }
        }
    }
    
    return result;
}

const Process* DependencyGraph::choose_best_producer(
    const std::vector<const Process*>& producers) const
{
    const Process* best = producers[0];
    double best_efficiency = -1;
    
    for (const auto* proc : producers) {
        double production = 0;
        for (const auto& [res, qty] : proc->produces) {
            production += qty;
        }
        
        double cost = proc->delay;
        for (const auto& [req, qty] : proc->requisites) {
            cost += qty * (resource_info.count(req) ? 1 : 10);
        }
        
        double efficiency = production / (cost + 1);
        
        if (efficiency > best_efficiency) {
            best_efficiency = efficiency;
            best = proc;
        }
    }
    
    return best;
}

void DependencyGraph::calculate_availability_and_priorities(
    const std::map<std::string, int>& current_stocks)
{
    for (auto& [resource, info] : resource_info) {
        info.available_in_stock = 
            current_stocks.count(resource) ? 
            current_stocks.at(resource) : 0;
        
        if (info.total_needed > 0) {
            info.availability_ratio = 
                (double)info.available_in_stock / info.total_needed;
        } else {
            info.availability_ratio = 999.0;
        }
        
        info.is_bottleneck = 
            (info.availability_ratio < 0.5) || 
            (info.availability_ratio < 1.0 && info.distance_to_target > 3);
    }
}

void DependencyGraph::calculate_final_priorities()
{
    for (auto& [resource, info] : resource_info) {
        int priority = 0;
        
        // 1. Distancia al objetivo
        if (info.distance_to_target < 9999) {
            priority += 1000 / (1 << std::min(info.distance_to_target, 10));
        }
        
        // 2. Camino crítico (máxima prioridad)
        if (info.is_on_critical_path) {
            priority += 10000;
        }
        
        // 3. Longitud del camino crítico desde este recurso
        priority += info.critical_path_length * 10;
        
        // 4. Tiempo necesario para producir
        if (info.time_to_produce > 0) {
            priority += std::min(info.time_to_produce * 5, 5000);
        }
        
        // 5. Bottleneck
        if (info.is_bottleneck) {
            priority += 5000;
        }
        
        // 6. Ratio de disponibilidad
        if (info.availability_ratio < 0.1) {
            priority += 3000;
        } else if (info.availability_ratio < 0.5) {
            priority += 1000;
        } else if (info.availability_ratio < 1.0) {
            priority += 500;
        }
        
        // 7. Cantidad necesaria
        priority += std::min(info.total_needed, 1000);
        
        info.priority = priority;
    }
}

const Process* DependencyGraph::find_process(const std::vector<Process>& processes,
                                            const std::string& name) const
{
    for (const auto& p : processes) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

// Getters
int DependencyGraph::get_resource_priority(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).priority;
    return 0;
}

bool DependencyGraph::is_bottleneck(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).is_bottleneck;
    return false;
}

int DependencyGraph::get_total_needed(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).total_needed;
    return 0;
}

double DependencyGraph::get_availability_ratio(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).availability_ratio;
    return 1.0;
}

bool DependencyGraph::is_on_critical_path(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).is_on_critical_path;
    return false;
}

int DependencyGraph::get_critical_path_length(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).critical_path_length;
    return 0;
}

int DependencyGraph::get_time_to_produce(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).time_to_produce;
    return 0;
}

int DependencyGraph::get_earliest_available_time(const std::string& resource) const
{
    if (resource_info.count(resource))
        return resource_info.at(resource).earliest_available_time;
    return 0;
}

bool DependencyGraph::is_process_critical(const std::string& process_name) const
{
    if (process_info.count(process_name))
        return process_info.at(process_name).is_critical;
    return false;
}

int DependencyGraph::get_process_slack(const std::string& process_name) const
{
    if (process_info.count(process_name))
        return process_info.at(process_name).slack;
    return 999;
}

int DependencyGraph::get_earliest_start_time(const std::string& process_name) const
{
    if (process_info.count(process_name))
        return process_info.at(process_name).earliest_start_time;
    return 0;
}

void DependencyGraph::print_analysis() const
{
    std::cout << "\n=== ANÁLISIS DE RECURSOS (CON TIEMPO) ===\n";
    
    std::vector<std::pair<std::string, ResourceInfo>> sorted;
    for (const auto& [res, info] : resource_info) {
        sorted.push_back({res, info});
    }
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) { return a.second.priority > b.second.priority; });
    
    for (const auto& [resource, info] : sorted) {
        std::cout << resource << ":\n";
        std::cout << "  Distancia: " << info.distance_to_target << "\n";
        std::cout << "  Necesario: " << info.total_needed << "\n";
        std::cout << "  Disponible: " << info.available_in_stock << "\n";
        std::cout << "  Ratio: " << std::fixed << std::setprecision(2) 
                  << info.availability_ratio << "\n";
        std::cout << "  Tiempo producir 1: " << info.time_to_produce << "\n";
        std::cout << "  Tiempo total necesario: " << info.time_to_produce_needed << "\n";
        std::cout << "  Longitud camino crítico: " << info.critical_path_length << "\n";
        std::cout << "  En camino crítico: " << (info.is_on_critical_path ? "SÍ ⚡" : "no") << "\n";
        std::cout << "  Prioridad: " << info.priority << "\n";
        std::cout << "  Bottleneck: " << (info.is_bottleneck ? "SÍ" : "no") << "\n";
        if (!info.produced_by.empty()) {
            std::cout << "  Producido por: ";
            for (const auto& proc : info.produced_by)
                std::cout << proc << " ";
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    std::cout << "\n=== PROCESOS CRÍTICOS ===\n";
    for (const auto& [proc_name, info] : process_info) {
        if (info.is_critical) {
            std::cout << proc_name << " ⚡\n";
            std::cout << "  Earliest start: " << info.earliest_start_time << "\n";
            std::cout << "  Latest start: " << info.latest_start_time << "\n";
            std::cout << "  Holgura: " << info.slack << "\n";
            std::cout << "\n";
        }
    }
}