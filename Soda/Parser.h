#pragma once

#include "Ast.h"
#include "Tokenizer.h"
#include <memory>

namespace Soda
{

    class Compiler;

    AstModulePtr parseTokens(Compiler &compiler, TokenList &tokenList);

} // namespace Soda
