/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   optimizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 01:08:13 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 01:15:10 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "parser.hpp"
#include <queue>
#include <set>
#include <iomanip>

class DependencyGraph {
private:
    // Información completa de cada recurso
    struct ResourceInfo {
        // Análisis de dependencias
        int distance_to_target;           // Distancia en el grafo al objetivo
        int total_needed;                 // Cantidad total necesaria
        int available_in_stock;           // Cantidad disponible en stock
        double availability_ratio;        // available / needed
        bool is_bottleneck;              // ¿Es cuello de botella?
        std::vector<std::string> produced_by;  // Procesos que lo producen
        
        // Análisis temporal
        int time_to_produce;              // Tiempo mínimo para producir 1 unidad
        int time_to_produce_needed;       // Tiempo para producir lo necesario
        int critical_path_length;         // Longitud del camino crítico
        int earliest_available_time;      // Cuándo estará disponible
        bool is_on_critical_path;         // ¿Está en el camino crítico?
        
        // Prioridad final calculada
        int priority;
        
        // Constructor con valores por defecto
        ResourceInfo() : distance_to_target(9999), total_needed(0),
                        available_in_stock(0), availability_ratio(0.0),
                        is_bottleneck(false), time_to_produce(0),
                        time_to_produce_needed(0), critical_path_length(0),
                        earliest_available_time(0), is_on_critical_path(false),
                        priority(0) {}
    };
    
    // Información temporal de cada proceso
    struct ProcessInfo {
        std::string name;
        int earliest_start_time;     // Cuándo puede empezar como mínimo
        int earliest_finish_time;    // Cuándo termina como mínimo
        int latest_start_time;       // Último momento que puede empezar
        int latest_finish_time;      // Último momento que puede terminar
        int slack;                   // Holgura (latest - earliest)
        bool is_critical;            // Sin holgura = crítico
        
        ProcessInfo() : earliest_start_time(0), earliest_finish_time(0),
                       latest_start_time(0), latest_finish_time(0),
                       slack(0), is_critical(false) {}
    };
    
    std::map<std::string, ResourceInfo> resource_info;
    std::map<std::string, ProcessInfo> process_info;
    const std::vector<Process>* all_processes;  // Referencia a todos los procesos
    
public:
    DependencyGraph() : all_processes(nullptr) {}
    
    // Análisis completo: dependencias, cantidades, stocks y tiempo
    void analyze_full_chain(const std::string& target,
                           int target_quantity,
                           const std::map<std::string, int>& current_stocks,
                           const std::vector<Process>& processes);
    
    // Getters para información de recursos
    int get_resource_priority(const std::string& resource) const;
    bool is_bottleneck(const std::string& resource) const;
    int get_total_needed(const std::string& resource) const;
    double get_availability_ratio(const std::string& resource) const;
    
    // Getters para información temporal de recursos
    bool is_on_critical_path(const std::string& resource) const;
    int get_critical_path_length(const std::string& resource) const;
    int get_time_to_produce(const std::string& resource) const;
    int get_earliest_available_time(const std::string& resource) const;
    
    // Getters para información temporal de procesos
    bool is_process_critical(const std::string& process_name) const;
    int get_process_slack(const std::string& process_name) const;
    int get_earliest_start_time(const std::string& process_name) const;
    
    // Debug: imprimir análisis completo
    void print_analysis() const;
    
private:
    // Fase 1: Análisis de cantidades (BFS inverso)
    void analyze_quantities_backward(const std::string& target,
                                    int target_quantity,
                                    const std::map<std::string, int>& current_stocks,
                                    const std::vector<Process>& processes);
    
    // Fase 2: Análisis temporal forward (earliest times)
    void analyze_time_forward(const std::vector<Process>& processes);
    
    // Fase 3: Análisis temporal backward (latest times)
    void analyze_time_backward(const std::string& target,
                              const std::vector<Process>& processes);
    
    // Fase 4: Calcular prioridades finales
    void calculate_final_priorities();
    
    // Calcular longitud de caminos críticos
    void calculate_critical_path_lengths(const std::string& target);
    
    // Ordenamiento topológico de procesos
    std::vector<std::string> topological_sort(const std::vector<Process>& processes) const;
    
    // Elegir el mejor productor de un recurso
    const Process* choose_best_producer(const std::vector<const Process*>& producers) const;
    
    // Helpers
    const Process* find_process(const std::vector<Process>& processes,
                               const std::string& name) const;
    
    void calculate_availability_and_priorities(
        const std::map<std::string, int>& current_stocks);
    
    int calculate_priority_value(const ResourceInfo& info) const;
};

#endif