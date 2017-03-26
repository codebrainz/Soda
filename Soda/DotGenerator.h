#pragma once

#include "Ast.h"
#include <memory>
#include <ostream>

namespace SODA
{

	void generateDot(std::unique_ptr<AST_MODULE> &m, std::ostream &os);

} // namespace SODA