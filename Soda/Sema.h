#pragma once

namespace Soda
{

    class Compiler;
    struct AstNode;

    bool analyzeSemantics(Compiler &compiler, AstNode &node);

} // namespace Soda
