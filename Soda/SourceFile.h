#pragma once

#include "Tokenizer.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SODA
{

	class COMPILER;

	class SOURCE_FILE
	{
	public:
		SOURCE_FILE(COMPILER &compiler, const std::string &fn);
		~SOURCE_FILE();

		COMPILER &GetCompiler();
		const COMPILER &GetCompiler() const;
		const std::string &GetFileName() const;
		size_t GetSize() const;
		void GetPosition(size_t inputOffset, size_t &outputLine, size_t &outputColumn) const;

		size_t GetLine(size_t pos) const
		{
			size_t line = 0, column = 0;
			GetPosition(pos, line, column);
			return line;
		}

		size_t GetColumn(size_t pos) const
		{
			size_t line = 0, column = 0;
			GetPosition(pos, line, column);
			return column;
		}
		
		uint8_t &operator[](size_t off);
		const uint8_t &operator[](size_t off) const;

		bool Tokenize(TOKEN_LIST &tokenList)
		{
			return TokenizeFile(GetCompiler(), *this, tokenList);
		}

	private:
		struct PRIVATE;
		PRIVATE *m_Impl;
	};

	typedef std::vector<SOURCE_FILE> SOURCE_FILE_LIST;

} // namespace Soda