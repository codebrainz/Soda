#pragma once

#include "Tokenizer.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Soda
{

	class Compiler;

	class SourceFile
	{
	public:
		SourceFile(Compiler &compiler, const std::string &fn);
		~SourceFile();

		Compiler &getCompiler();
		const Compiler &getCompiler() const;
		const std::string &getFileName() const;
		size_t getSize() const;
		void getPosition(size_t inputOffset, size_t &outputLine, size_t &outputColumn) const;

		size_t getLine(size_t pos) const
		{
			size_t line = 0, column = 0;
			getPosition(pos, line, column);
			return line;
		}

		size_t getColumn(size_t pos) const
		{
			size_t line = 0, column = 0;
			getPosition(pos, line, column);
			return column;
		}
		
		uint8_t &operator[](size_t off);
		const uint8_t &operator[](size_t off) const;

		bool tokenize(TokenList &tokenList)
		{
			return tokenizeFile(getCompiler(), *this, tokenList);
		}

	private:
		struct Private;
		Private *m_Impl;
	};

	typedef std::vector<SourceFile> SourceFileList;

} // namespace Soda