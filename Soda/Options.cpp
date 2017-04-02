#include "Options.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
# define SODA_WIN32
#endif

namespace Soda
{

	enum OptionKind
	{
		OK_HELP,
		OK_OUTPUT,
	};

	static bool argumentMatches(OptionKind kind, const std::string &optName)
	{
		bool result = false;
		switch (kind)
		{
		case OK_HELP:
#ifdef SODA_WIN32
			if (optName == "/h" || optName == "/H" || optName == "/?" || optName == "/help" || optName == "/HELP")
				result = true;
#else
			if (optName == "-h" || optName == "--help")
				result = true;
#endif
			break;
		case OK_OUTPUT:
#ifdef SODA_WIN32
			if (optName == "/o" || optName == "/O" || optName == "/output" || optName == "/OUTPUT")
				result = true;
#else
			if (optName == "-o" || optName == "--output")
				result = true;
#endif
			break;
		}
		return result;
	}

	Options::Options(int argc, char ** argv)
	{
		parse(argc, argv);
	}

	void Options::parse(int argc, char **argv)
	{
		assert(argc > 0 && argv);

		programName = argv[0];

		for (int i = 1; i < argc; i++)
			args.emplace_back(argv[i]);
		
		for (size_t i = 0; i < args.size(); i++)
		{
			const auto &thisArg = args[i];
			if (argumentMatches(OK_HELP, thisArg))
			{
				printHelp();
			}
			else if (argumentMatches(OK_OUTPUT, thisArg))
			{
				if (i < (args.size() - 1))
				{
					const auto &nextArg = args[i + 1];
					if (outputFile.empty())
					{
						outputFile = nextArg;
						i++;
					}
					else
					{
						std::cerr << "error: output file specified multiple times." << std::endl;
						std::exit(EXIT_FAILURE);
					}
				}
				else
				{
					std::cerr << "error: missing output file argument." << std::endl;
					std::exit(EXIT_FAILURE);
				}
			}
			else
			{
				inputFiles.emplace_back(thisArg);
			}
		}
	}

	void Options::printHelp(bool doExit)
	{
		std::cout <<
			"Usage: soda [options] input_files...\n"
			"\n"
			"Soda source file compiler.\n"
			"\n"
			"Options:\n"
#ifdef SODA_WIN32
			"  /?  Show this message and exit.\n"
			"  /O  Write output to this file.\n"
#else
			"  -h, --help      Show this message and exit.\n"
			"  -o, --output    Write output to this file.\n"
#endif
			"  input_files...  Input source files.\n"
			"\n"
			"Written and maintained by Matthew Brush <mbrush@codebrainz.ca>.\n";
		if (doExit)
			std::exit(EXIT_SUCCESS);
	}

} // namespace Soda