//g++ -o track-convert -Wall -std=c++11 tinyxml2.cpp trkconv.cpp main.cpp

#include "trkconv.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <getopt.h>


typedef struct InputParms
{
	InputParms(string file) : infile(file) {}
	string infile = "";
	string outfile = "";
	int outtype = 4; // kml
	bool verbose = false;
} InputParms;


void print_usage()
{
	cout << "Usage: track-convert [options] infilename\n";
	cout << "Allowed options:\n";
	cout << "  -o [ --outfile ] arg  output filename\n";
	cout << "  -t [ --outtype ] arg  output type (1=CSV, 2=GPX, 3=TCX, 4=KML)\n";
	cout << "  -v [ --verbose ]      print input details\n";
	exit(EXIT_FAILURE);
}


void valid_input(const InputParms &in)
{
	if (in.verbose)
	{
		cout << "Using parameters:\n";
		cout << "Input filename:    " << in.infile << endl;
		cout << "Output filename:   " << (in.outfile.empty() ? "<default: console>" : in.outfile) << endl;
		cout << "Output type:       " << in.outtype << endl;
		cout << "Converting...\n";
	}

	trkconv::Converter c;
	c.SetInFilename(in.infile);
	c.SetOutFilename(in.outfile);
	c.SetOutFileType((trkconv::FileType)in.outtype);

	if (!c.Convert())
		cout << "Error: " << c.GetErrorMsg() << endl;
}


int main(int argc, char *argv[])
{
	if (argc == 1)
		print_usage();

	InputParms input(argv[argc - 1]);

	static struct option options[] = 
	{
		{"outfile", required_argument, 0, 'o'},
		{"outtype", required_argument, 0, 't'},
		{"verbose", no_argument,       0, 'v'},
		{0, 0, 0, 0}
	};

	while (true)
	{
		int index = 0;
		int g = getopt_long(argc, argv, "o:t:v", options, &index);
		if (g == -1)
		{
			if (input.outtype < 1 || input.outtype > 4)
			{
				cout << "Invalid/missing output filetype - Must provide (1=CSV, 2=GPX, 3=TCX, 4=KML)\n";
				exit(EXIT_FAILURE);
			}
			else 
				break;
		}

		switch (g)
		{
			case 0:
				break;
			case 'o':
				input.outfile = string(optarg);
				break;
			case 't':
				input.outtype = atoi(optarg);
				break;
			case 'v':
				input.verbose = true;
				break;
			case '?':
				print_usage();
				break;
			default:
				abort();
		}
	}

	valid_input(input);

	exit(EXIT_SUCCESS);
}
