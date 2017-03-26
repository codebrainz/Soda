#include "Soda.h"
#include <iostream>

using namespace SODA;

int main()
{
	COMPILER compiler;

	compiler.AddFile("C:\\Projects\\Soda\\Soda\\test.soda");
	compiler.Tokenize();
	auto mod = ParseTokens(compiler, compiler.GetTokens());
	compiler.ReportDiagnostics(std::cerr);

	//for (auto &tok : compiler.GetTokens())
	//	std::cout << tok << std::endl;

	std::cout << "=================================\nPress enter to exit" << std::endl;
	std::cin.get();
	return 0;
}