#pragma once

#include <ostream>

namespace Soda
{
    struct AstNode;

    void generateDot(AstNode &node, std::ostream &os);

} // namespace Soda
