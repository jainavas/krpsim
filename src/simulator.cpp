/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 18:12:17 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 02:16:30 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/simulator.hpp"

Simulator::Simulator(Parser P)
	: time(0), stocks_now(P.getStocks()), process_pending(P.getAllProcesses()),
	  info(P), target_quantity(100)
{
}

bool Simulator::haveStocksFor(Process to_do)
{
	for (const auto &[resource, amount_needed] : to_do.requisites)
	{
		auto stock_it = stocks_now.find(resource);
		if (stock_it != stocks_now.end())
		{
			if (stock_it->second - amount_needed < 0)
				return false;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void Simulator::substractStocks(std::string stock, int amount)
{
	auto stock_it = stocks_now.find(stock);
	if (stock_it != stocks_now.end())
	{
		stock_it->second -= amount;
	}
	else
	{
		stocks_now[stock] = -amount;
	}
}

void Simulator::addStocks(std::string stock, int amount)
{
	auto stock_it = stocks_now.find(stock);
	if (stock_it != stocks_now.end())
	{
		stock_it->second += amount;
	}
	else
	{
		stocks_now[stock] = amount;
	}
}

bool Simulator::start_execution(const Process &to_do)
{
	if (!haveStocksFor(to_do))
		return false;

	// Restar stocks
	for (const auto &[resource, amount_needed] : to_do.requisites)
		substractStocks(resource, amount_needed);

	// Ejecutar
	process_executing.push_back(exec_process{to_do, time});
	return true;
}

void Simulator::end_execution(std::string process_n)
{
	auto it = std::find_if(process_executing.begin(), process_executing.end(),
						   [&](const exec_process &e)
						   { return e.proc.name == process_n; });

	if (it != process_executing.end())
	{
		for (auto &st : it->proc.produces)
			addStocks(st.first, st.second);
		history.push_back(execution{it->start, time, it->proc.name, stocks_now});
		process_executing.erase(it);
	}
}

void Simulator::simulate()
{
    time = 0;
    max_cycles = 10000;
	liquidation_mode = false;
    // Análisis inicial si hay objetivo
    if (!target_stock.empty()) {
        dep_graph.analyze_full_chain(
            target_stock,
            target_quantity,
            stocks_now,
            info.getAllProcesses()
        );
    }
    
    while(1)
    {
        checkRunningProcs();
        
        std::vector<Process> can_execute;
        
        can_execute = executableProcesses_Smart();
        
        for (auto& p : can_execute)
            start_execution(p);
        
        // Parar si no hay nada que hacer
        if (can_execute.empty() && process_executing.empty())
            break;
        
        // Parar si alcanzamos el límite de ciclos
        if (time >= max_cycles)
            break;
        
		// if (time >= max_cycles * 0.8)
        //     liquidation_mode = true;
		
        time++;
    }
}

void Simulator::checkRunningProcs()
{
	for (int i = process_executing.size() - 1; i >= 0; --i)
	{
		if (this->time == (process_executing[i].proc.delay + process_executing[i].start))
		{
			end_execution(process_executing[i].proc.name);
		}
	}
}

// bool needsTarget(const Process& p, std::string target)
// {
// 	for (auto& d : p.requisites)
// 		if (d.first == target)
// 			return true;
// 	return false;
// }

std::vector<Process> Simulator::executableProcesses_Smart()
{
    std::vector<Process> executable;
    
    for (auto& p : process_pending) {
        if (haveStocksFor(p))
			// if (liquidation_mode && !needsTarget(p, target_stock))
            executable.push_back(p);
    }
    
    // Ordenar por score
    std::sort(executable.begin(), executable.end(),
        [&](const Process& a, const Process& b) {
            return smart_score(a) > smart_score(b);
        });
    
    return executable;
}

int Simulator::smart_score(const Process& p) {
    int score = 0;
    
    // 1. CRÍTICO: ¿El proceso está en el camino crítico?
    if (dep_graph.is_process_critical(p.name)) {
        score += 20000;  // MÁXIMA PRIORIDAD ABSOLUTA
    }
    
    // 2. Holgura del proceso (slack)
    int slack = dep_graph.get_process_slack(p.name);
    score += (1000 - slack * 10);  // Menos holgura = más urgente
    
    // 3. Score por lo que PRODUCE
    for (auto& [resource, qty] : p.produces) {
        // Prioridad base del recurso
        int resource_priority = dep_graph.get_resource_priority(resource);
        score += resource_priority * qty;
        
        // BONUS: Recurso en camino crítico
        if (dep_graph.is_on_critical_path(resource)) {
            score += 5000 * qty;
        }
        
        // BONUS: Tiempo de producción largo
        int time_to_produce = dep_graph.get_time_to_produce(resource);
        if (time_to_produce > 50) {
            score += 2000 * qty;  // Empezar pronto los procesos largos
        }
        
        // BONUS: Longitud de camino crítico desde aquí
        int crit_path_length = dep_graph.get_critical_path_length(resource);
        score += crit_path_length * 20;
        
        // Escasez
        if (dep_graph.is_bottleneck(resource)) {
            score += 2000 * qty;
        }
        
        double ratio = dep_graph.get_availability_ratio(resource);
        if (ratio < 0.5) {
            score += 1000 * qty;
        }
    }
    
    // 4. Penalización por lo que CONSUME
    for (auto& [resource, qty] : p.requisites) {
        // No consumir recursos críticos salvo que sea necesario
        if (dep_graph.is_on_critical_path(resource)) {
            // Si lo que produce TAMBIÉN es crítico, ok
            bool produces_critical = false;
            for (auto& [prod, qty] : p.produces) {
                if (dep_graph.is_on_critical_path(prod)) {
                    produces_critical = true;
                    break;
                }
            }
            
            if (!produces_critical) {
                score -= 3000 * qty;  // MUCHA penalización
            }
        }
        
        // Escasez del recurso consumido
        double ratio = dep_graph.get_availability_ratio(resource);
        if (ratio < 1.0) {
            score -= (int)(500 * qty / (ratio + 0.1));
        }
        
        if (dep_graph.is_bottleneck(resource)) {
            score -= 1000 * qty;
        }
    }
    
    // 5. Delay del proceso
    // Procesos rápidos son mejores, pero si están en camino crítico
    // el delay ya está considerado
    if (!dep_graph.is_process_critical(p.name)) {
        score -= p.delay * 10;
    }
    
    // 6. Objetivo alcanzado
    if (!target_stock.empty() && 
        stocks_now[target_stock] >= target_quantity) {
        score = score / 10;
    }
    
    return score;
}
