#pragma once

#include "Ast.h"
#include "Logger.h"
#include "Parser.h"
#include "SourceFile.h"
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

		void extendTokens(TokenList &toks)
		{
			tokens.reserve(tokens.size() + toks.size());
			for (auto &tok : toks)
				tokens.push_back(std::move(tok));
		}

		std::vector<std::unique_ptr<AstModule>> parse()
		{
			std::vector<std::unique_ptr<AstModule>> modules;
			for (auto &sourceFile : sourceFiles)
			{
				TokenList tokens;
				sourceFile.tokenize(tokens);
				modules.emplace_back(parseTokens(*this, tokens));
				extendTokens(tokens);
			}
			return modules;
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
		Logger logger;
	};

} // namespace Soda