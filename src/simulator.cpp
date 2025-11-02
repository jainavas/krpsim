/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulator.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 18:12:17 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/02 22:15:17 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/simulator.hpp"

bool Simulator::haveStocksFor(Process to_do)
{
	for (const auto& [resource, amount_needed] : to_do.requisites)
    {
        auto stock_it = stocks_now.find(resource);
        if (stock_it != stocks_now.end())
			if (stock_it->second - amount_needed < 0)
				return false;
        else
            return false;
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
    process_executing.emplace_back(time, to_do);

    return true;
}

void Simulator::end_execution(std::string process_n)
{
	auto it = std::find_if(process_executing.begin(), process_executing.end(),
        [&](const Process& p) { return p.name == process_n; });

    if (it != process_executing.end())
	{
		for (auto& st : it->second.produces)
			this->addStocks(st.first, st.second);
        process_executing.erase(it);
	}
}

void Simulator::simulate()
{
	time = 0;
	while(1)
	{
		//
	}
}
