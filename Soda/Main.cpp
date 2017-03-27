#include "Soda.h"
#include <iostream>
#include <fstream>

using namespace Soda;

int main(int argc, char **argv)
{
	Compiler compiler;
	Options options(argc, argv);

	for (auto &fileName : options.inputFiles)
		compiler.addFile(fileName);

	if (!options.outputFile.empty())
	{
		std::ofstream f(options.outputFile);
		for (auto &mod : compiler.parse())
			generateDot(*mod, f);
	}
	else
	{
		for (auto &mod : compiler.parse())
			generateDot(*mod, std::cout);
	}

	compiler.reportDiagnostics(std::cerr);

	std::cerr << "=================================\nPress enter to exit" << std::endl;
	std::cin.get();
	return 0;
}