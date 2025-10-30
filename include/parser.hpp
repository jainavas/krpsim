/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:57:53 by jainavas          #+#    #+#             */
/*   Updated: 2025/10/30 17:51:31 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

struct Process
{
	std::string name;
	std::map<std::string, int> requisites;
	std::map<std::string, int> produces;
	int delay;
};

class Parser
{
private:
	std::map<std::string, int> stock;
	std::vector<Process> processes;
	std::vector<std::string> optimizations;

public:
	void parse(std::string file);
	std::string extractBetweenParens(const std::string &str, size_t start);
	bool parseItemMap(const std::string &content, std::map<std::string, int> &out);
	bool parseProcessLine(const std::string &line, Process &process);
	bool parseOptimizeLine(const std::string &line, std::vector<std::string> &out);
	void printinfo()
	{
		std::cout << "Stocks\n";
		for (auto &pair : this->stock)
			std::cout << pair.first << ": " << pair.second << std::endl;
		std::cout << "Processes\n";
		for (auto &proc : this->processes)
		{
			std::cout << "=====================================\n";
			std::cout << proc.name << " needs:\n";
			for (auto &pair : proc.requisites)
				std::cout << pair.first << ": " << pair.second << std::endl;
			std::cout << proc.name << " produces:\n";
			for (auto &pair : proc.produces)
				std::cout << pair.first << ": " << pair.second << std::endl;
			std::cout << proc.name << " has a delay of: " << proc.delay << std::endl;
		}
	}
};

#endif