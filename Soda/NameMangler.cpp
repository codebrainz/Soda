#include "Ast.h"
#include "Compiler.h"
#include "NameMangler.h"
#include "Visitor.h"
#include <cassert>
#include <string>
#include <vector>

namespace Soda
{

    struct NameMangler : public AstDefaultVisitor
    {
        Compiler &compiler;
        std::vector< std::string > nameStack;
        size_t nameCnt;

        NameMangler(Compiler &compiler)
            : compiler(compiler)
            , nameCnt(0)
        {
        }

        std::string generateName(const std::string &prefix = "anon")
        {
            auto id = nameCnt++;
            return prefix + std::to_string(id);
        }

        std::string mangleDottedName(const std::string &dottedName)
        {
            std::string name;
            name.reserve(dottedName.size());
            for (auto &ch : dottedName) {
                if (ch != '.')
                    name += ch;
                else
                    name += '_';
            }
            return name;
        }

        void mangleName(AstDecl &n)
        {
            n.mangledName.clear();
            for (size_t i = 0; i < nameStack.size(); i++)
                n.mangledName += nameStack[i] + '_';
            n.mangledName += mangleDottedName(n.name);
        }

        virtual void visit(AstBlockStmt &n)
        {
            nameStack.push_back(generateName());
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstIfStmt &n)
        {
            nameStack.push_back(generateName());
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstCaseStmt &n)
        {
            nameStack.push_back(generateName());
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstSwitchStmt &n)
        {
            nameStack.push_back(generateName());
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstForStmt &n)
        {
            nameStack.push_back(generateName());
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstTypedef &n)
        {
            mangleName(n);
        }

        virtual void visit(AstNamespaceDecl &n)
        {
            mangleName(n);
            nameStack.push_back(std::move(mangleDottedName(n.name)));
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstVarDecl &n)
        {
            mangleName(n);
        }

        virtual void visit(AstParamDecl &n)
        {
            mangleName(n);
        }

        virtual void visit(AstFuncDecl &n)
        {
            mangleName(n);
            nameStack.push_back(n.name);
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstStructDecl &n)
        {
            mangleName(n);
            nameStack.push_back(n.name);
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstEnumeratorDecl &n)
        {
            mangleName(n);
        }

        virtual void visit(AstEnumDecl &n)
        {
            mangleName(n);
            nameStack.push_back(n.name);
            n.acceptChildren(*this);
            nameStack.pop_back();
        }

        virtual void visit(AstModule &n)
        {
            //nameStack.push_back(std::move(n.identifierName()));
            n.acceptChildren(*this);
            //nameStack.pop_back();
        }
    };

    bool mangleNames(Compiler &compiler, AstNode &node)
    {
        NameMangler mangler(compiler);
        node.accept(mangler);
        return true;
    }

} // namespace Soda
