#!/usr/bin/env python3

import sys

def main(args):
	lines = []
	with open(args[1]) as inputFile:
		lines = inputFile.readlines()
	outputLines = []
	for line in lines:
		line = line.strip()
		if line.startswith('//>'):
			line = line[4:]
			outputLines.append(line + '\n')
			if line.lstrip().startswith(';'):
				outputLines.append('\n')
	for line in outputLines:
		sys.stdout.write(line)
	return 0

if __name__ == "__main__":
	sys.exit(main(sys.argv))
