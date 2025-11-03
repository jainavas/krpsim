/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 18:12:43 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 02:12:08 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "parser.hpp"
#include "optimizer.hpp"

struct exec_process
{
	Process proc;
	int start;
};

struct execution {
    int start;
    int end;
    std::string process_name;
    std::map<std::string, int> stocks_snapshot;
};

class Simulator {
private:
    int	time;
    int max_cycles;
    std::vector<execution> history;
    std::map<std::string, int> stocks_now;
    std::vector<Process> process_pending;
    std::vector<exec_process> process_executing;
    Parser info;
    
    // Para optimización
    DependencyGraph dep_graph;
    std::string target_stock;
    int target_quantity;
	bool liquidation_mode;
    
public:
    Simulator(Parser P);
    void simulate();
    
    // Setters
    void setTargetStock(const std::string& target) { target_stock = target; }
    void setTargetQuantity(int qty) { target_quantity = qty; }
    void setMaxCycles(int max) { max_cycles = max; }
    
    // Getters
    const std::vector<execution>& getHistory() const { return history; }
    const std::map<std::string, int>& getStocksNow() const { return stocks_now; }
    int getCurrentTime() const { return time; }
    
    // Métodos de simulación
    bool haveStocksFor(Process to_do);
    bool start_execution(const Process& to_do);
    void end_execution(std::string process_n);
    void substractStocks(std::string stock, int amount);
    void addStocks(std::string stock, int amount);
    std::vector<Process> executableProcesses();
    void checkRunningProcs();
    
private:
    std::vector<Process> executableProcesses_Smart();
    int smart_score(const Process& p);
};

#endif