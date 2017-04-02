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
		compiler.parse();
		compiler.analyze();
		for (auto &mod : compiler.GetModules())
			generateDot(*mod, f);
	}
	else
	{
		compiler.parse();
		compiler.analyze();
		for (auto &mod : compiler.GetModules())
			generateDot(*mod, std::cout);
	}

	compiler.reportDiagnostics(std::cerr);

	std::cerr << "=================================\nPress enter to exit" << std::endl;
	std::cin.get();
	return 0;
}