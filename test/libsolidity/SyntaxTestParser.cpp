/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SyntaxTestParser.h"
#include <fstream>
#include <stdexcept>
#include <boost/throw_exception.hpp>

namespace dev
{
namespace solidity
{
namespace test
{

SyntaxTest SyntaxTestParser::parse(std::string const& _filename)
{
	std::ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(std::runtime_error("cannot open test contract: \"" + _filename + "\""));
	file.exceptions(std::ios::badbit);

	SyntaxTest result;
	std::string line;
	std::string const delimiter("// ----");
	while (std::getline(file, line))
	{
		if (!line.compare(0, delimiter.size(), delimiter))
			break;
		else
			result.source += line + "\n";
	}
	while (std::getline(file, line))
	{
		auto it = line.begin();

		while (it != line.end() && (std::isspace(*it) || *it == '/'))
			it++;

		if (it == line.end()) continue;

		auto type_begin = it;
		while (it != line.end() && *it != ':')
			it++;
		std::string errorType(type_begin, it);

		// skip colon
		if (it != line.end()) it++;

		while (it != line.end() && std::isspace(*it))
			it++;

		std::string errorMessage(it, line.end());
		result.expectations.emplace_back(errorType, std::move(errorMessage));
	}

	return result;
}


}
}
}
