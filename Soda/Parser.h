#pragma once

#include "Tokenizer.h"
#include <memory>

namespace SODA
{

	class COMPILER;
	struct AST_MODULE;

	std::unique_ptr<AST_MODULE> ParseTokens(COMPILER &compiler, TOKEN_LIST &tokenList);

} // namespace SODA