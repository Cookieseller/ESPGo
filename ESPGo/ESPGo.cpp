// ESPGo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("No filename found");
		return 1;
	}

	CDemoFile demoFile;
	if (!demoFile.Open(argv[1]))
		return 1;
	
	CFileParser fileParser;
	fileParser.ParseDemoFile(demoFile);

	getchar();

    return 0;
}

