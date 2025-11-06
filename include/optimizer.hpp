/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   optimizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/03 01:08:13 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 18:36:29 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "parser.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>

// ============================================================================
// ESTRUCTURAS DE DATOS
// ============================================================================

// Representa una actividad programada en el schedule
struct ScheduledActivity {
    std::string process_name;
    int start_time;
    int finish_time;
    
    ScheduledActivity() : start_time(0), finish_time(0) {}
    ScheduledActivity(std::string name, int start, int finish) 
        : process_name(name), start_time(start), finish_time(finish) {}
};

// Representa una solución completa (schedule)
struct Solution {
    std::vector<ScheduledActivity> schedule;  // Lista de actividades programadas
    int makespan;                              // Tiempo total (duración del proyecto)
    std::map<std::string, int> final_stocks;  // Stocks finales
    
    Solution() : makespan(std::numeric_limits<int>::max()) {}
};

// ============================================================================
// ENUMS PARA PRIORITY RULES
// ============================================================================

enum PriorityRule {
    LFT,    // Latest Finish Time
    MTS,    // Minimum Total Slack  
    GRPW,   // Greatest Rank Positional Weight
    SPT,    // Shortest Processing Time
    RANDOM  // Random (para diversidad)
};

// ============================================================================
// GRASP OPTIMIZER
// ============================================================================

class GraspOptimizer {
private:
    // Datos del problema
    const std::vector<Process>& processes;
    std::map<std::string, int> initial_stocks;
    int max_time;  // Tiempo máximo de simulación
    
    // Parámetros GRASP
    int num_iterations;     // Número de iteraciones GRASP
    double alpha;           // Parámetro RCL (0.0 = greedy puro, 1.0 = random puro)
    
    // Mejor solución encontrada
    Solution best_solution;
    
public:
    GraspOptimizer(const std::vector<Process>& procs, 
                   const std::map<std::string, int>& stocks,
                   int max_t = 10000);
    
    // Método principal - ejecuta GRASP y devuelve la mejor solución
    Solution solve(int iterations = 100, double alpha_param = 0.3);
    
    // Getters
    const Solution& getBestSolution() const { return best_solution; }
    
private:
    // ========================================================================
    // FASE CONSTRUCTIVA (Greedy Randomizado con Serial SGS)
    // ========================================================================
    
    // Construye una solución usando Serial SGS con randomización
    Solution constructGreedySolution(PriorityRule rule, double alpha);
    
    // Calcula la prioridad de un proceso según la regla
    double calculatePriority(const Process& proc, 
                            const std::map<std::string, int>& current_stocks,
                            int current_time,
                            PriorityRule rule) const;
    
    // Obtiene los procesos elegibles en un momento dado
    std::vector<const Process*> getEligibleProcesses(
        const std::map<std::string, int>& current_stocks,
        const std::vector<bool>& scheduled) const;
    
    // Selecciona un proceso de la RCL (Restricted Candidate List)
    const Process* selectFromRCL(const std::vector<const Process*>& eligible,
                                 const std::map<std::string, int>& current_stocks,
                                 int current_time,
                                 PriorityRule rule,
                                 double alpha) const;
    
    // Verifica si un proceso tiene suficientes recursos
    bool hasResourcesFor(const Process& proc, 
                        const std::map<std::string, int>& stocks) const;
    
    // Verifica si las dependencias están satisfechas
    bool areDependenciesSatisfied(const Process& proc,
                                  const std::vector<bool>& scheduled,
                                  const std::map<std::string, int>& stocks) const;
    
    // ========================================================================
    // FASE DE MEJORA LOCAL (Forward-Backward Improvement)
    // ========================================================================
    
    // Mejora una solución usando FBI
    void localSearch(Solution& solution);
    
    // Forward pass: intenta adelantar cada actividad
    bool forwardPass(Solution& solution);
    
    // Backward pass: intenta retrasar actividades sin afectar el makespan
    bool backwardPass(Solution& solution);
    
    // Verifica si un schedule es factible
    bool isScheduleFeasible(const Solution& solution) const;
    
    // ========================================================================
    // FUNCIONES AUXILIARES
    // ========================================================================
    
    // Actualiza los stocks después de iniciar un proceso
    void consumeResources(std::map<std::string, int>& stocks, 
                         const Process& proc) const;
    
    // Actualiza los stocks después de terminar un proceso
    void produceResources(std::map<std::string, int>& stocks, 
                         const Process& proc) const;
    
    // Calcula el makespan de una solución
    int calculateMakespan(const Solution& solution) const;
    
    // Busca un proceso por nombre
    const Process* findProcess(const std::string& name) const;
    
    // Calcula el slack de un proceso (para MTS rule)
    int calculateSlack(const Process& proc,
                      const std::map<std::string, int>& current_stocks,
                      int current_time) const;
    
    // Calcula el rank de un proceso (para GRPW rule)
    double calculateRank(const Process& proc) const;
};

#endif