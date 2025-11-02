/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulator.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 18:12:43 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/02 23:12:58 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "parser.hpp"

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

struct ResultSim {
    std::map<std::string, int> stocks_after;
    int time_total;
    std::vector<std::pair<int, std::string>> trace;  // (ciclo, nombre_proceso)
    bool valid;  // ¿se pudo ejecutar todo?
};

class Simulator {
	private:
		int	time;
    	std::vector<execution> history;  // (ciclo, nombre_proceso)
    	std::map<std::string, int> stocks_now;
		std::vector<Process> process_pending;
		std::vector<exec_process> process_executing;
		Parser info;
    public:
		Simulator(Parser P);
		void simulate();
		bool haveStocksFor(Process to_do);
		bool start_execution(const Process& to_do);
		void end_execution(std::string process_n);
		void substractStocks(std::string stock, int amount);
		void addStocks(std::string stock, int amount);
		std::vector<Process> executableProcesses();
		void checkRunningProcs();
		// Getters para inspeccionar resultados desde main
		const std::vector<execution>& getHistory() const { return history; }
		const std::map<std::string, int>& getStocksNow() const { return stocks_now; }
		int getCurrentTime() const { return time; }
};

#endif