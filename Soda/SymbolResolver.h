#pragma once

namespace Soda
{

    struct AstNode;
    class Compiler;

    unsigned int resolveSymbols(Compiler &compiler, AstNode &node);

} // namespace Soda
