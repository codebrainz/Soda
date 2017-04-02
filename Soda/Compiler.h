#pragma once

#include "Ast.h"
#include "Logger.h"
#include "NameMangler.h"
#include "Parser.h"
#include "ScopeBuilder.h"
#include "SourceFile.h"
#include "SymbolResolver.h"
#include "SymbolTable.h"
#include "Tokenizer.h"
#include <iostream>
#include <vector>

namespace Soda
{

	class Compiler
	{
	public:

		void addFile(const std::string &fn)
		{
			sourceFiles.emplace_back(*this, fn);
		}

		bool parse()
		{
			for (auto &sourceFile : sourceFiles)
			{
				sourceFile.tokenize(tokens);
				modules.emplace_back(parseTokens(*this, tokens));
			}
			return true;
		}

		unsigned int analyze()
		{
			unsigned int failures = 0;
			for (auto &mod : modules)
				failures += buildScopes(*this, *mod);
			for (auto &mod : modules)
				failures += resolveSymbols(*this, *mod);
			for (auto &mod : modules)
				mangleNames(*this, *mod);
			return failures;
		}

		const TokenList &getTokens() const
		{
			return tokens;
		}

		TokenList &getTokens()
		{
			return tokens;
		}

		SymbolTable &getGlobalScope() 
		{ 
			return globalScope; 
		}

		const SymbolTable &getGlobalScope() const 
		{ 
			return globalScope; 
		}

		AstModuleList &GetModules()
		{
			return modules;
		}

		const AstModuleList &GetModules() const
		{
			return modules;
		}

		template< class... Args >
		void error(Args&&... args)
		{
			logger.error(std::forward<Args>(args)...);
		}

		template< class... Args >
		void warning(Args&&... args)
		{
			logger.warning(std::forward<Args>(args)...);
		}

		template< class... Args >
		void note(Args&&... args)
		{
			logger.note(std::forward<Args>(args)...);
		}

		template< class... Args >
		void debug(Args&&... args)
		{
			logger.debug(std::forward<Args>(args)...);
		}

		bool reportDiagnostics(std::ostream &os = std::cerr, size_t limit = size_t(-1))
		{
			return logger.outputDiagnostics(os, limit);
		}

	private:
		SourceFileList sourceFiles;
		TokenList tokens;
		SymbolTable globalScope;
		AstModuleList modules;
		Logger logger;
	};

} // namespace Soda