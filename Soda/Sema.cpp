#include "Sema.h"
#include "Ast.h"
#include "Compiler.h"
#include "Visitor.h"

namespace Soda
{

    struct Sema1 : public AstDefaultVisitor
    {
        Compiler &compiler;
        Sema1(Compiler &compiler)
            : compiler(compiler)
        {
        }
    };

    // cases only in switches
    // break only in cases or loops
    // continue only in loops
    // gotos are local
    // void return with value
    // return value from void function
    // if/loop conditions are compatible with boolean
    // only one destructor per struct

    bool analyzeSemantics(Compiler &compiler, AstNode &node)
    {
        Sema1 pass1(compiler);
        node.accept(pass1);
        return true;
    }

} // namespace Soda
