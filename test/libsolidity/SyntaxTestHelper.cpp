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

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <test/TestHelper.h>
#include <test/libsolidity/AnalysisFramework.h>
#include <test/libsolidity/SyntaxTestHelper.h>
#include <test/libsolidity/SyntaxTestParser.h>

namespace dev
{
namespace solidity
{
namespace test
{

class SyntaxTester : public AnalysisFramework
{
public:
	void runSyntaxTest(SyntaxTest const& _test)
	{
		std::vector<std::string> unexpectedErrors;
		auto expectations = _test.expectations;
		auto errorList = parseAnalyseAndReturnError(_test.source, true, true, true).second;

		// checks for presence of errors and warnings with multiplicities
		// could be modified to enforce the precise order as well
		for (auto const& error: errorList)
		{
			std::string msg = errorMessage(*error);
			bool found = false;
			for (auto it = expectations.begin(); it != expectations.end(); ++it)
				if (msg.find(it->second) != std::string::npos && error->typeName() == it->first)
				{
					found = true;
					expectations.erase(it);
					break;
				}
			if (!found)
				unexpectedErrors.emplace_back(error->typeName() + ": " + msg);
		}

		if (!unexpectedErrors.empty())
		{
			std::string msg = "Unexpected error(s):\n";
			for (auto const& error: unexpectedErrors)
				msg += error + "\n";
			BOOST_ERROR(msg);
		}

		if (!expectations.empty())
		{
			std::string msg = "Expected error(s) not present:\n";
			for (auto const& expectation: expectations)
				msg += expectation.first + ": " + expectation.second + "\n";
			BOOST_ERROR(msg);
		}
	}
private:
	static std::string errorMessage(Error const& _e)
	{
		return _e.comment() ? *_e.comment() : "NONE";
	}

};

int registerSyntaxTests(
	boost::unit_test::test_suite& _suite,
	boost::filesystem::path const& _basepath,
	boost::filesystem::path const& _path
)
{
	static SyntaxTestParser syntaxTestParser;
	static SyntaxTester syntaxTester;

	int numTestsAdded = 0;
	boost::filesystem::path fullpath = _basepath / _path;
	if (boost::filesystem::is_directory(fullpath))
	{
		boost::unit_test::test_suite* sub_suite = BOOST_TEST_SUITE(_path.filename().string());
		for (auto &entry : boost::filesystem::directory_iterator(fullpath))
			numTestsAdded += registerSyntaxTests(*sub_suite, _basepath, _path / entry.path().filename());
		_suite.add(sub_suite);
	}
	else
	{
		_suite.add(boost::unit_test::make_test_case(
			[fullpath] { syntaxTester.runSyntaxTest(syntaxTestParser.parse(fullpath.string())); },
			_path.stem().string(),
			_path.string(),
			0
		));
		numTestsAdded = 1;
	}
	return numTestsAdded;
}

void registerSyntaxTests()
{
	boost::filesystem::path testPath;
	if (!dev::test::Options::get().testPath.empty())
		testPath = boost::filesystem::path(dev::test::Options::get().testPath);
	else
	{
		auto search_path =
		{
			// assuming call from source tree root
			boost::filesystem::current_path() / "test" ,
			// assuming call from build/test
			boost::filesystem::current_path() / ".." / ".." / "test",
			// assuming call from build/
			boost::filesystem::current_path() / ".." / "test",
			// assuming call from test/
			boost::filesystem::current_path(),
			// assuming call from test/libsolidity
			boost::filesystem::current_path()
		};
		for (auto &path: search_path)
		{
			auto syntaxTestPath = path / "libsolidity" / "syntaxTests";
			if (boost::filesystem::exists(syntaxTestPath) && boost::filesystem::is_directory(syntaxTestPath))
			{
				testPath = path;
				break;
			}
		}
	}

	if (boost::filesystem::exists(testPath) && boost::filesystem::is_directory(testPath))
	{
		int numTestsAdded = registerSyntaxTests(
			boost::unit_test::framework::master_test_suite(),
			testPath / "libsolidity",
			"syntaxTests"
		);
		solAssert(numTestsAdded > 0, "no syntax tests found in libsolidity/syntaxTests");
	}
	else
		solAssert(false, "libsolidity/syntaxTests directory not found");
}

}
}
}
