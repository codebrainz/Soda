#pragma once

#include "Ast.h"
#include <memory>
#include <ostream>

namespace Soda
{

	void generateDot(std::unique_ptr<AstModule> &m, std::ostream &os);

} // namespace Soda