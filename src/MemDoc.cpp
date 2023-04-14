#include "MemDoc.h"
#include <cstring>

#ifdef _DEBUG

MemDoc* MemDoc::getInstance(bool useCustomHook/* = false*/)
{
	static MemDoc mInstance(useCustomHook);
	return &mInstance;
}

// Ad-hoc Begin A snapshot
void MemDoc::begin()
{
	record(mLastState);
}

// Ad-hoc End and Dump the last Begin snapshot
void MemDoc::end()
{
	dumpSince(mLastState);
}

// It will Dump since we created the instance of this class
void MemDoc::dump()
{
	dumpSince(mBegState);

	if (pCustomReport)
	{
		std::fclose(pCustomReport);
	}
	pCustomReport = nullptr;

	CloseHandle(mFileHandle);
	CloseHandle(mFilteredFileHandle);
}

std::FILE* MemDoc::getFilePtr()
{
	return pCustomReport;
}

MemDoc::MemDoc(bool useCustomHook)
{
	// Set ON debug allocation Or'ed with check leak at program exit
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	if (useCustomHook)
	{
		if (fopen_s(&pCustomReport, "MemDoc_CustomReport.txt", "w") != 0)
		{
			printf("File open error!");
			return;
		}
		_CrtSetAllocHook(CustomAllocHook);
	}

	mFileHandle = CreateFile("MemDoc_CrtDbgReport.txt", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	mFilteredFileHandle = CreateFile("MemDoc_FilteredCrtDbgReport.txt", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	// Associate Window style file handle for reporting
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, mFileHandle);
	
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, mFileHandle);
	
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, mFileHandle);

	_CrtSetReportHook(CustomReportHook);

	record(mBegState);
}

void MemDoc::record(_CrtMemState &state)
{
	// Take snap at checkpoint
	_CrtMemCheckpoint(&state);
}

void MemDoc::dumpSince(_CrtMemState &state)
{
	_CrtMemState endState, diff;
	memset(&endState, 0, sizeof(_CrtMemState));
	memset(&diff, 0, sizeof(_CrtMemState));

	record(endState);
	if (_CrtMemDifference(&diff, &state, &endState))		// returns true if there is a difference between two states
	{
		_CrtMemDumpAllObjectsSince(&state);
	}
	else
	{
		AddToFile(mFileHandle, "No Leak Detected!!!");
	}
}

// Called before any memory operation
int CustomAllocHook(int nAllocType, void */*userData*/, size_t size,
	int nBlockType, long /*requestNumber*/,
	unsigned char const *filename,
	int lineNumber)
{
	if (nBlockType == _CRT_BLOCK)
		return TRUE; // better to not handle

	if (!MemDoc::getInstance()->getFilePtr())
	{
		return TRUE;	// Can't wrtie as file pointer is NULL
	}

	switch (nAllocType)
	{
	case _HOOK_ALLOC:
		// add the code for handling the allocation requests
		if (filename)
		{
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::ALLOC::Size = %u, FILE = %s::%u\n", (unsigned int)size, (const char *)filename, lineNumber);
		}
		else
		{
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::ALLOC::Size = %u, FILE = UNKOWN::%u\n", (unsigned int)size, lineNumber);
		}

		break;

	case _HOOK_REALLOC:
		// add the code for handling the reallocation requests
		if (filename)
		{
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::RE-ALLOC::Size = %u, FILE = %s::%u\n", (unsigned int)size, (const char *)filename, lineNumber);
		}
		else
		{
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::RE-ALLOC::Size = %u, FILE = UNKOWN::%u\n", (unsigned int)size, lineNumber);
		}
		break;

	case _HOOK_FREE:
		// add the code for handling the free requests
		if (filename)
		{
			//fprintf(MemDoc::getInstance()->getFilePtr(), "MD::FREE::FILE = %s::%u :: ADD = %d\n", (const char *)filename, lineNumber, (int)userData);
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::FREE::FILE = %s::%u\n", (const char *)filename, lineNumber);
		}
		else
		{
			//fprintf(MemDoc::getInstance()->getFilePtr(), "MD::FREE::FILE = UNKOWN::%u :: ADD = %d\n", lineNumber, (int)userData);
			fprintf(MemDoc::getInstance()->getFilePtr(), "MD::FREE::FILE = UNKOWN::%u\n", lineNumber);
		}
		break;
	}

	return TRUE;
}

// Called every time _CrtDbgReport generates a debug report
int CustomReportHook(int /*nRptType*/, char* szMsg, int* retVal)
{
	retVal = 0;
	const char *findStr[] = { ".cpp", ".h" };

	size_t size = sizeof(findStr) / sizeof(findStr[0]);

	for (size_t i = 0; i < size; ++i)
	{
		if (strstr(szMsg, findStr[i]))
			AddToFile(MemDoc::getInstance()->mFilteredFileHandle, szMsg);
	}

	return FALSE;	// Return false as we need the actual report always
}

// Helper function
void AddToFile(HANDLE &fileHandle, const char *msg)
{
	DWORD bytesToWrite = (DWORD)strlen(msg);
	DWORD bytesWritten = 0;

	WriteFile(fileHandle, msg, bytesToWrite, &bytesWritten, nullptr);
	
	// As window has \r\n as new line append it manually
	static const char *newLine = "\r\n";
	bytesToWrite = (DWORD)strlen(newLine);
	bytesWritten = 0;
	WriteFile(fileHandle, newLine, bytesToWrite, &bytesWritten, nullptr);
}
#endif