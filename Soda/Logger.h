#pragma once

#include "Ast.h"
#include "SourceFile.h"
#include "Tokenizer.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Soda
{

	struct DiagnosticPosition
	{
		size_t position, line, column;
		DiagnosticPosition(size_t position = 0,
			                size_t line = size_t(-1), size_t column = size_t(-1))
			: position(position), line(line), column(column)
		{
		}
		bool isResolved() const
		{
			return (line != size_t(-1) && column != size_t(-1));
		}
		void resolvePosition(SourceFile &sourceFile)
		{
			sourceFile.getPosition(position, line, column);
		}
	};

	struct DiagnosticLocation
	{
		SourceFile &file;
		DiagnosticPosition start;
		DiagnosticPosition end;
		DiagnosticLocation(SourceFile &file, size_t start = 0, size_t end = 0)
			: file(file), start(start), end(end) {}
		bool isResolved() const
		{
			return (start.isResolved() && end.isResolved());
		}
		void resolvePosition()
		{
			start.resolvePosition(file);
			end.resolvePosition(file);
		}
		void getLine(std::string &line) const
		{
			auto lineStartOffset = start.position;
			while (file[lineStartOffset--] != '\n') { ; }
			auto lineEndOffset = lineStartOffset;
			while (file[lineEndOffset++] != '\n') { ; }
			auto lineLength = lineEndOffset - lineStartOffset;
			line.clear();
			line.reserve(lineLength);
			for (auto i = lineStartOffset; i < lineEndOffset; i++)
				line += static_cast<char>(file[i]);
		}
	};

	enum DiagnosticCategory
	{
		DC_ERROR,
		DC_WARNING,
		DC_NOTE,
		DC_DEBUG,
	};

	class Diagnostic
	{
	public:
		DiagnosticCategory category;
		DiagnosticLocation location;
		std::string message;

		Diagnostic(DiagnosticCategory cat, DiagnosticLocation loc, std::string msg)
			: category(cat), location(std::move(loc)), message(std::move(msg)) {}

		void getLine(std::string &line) const
		{
			location.getLine(line);
		}

		friend std::ostream &operator<<(std::ostream &os, const Diagnostic &diag)
		{
			std::string cat;
			switch (diag.category)
			{
			case DC_ERROR:   cat = "error"; break;
			case DC_WARNING: cat = "warning"; break;
			case DC_NOTE:    cat = "note"; break;
			case DC_DEBUG:   cat = "debug"; break;
			}
			os << diag.location.file.getFileName() << ":" 
				<< diag.location.start.line << ":" << diag.location.start.column << ": "
				<< cat << ": " << diag.message << '\n';
			return os;
		}

		void resolvePosition()
		{
			if (!location.isResolved())
				location.resolvePosition();
		}

	private:
	};

	typedef std::vector<Diagnostic> DiagnosticList;

	class Logger
	{
	public:

		template< class... Args >
		void log(DiagnosticCategory cat, DiagnosticLocation loc, Args&&... args)
		{
			std::stringstream ss;
			format(ss, std::forward<Args>(args)...);
			Diagnostic diag(cat, std::move(loc), ss.str());
			diag.resolvePosition();
			diagnostics.push_back(std::move(diag));
		}

		template< class... Args >
		void log(DiagnosticCategory cat, Token &token, Args&&... args)
		{
			log(cat, DiagnosticLocation(token.file, token.start, token.end), std::forward<Args>(args)...);
		}

		template< class... Args >
		void log(DiagnosticCategory cat, AstNode &node, Args&&... args)
		{
			log(cat, *node.start, std::forward<Args>(args)...);
		}

		template< class... Args >
		void error(Args&&... args)
		{
			log(DC_ERROR, std::forward<Args>(args)...);
		}

		template< class... Args >
		void warning(Args&&... args)
		{
			log(DC_WARNING, std::forward<Args>(args)...);
		}
		
		template< class... Args >
		void note(Args&&... args)
		{
			log(DC_NOTE, std::forward<Args>(args)...);
		}

		template< class... Args >
		void debug(Args&&... args)
		{
			log(DC_DEBUG, std::forward<Args>(args)...);
		}

		bool haveMessages() const 
		{ 
			return !diagnostics.empty(); 
		}

		bool outputDiagnostics(std::ostream &os, size_t limit = size_t(-1))
		{
			auto errors = 0u;
			auto cnt = 0u;
			for (auto &diag : diagnostics)
			{
				if (diag.category == DC_ERROR)
					errors++;
				os << diag << '\n';
				cnt++;
				if (cnt == limit)
					break;
			}
			return (errors == 0);
		}

	private:
		DiagnosticList diagnostics;
		
		void format(std::ostream &os, const char *s)
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
		void format(std::ostream &os, const char *s, const T &value, Args&&... args)
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
						format(os, s, std::forward<Args>(args)...);
						return;
					}
				}
				os << *s++;
			}
		}
	};

} // namespade Soda