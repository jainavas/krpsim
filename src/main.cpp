/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 17:54:55 by jainavas          #+#    #+#             */
/*   Updated: 2025/11/03 01:58:36 by jainavas         ###   ########.fr       */
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
    
    Parser p;
    p.parse(argv[1]);
    
    std::cout << "== Stocks iniciales ==\n";
    for (auto &kv : p.getStocks())
        std::cout << kv.first << ": " << kv.second << "\n";

    // Lanzar simulador
    Simulator sim(p);
    sim.simulate();

    // Resultado
    std::cout << "\n== Resultado final ==\n";
    std::cout << "Tiempo total: " << sim.getCurrentTime() << "\n";
    std::cout << "Stocks finales:\n";
    for (const auto &kv : sim.getStocksNow())
        std::cout << "  " << kv.first << ": " << kv.second << "\n";
    
    return 0;
}