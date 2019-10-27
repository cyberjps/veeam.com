// HashCalc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <queue>
#include <ctime>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "CCalcBlockHash.h"

int main(int ac, char* av[])
{
	using namespace std;
	namespace po = boost::program_options;
	namespace bfs = boost::filesystem;

	try
	{
		// First, parse input parameters
		//Command Line Parameters
		//	Allowed options :
		//	-h[--help]                      produce help message
		//	- i[--input] arg                 Input file
		//	- o[--output] arg                Output file
		//	- s[--block - size] arg(= 1048576) Block size in bytes(default 1M (1024 * 1024 = 1048576 bytes)

		size_t				nBlockSize;
		unsigned int		nMaxThreads;
		string				sInputFile, sOutputFile;
		uintmax_t			nFileSize;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("input,i", po::value<string>(&sInputFile), "Input file")
			("output,o", po::value<string>(&sOutputFile), "Output file")
			("block-size,s", po::value<size_t>(&nBlockSize)->default_value(1024 * 1024), "Block size in bytes (default 1M (1024*1024=1048576 bytes)")
			("max-threads,t", po::value<unsigned int>(&nMaxThreads)->default_value(10), "Maximum concurrent threads (default 10)")
			;

		try
		{
			po::variables_map vm;
			po::store(po::parse_command_line(ac, av, desc), vm);
			if (vm.count("help"))
			{
				cout << "Command Line Parameters" << endl
					<< desc << endl;
				return EXIT_SUCCESS;
			}
			else if (!vm.count("input") || !vm.count("output")) {
				cout << "Wrong parameters!" << endl << endl
					<< "Command Line Parameters" << endl
					<< desc << endl;
				return EXIT_FAILURE;
			}

			po::notify(vm);
		}
		catch (po::error & e)
		{
				cerr << "ERROR: " << e.what() << endl << endl;
				cerr << desc << endl;
				return EXIT_FAILURE;
		} 

		nFileSize = bfs::file_size(sInputFile);
		// TODO: check file for writable
		ofstream out(sOutputFile);
		
		unsigned int start_time = clock(); // Start time

		cout << "Block size is:\t\t" << nBlockSize << endl
			<< "Maximum threads:\t" << nMaxThreads << endl
			<< "Input file is:\t\t" << sInputFile << endl
			<< "Input filesize is:\t" << nFileSize << endl
			<< "Output file is:\t\t" << sOutputFile << endl << endl;
		

		// 1. Split file to chunks and run calculate hash
		// 2. Create a file mapping
		// 3. Push chuncks to queue
		// 4. Run separate threads for calculate hash
		// 5. Pull calculated data from queue
		// 6. Store to output file (non multithreaded because we need to keep the recording order)

		try
		{
			queue<CCalcBlockHash*> qPrQueue; // Create queue
			uintmax_t nPos = 0;	// Current file position
			do {
				while (qPrQueue.size() < nMaxThreads && nPos < nFileSize) { // We can run more thread and file not ended
					uintmax_t tSize = nFileSize - nPos; // Size of last part of file, for last block can be < nBlockSize;
					if (tSize != 0) { // Last part size eqial to zero only if size of previous block exactly == nBlockSize
						CCalcBlockHash* p_CalcBlockHash = new CCalcBlockHash(sInputFile, nPos, (tSize >= nBlockSize) ? nBlockSize : tSize);
						qPrQueue.push(p_CalcBlockHash); // Push block to queue
						p_CalcBlockHash->Start(); // Start new thread for calculate hash of block
						nPos += nBlockSize;
					}
				}
				// TODO: Separate process to independed thread. This is "bottleneck" now
				while (!qPrQueue.empty()) {
					CCalcBlockHash* pNextBlock = qPrQueue.front(); // Get first element from top of queue (FIFO)
					string hash = pNextBlock->GetResult(); // Get result from this block (wait thread if needed)
					// Store hash to outputfile (append)
					out << hash << endl;
					delete pNextBlock; // Delete block (resources are freed in the destructor, see class code)
					qPrQueue.pop(); // Delete element from top of queue
				}
			} while (nPos < nFileSize);
		}
		catch (exception & e) {
			cerr << "Error in application: " << sInputFile << ", "
				<< e.what() << ", application will now exit" << endl;
			return EXIT_FAILURE;
		}
		unsigned int end_time = clock(); // End time
		unsigned int work_time = (end_time - start_time)/1000;
		cout << "Working time: " << work_time << endl;
	}
	catch (exception & e) 
	{
		cerr << "Unhandled Exception reached the top of main: "
			<< e.what() << ", application will now exit" << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
