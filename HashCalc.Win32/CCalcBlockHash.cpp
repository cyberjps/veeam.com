#include "CCalcBlockHash.h"

using namespace std;

// Static independed thread code. Used only for run thread and start calc
// ATTENTION: this is not a class member
void WorkThread(CCalcBlockHash* pCalc) {
	pCalc->Calc();
}

// Start working thread, remember pointer to thread
// TODO: error parsing
bool CCalcBlockHash::Start()
{
	m_thWorkThread = new thread(WorkThread, this);
	return true;
}

// Calculate CRC32 (or any other hash algo) and store result to internal variable
// We using memory mapped files to fast acces to big file
// ATTENTION: this is asynchronous code, do not use stdio to print to console, the result is unpredictable
// TODO: Add parameters from other algo hasing
void CCalcBlockHash::Calc()
{
	file_mapping m_file((m_sInputFile).c_str(), read_only);
	mapped_region region(m_file, read_only, m_nPos, m_nBlockSize);

	boost::crc_32_type  result;
	result.process_bytes(region.get_address(), m_nBlockSize);
	m_iCheckSum = result.checksum();
}

// Wait result calculation. Method must be called outside the class at least one time
// Convert uint to string
// TODO: add parameters for conversion to other formats
string CCalcBlockHash::GetResult()
{
	if(m_thWorkThread->joinable()) m_thWorkThread->join();
	// Convert uint to string
	stringstream stream;
	stream << hex << m_iCheckSum;
	string tstr = stream.str();
	return stream.str();
}
