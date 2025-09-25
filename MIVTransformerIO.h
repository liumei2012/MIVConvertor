#pragma once
#include <iostream>
#include <fstream>
using namespace std;


int YUVFileRead(const char* pStrFile, char*& buffer, int nFilePosition)
{
	int length = 12582912;
	std::ifstream is(pStrFile, std::ifstream::binary);
	if (is) {
		// get length of file:
		//is.seekg(0, is.end);
		//length = is.tellg();
		//is.seekg(0, is.beg);

		buffer = new char[length];

		//std::cout << "Reading Source" << length << "\n";
		//std::cout << "Frame " << nFilePosition << "\n";
		// read data as a block:
		unsigned long int n = nFilePosition * length;
		is.seekg(n);
		is.read(buffer, length);


		//if (is)
			//std::cout << pStrFile << "read successfully." << endl;
		//else
		{
			//int n = nFilePosition * length;
			//std::cout << "Position " << n << endl;
			//std::cout << "error: only " << is.gcount() << " could be read" << endl;
			//continue;
		}
		is.seekg(0, is.beg);
		is.close();

		// ...buffer contains the entire file...
	}
	else
	{
		std::cout << "No " << pStrFile << endl;
		return 0;
	}

	return length;
}



int RGBFileRead(const char* pStrFile, char*& buffer, int nFilePosition)
{
	int length = 16777216;
	std::ifstream is(pStrFile, std::ifstream::binary);
	if (is) {
		// get length of file:
		//is.seekg(0, is.end);
		//length = is.tellg();
		//is.seekg(0, is.beg);

		buffer = new char[length];

		//std::cout << "Reading Source" << length << "\n";
		//std::cout << "Frame " << nFilePosition << "\n";
		// read data as a block:
		unsigned long int n = nFilePosition * length;
		is.seekg(n);
		is.read(buffer, length);


		//if (is)
		//	std::cout << pStrFile << "read successfully." << endl;
		//else
		{
			//int n = nFilePosition * length;
			//std::cout << "Position " << n << endl;
			//std::cout << "error: only " << is.gcount() << " could be read" << endl;
			//continue;
		}
		is.seekg(0, is.beg);
		is.close();

		// ...buffer contains the entire file...
	}
	else
	{
		std::cout << "No " << pStrFile << endl;
		return 0;
	}


	return length;
}