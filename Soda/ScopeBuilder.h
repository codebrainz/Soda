#pragma once

namespace Soda
{

    struct AstNode;
    class Compiler;

    unsigned int buildScopes(Compiler &compiler, AstNode &node);

} // namespace Soda
