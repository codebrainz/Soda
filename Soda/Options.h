#pragma once

#include <string>
#include <vector>

namespace Soda
{

	class Options
	{
	public:
		std::vector<std::string> args;
		std::string programName;
		std::vector<std::string> inputFiles;
		std::string outputFile;
		Options(int argc, char **argv);
	private:
		void parse(int argc, char **argv);
		void printHelp(bool doExit = true);
	};

} // namespace Soda