#pragma once

namespace Soda
{

	struct AstNode;
	class Compiler;

	bool mangleNames(Compiler &compiler, AstNode &node);

} // namespace Soda