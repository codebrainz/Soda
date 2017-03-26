#pragma once

#include "SourceFile.h"
#include "Tokenizer.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SODA
{

	struct DIAGNOSTIC_POSITION
	{
		size_t Position, Line, Column;
		DIAGNOSTIC_POSITION(size_t position = 0,
			                size_t line = size_t(-1), size_t column = size_t(-1))
			: Position(position), Line(line), Column(column)
		{
		}
		bool IsResolved() const
		{
			return (Line != size_t(-1) && Column != size_t(-1));
		}
		void ResolvePosition(SOURCE_FILE &sourceFile)
		{
			sourceFile.GetPosition(Position, Line, Column);
		}
	};

	struct DIAGNOSTIC_LOCATION
	{
		SOURCE_FILE &File;
		DIAGNOSTIC_POSITION Start;
		DIAGNOSTIC_POSITION End;
		DIAGNOSTIC_LOCATION(SOURCE_FILE &file, size_t start = 0, size_t end = 0)
			: File(file), Start(start), End(end) {}
		bool IsResolved() const
		{
			return (Start.IsResolved() && End.IsResolved());
		}
		void ResolvePosition()
		{
			Start.ResolvePosition(File);
			End.ResolvePosition(File);
		}
		void GetLine(std::string &line) const
		{
			auto lineStartOffset = Start.Position;
			while (File[lineStartOffset--] != '\n') { ; }
			auto lineEndOffset = lineStartOffset;
			while (File[lineEndOffset++] != '\n') { ; }
			auto lineLength = lineEndOffset - lineStartOffset;
			line.clear();
			line.reserve(lineLength);
			for (auto i = lineStartOffset; i < lineEndOffset; i++)
				line += static_cast<char>(File[i]);
		}
	};

	enum DIAGNOSTIC_CATEGORY
	{
		DC_ERROR,
		DC_WARNING,
		DC_NOTE,
		DC_DEBUG,
	};

	class DIAGNOSTIC
	{
	public:
		DIAGNOSTIC_CATEGORY Category;
		DIAGNOSTIC_LOCATION Location;
		std::string Message;

		DIAGNOSTIC(DIAGNOSTIC_CATEGORY cat, DIAGNOSTIC_LOCATION loc, std::string msg)
			: Category(cat), Location(std::move(loc)), Message(std::move(msg)) {}

		void GetLine(std::string &line) const
		{
			Location.GetLine(line);
		}

		friend std::ostream &operator<<(std::ostream &os, const DIAGNOSTIC &diag)
		{
			std::string cat;
			switch (diag.Category)
			{
			case DC_ERROR:   cat = "error"; break;
			case DC_WARNING: cat = "warning"; break;
			case DC_NOTE:    cat = "note"; break;
			case DC_DEBUG:   cat = "debug"; break;
			}
			os << diag.Location.File.GetFileName() << ":" 
				<< diag.Location.Start.Line << ":" << diag.Location.Start.Column << ": "
				<< cat << ": " << diag.Message << '\n';
			return os;
		}

		void ResolvePosition()
		{
			if (!Location.IsResolved())
				Location.ResolvePosition();
		}

	private:
	};

	typedef std::vector<DIAGNOSTIC> DIAGNOSTIC_LIST;

	class LOGGER
	{
	public:

		template< class... Args >
		void Log(DIAGNOSTIC_CATEGORY cat, DIAGNOSTIC_LOCATION loc, Args&&... args)
		{
			std::stringstream ss;
			Format(ss, std::forward<Args>(args)...);
			DIAGNOSTIC diag(cat, std::move(loc), ss.str());
			diag.ResolvePosition();
			Diagnostics.push_back(std::move(diag));
		}

		template< class... Args >
		void Log(DIAGNOSTIC_CATEGORY cat, TOKEN &token, Args&&... args)
		{
			Log(cat, DIAGNOSTIC_LOCATION(token.File, token.Start, token.End), std::forward<Args>(args)...);
		}

		template< class... Args >
		void Error(Args&&... args)
		{
			Log(DC_ERROR, std::forward<Args>(args)...);
		}

		template< class... Args >
		void Warning(Args&&... args)
		{
			Log(DC_WARNING, std::forward<Args>(args)...);
		}
		
		template< class... Args >
		void Note(Args&&... args)
		{
			Log(DC_NOTE, std::forward<Args>(args)...);
		}

		template< class... Args >
		void Debug(Args&&... args)
		{
			Log(DC_DEBUG, std::forward<Args>(args)...);
		}

		bool HaveMessages() const 
		{ 
			return !Diagnostics.empty(); 
		}

		bool OutputDiagnostics(std::ostream &os, size_t limit = size_t(-1))
		{
			auto errors = 0u;
			auto cnt = 0u;
			for (auto &diag : Diagnostics)
			{
				if (diag.Category == DC_ERROR)
					errors++;
				os << diag << '\n';
				cnt++;
				if (cnt == limit)
					break;
			}
			return (errors == 0);
		}

	private:
		DIAGNOSTIC_LIST Diagnostics;
		
		void Format(std::ostream &os, const char *s)
		{
			while (*s)
			{
				if (*s == '%')
				{
					if (*(s + 1) == '%')
						++s;
					else
						throw std::runtime_error("invalid format string: missing argument");
				}
				os << *s++;
			}
		}

		template< class T, class... Args >
		void Format(std::ostream &os, const char *s, const T &value, Args&&... args)
		{
			while (*s)
			{
				if (*s == '%')
				{
					if (*(s + 1) == '%')
						++s;
					else
					{
						os << value;
						++s;
						Format(os, s, std::forward<Args>(args)...);
						return;
					}
				}
				os << *s++;
			}
		}
	};

} // namespade SODA