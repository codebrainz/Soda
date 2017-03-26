#include "Soda.h"
#include <iostream>

using namespace Soda;

int main()
{
	Compiler compiler;

	compiler.addFile("C:\\Projects\\Soda\\Soda\\test.soda");
	compiler.tokenize();
	auto mod = parseTokens(compiler, compiler.getTokens());
	compiler.reportDiagnostics(std::cerr);

	//for (auto &tok : compiler.getTokens())
	//	std::cout << tok << std::endl;

	std::cout << "=================================\nPress enter to exit" << std::endl;
	std::cin.get();
	return 0;
}