/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:54:55 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/02 23:14:21 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/simulator.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: ./krpsim \"file\"\n";
		return 1;
	}
	else
	{
		Parser p;
		p.parse(argv[1]);
		std::cout << "== Stocks iniciales ==\n";
		for (auto &kv : p.getStocks())
			std::cout << kv.first << ": " << kv.second << "\n";

		// Lanzar simulador
		Simulator sim(p);
		sim.simulate();

		// Trazar lo que ha hecho
		std::cout << "\n== Trazas de ejecución (fin de procesos) ==\n";
		for (const auto &e : sim.getHistory())
		{
			std::cout << "t=" << e.end
					  << ": finish '" << e.process_name
					  << "' (start=" << e.start
					  << ", dur=" << (e.end - e.start) << ")\n";
			std::cout << "  stocks: ";
			bool first = true;
			for (const auto &s : e.stocks_snapshot)
			{
				if (!first)
					std::cout << ", ";
				std::cout << s.first << "=" << s.second;
				first = false;
			}
			std::cout << "\n";
		}

		// Resultado final
		std::cout << "\n== Resultado final ==\n";
		std::cout << "Tiempo total: " << sim.getCurrentTime() << "\n";
		std::cout << "Stocks finales: \n";
		for (const auto &kv : sim.getStocksNow())
			std::cout << "  " << kv.first << ": " << kv.second << "\n";
	}
}