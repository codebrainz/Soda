#pragma once

#include "Tokenizer.h"
#include <memory>

namespace Soda
{

    class Compiler;
    struct AstModule;

    std::unique_ptr< AstModule > parseTokens(
        Compiler &compiler, TokenList &tokenList);

} // namespace Soda
