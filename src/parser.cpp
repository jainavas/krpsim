/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jainavas <jainavas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/30 15:57:31 by jainavas          #+#    #+#             */
/*   Updated: 2025/10/30 17:52:36 by jainavas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/parser.hpp"

std::vector<std::string> split(const std::string &str, char delim)
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim))
	{
		tokens.push_back(item);
	}

	return tokens;
}

void Parser::parse(std::string file)
{
	std::ifstream filestream(file);
	std::string line;
	std::vector<std::string> splited;
	while (std::getline(filestream, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		if (line.find("optimize:") == 0) // Empieza con "optimize:"
		{
			parseOptimizeLine(line, this->optimizations);
		}
		else if (line.find_first_of(':') == line.find_last_of(':'))
		{
			splited = split(line, ':');
			this->stock.insert(std::pair(splited[0], std::atoi(splited[1].c_str())));
		}
		else
		{
			Process p;
			if (!parseProcessLine(line, p))
			{
				std::cerr << "Error parseando: " << line << "\n";
				continue;
			}
			this->processes.push_back(p);
		}
	}
}

std::string Parser::extractBetweenParens(const std::string &str, size_t start)
{
	size_t openPos = str.find('(', start);
	if (openPos == std::string::npos)
		return "";

	size_t closePos = str.find(')', openPos);
	if (closePos == std::string::npos)
		return "";

	return str.substr(openPos + 1, closePos - openPos - 1);
}

// Helper: parsea "item:qty;item2:qty2" en un map
// Retorna false si encuentra error de formato
bool Parser::parseItemMap(const std::string &content, std::map<std::string, int> &out)
{
	if (content.empty())
		return false;

	std::stringstream ss(content);
	std::string pair;

	while (std::getline(ss, pair, ';'))
	{
		size_t colonPos = pair.find(':');

		// Validar que hay exactamente un ':' y que los dos lados existen
		if (colonPos == std::string::npos || colonPos == 0 || colonPos == pair.length() - 1)
			return false;

		std::string item = pair.substr(0, colonPos);
		std::string qtyStr = pair.substr(colonPos + 1);

		// Validar que qty es un número válido
		try
		{
			int qty = std::stoi(qtyStr);
			out[item] = qty;
		}
		catch (...)
		{
			return false;
		}
	}

	return true;
}

// Main parser de línea de proceso
bool Parser::parseProcessLine(const std::string &line, Process &process)
{
	// Format: name:(needs):(results):delay

	size_t firstColon = line.find(':');
	if (firstColon == std::string::npos)
		return false;

	process.name = line.substr(0, firstColon);

	// Extraer needs (primera sección entre paréntesis)
	std::string needsStr = extractBetweenParens(line, firstColon);
	if (needsStr.empty())
		return false;

	if (!parseItemMap(needsStr, process.requisites))
		return false;

	// Encontrar fin del primer paréntesis para empezar a buscar results
	size_t firstClosePos = line.find(')', firstColon);
	if (firstClosePos == std::string::npos)
		return false;

	// Extraer results (segunda sección entre paréntesis)
	std::string resultsStr = extractBetweenParens(line, firstClosePos);
	if (resultsStr.empty())
		return false;

	if (!parseItemMap(resultsStr, process.produces))
		return false;

	// Extraer delay (número después del último paréntesis)
	size_t secondClosePos = line.find(')', firstClosePos + 1);
	if (secondClosePos == std::string::npos)
		return false;

	size_t delayStart = line.find(':', secondClosePos);
	if (delayStart == std::string::npos)
		return false;

	std::string delayStr = line.substr(delayStart + 1);

	try
	{
		process.delay = std::stoi(delayStr);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool Parser::parseOptimizeLine(const std::string& line, std::vector<std::string>& out)
{
    std::string content = extractBetweenParens(line, 0);
    if (content.empty())
        return false;
    
    std::stringstream ss(content);
    std::string item;
    while (std::getline(ss, item, ';'))
    {
        out.push_back(item);
    }
    return true;
}
