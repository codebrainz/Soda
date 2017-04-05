#pragma once

#include "Ast.h"
#include "CCode.h"

namespace Soda
{

    class Compiler;

    enum class CCodeTreeKind
    {
        HEADER = 1,
        SOURCE = 2,
        COMBINED = (HEADER | SOURCE),
    };

    CFilePtr generateCTree(
        Compiler &compiler, AstNode &node, CCodeTreeKind kind);

} // namespace Soda
