#pragma once
#include <iostream>
#include <thread>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/crc.hpp>

using namespace boost::interprocess;
using namespace std;

class CCalcBlockHash
{
public:
	CCalcBlockHash(string sInputFile, uintmax_t nPos, size_t bs) : 
		m_sInputFile(sInputFile),
		m_nPos(nPos),
		m_nBlockSize(bs),
		m_iCheckSum(0),
		m_thWorkThread(NULL)
	{
		//cout << "Constructor: " << m_nPos << endl;
	}
	~CCalcBlockHash() {
		if (m_thWorkThread != NULL) {
			if (m_thWorkThread->joinable()) m_thWorkThread->join();
			delete m_thWorkThread;
			m_thWorkThread = NULL;
		}
		//cout << "Destructor: " << m_nPos << endl;
	}
	// Overloaded operators, not used now but possible used in future (if we use priority_queue)
	friend bool operator<(const CCalcBlockHash& left, const CCalcBlockHash& right) { return left.m_nPos < right.m_nPos; }
	friend bool operator==(const CCalcBlockHash& left, const CCalcBlockHash& right) { return left.m_nPos == right.m_nPos; }
	friend bool operator!=(const CCalcBlockHash& left, const CCalcBlockHash& right) { return left.m_nPos != right.m_nPos; }
private:
	uintmax_t m_nPos;			// Current file position
	size_t m_nBlockSize;		// Blocks size, last block can be smaller
	string m_sInputFile;		// Working file to calculate hash
	thread* m_thWorkThread;		// Working thread
	unsigned int m_iCheckSum;	// Calculated result
public:
	bool Start();
	void Calc();
	string GetResult();
};
