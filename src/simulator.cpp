/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 18:12:17 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/02 23:12:58 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/simulator.hpp"

Simulator::Simulator(Parser P)
    : time(0)
    , stocks_now(P.getStocks())
    , process_pending(P.getAllProcesses())
    , info(P)
{
}

bool Simulator::haveStocksFor(Process to_do)
{
	for (const auto& [resource, amount_needed] : to_do.requisites)
    {
        auto stock_it = stocks_now.find(resource);
        if (stock_it != stocks_now.end()) {
            if (stock_it->second - amount_needed < 0)
                return false;
        } else {
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

bool Simulator::start_execution(const Process& to_do)
{
    // 1. Comprobamos si hay recursos suficientes
    if (!haveStocksFor(to_do))
        return false;

    // 2. Eliminamos el proceso de la lista de pendientes
    auto it = std::find_if(process_pending.begin(), process_pending.end(),
        [&](const Process& p) { return p.name == to_do.name; });

    if (it != process_pending.end())
        process_pending.erase(it);

    // 3. Restamos los recursos requeridos del stock actual
    for (const auto& [resource, amount_needed] : to_do.requisites)
        this->substractStocks(resource, amount_needed);

    // 4. Movemos el proceso a la lista de ejecuciones
    process_executing.push_back(exec_process{to_do, time});

    return true;
}

void Simulator::end_execution(std::string process_n)
{
	auto it = std::find_if(process_executing.begin(), process_executing.end(),
    [&](const exec_process& e) { return e.proc.name == process_n; });

    if (it != process_executing.end())
	{
        for (auto& st : it->proc.produces)
			addStocks(st.first, st.second);
        history.push_back(execution{it->start, time, it->proc.name, stocks_now});
        process_executing.erase(it);
	}
}

void Simulator::simulate()
{
	time = 0;
	while(1)
	{
		checkRunningProcs();

		std::vector<Process> can_execute = executableProcesses();
		
		for (auto& p : can_execute)
			start_execution(p);
		
		if (can_execute.empty() && process_executing.empty())
			break;
		
		time++;
	}
}
std::vector<Process> Simulator::executableProcesses()
{
	std::vector<Process> res;
	for (auto& p : this->process_pending)
	{
		if (haveStocksFor(p))
			res.push_back(p);
	}
	return res;
}

void Simulator::checkRunningProcs()
{
	for(auto& p : this->process_executing)
	{
		if (this->time == (p.proc.delay + p.start))
			this->end_execution(p.proc.name);
	}
}