#pragma once

#include "Logger.h"
#include "SourceFile.h"
#include "SymbolTable.h"
#include "Tokenizer.h"
#include <iostream>
#include <vector>

namespace SODA
{

	class COMPILER
	{
	public:

		void AddFile(const std::string &fn)
		{
			SourceFiles.emplace_back(*this, fn);
		}

		void Tokenize()
		{
			Tokens.clear();
			for (auto &sourceFile : SourceFiles)
				sourceFile.Tokenize(Tokens);
		}

		const TOKEN_LIST &GetTokens() const
		{
			return Tokens;
		}

		TOKEN_LIST &GetTokens()
		{
			return Tokens;
		}

		SYMBOL_TABLE &GetGlobalScope() 
		{ 
			return GlobalScope; 
		}

		const SYMBOL_TABLE &GetGlobalScope() const 
		{ 
			return GlobalScope; 
		}

		template< class... Args >
		void Error(Args&&... args)
		{
			Logger.Error(std::forward<Args>(args)...);
		}

		template< class... Args >
		void Warning(Args&&... args)
		{
			Logger.Warning(std::forward<Args>(args)...);
		}

		template< class... Args >
		void Note(Args&&... args)
		{
			Logger.Note(std::forward<Args>(args)...);
		}

		template< class... Args >
		void Debug(Args&&... args)
		{
			Logger.Debug(std::forward<Args>(args)...);
		}

		bool ReportDiagnostics(std::ostream &os = std::cerr, size_t limit = size_t(-1))
		{
			return Logger.OutputDiagnostics(os, limit);
		}

	private:
		SOURCE_FILE_LIST SourceFiles;
		TOKEN_LIST Tokens;
		SYMBOL_TABLE GlobalScope;
		LOGGER Logger;
	};

} // namespace SODA