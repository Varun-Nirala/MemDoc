/*
	@author : Varun Nirala
	@date	: 1-June-2020
	@desc	: C++ memory leak detector
	@vers	: 1.0
*/

#pragma once

#define _CRTDBG_MAP_ALLOC

#include <cstdlib>
#include <crtdbg.h>
#include <cstdio>
#include <windows.h>

#ifdef _DEBUG
	#ifndef DBG_NEW
		#define DBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
		//#define new DBG_NEW
	#endif
#endif

#ifdef _DEBUG
	#undef	FindText					// This was clashing with our own NMG's GDK::Slot::FindText, and was throwing link time error

	int CustomAllocHook(
		int nAllocType,					// type of allocation like _HOOK_ALLOC , _HOOK_REALLOC or _HOOK_FREE 
		void *userData,					// ptr where memory will be re/assigned, or from where is should be freed
		size_t size,					// size in byte
		int nBlockType,					// type of Block like _CRT_BLOCK or _NORMAL_BLOCK
		long requestNumber,				// which number of request it is
		unsigned char const *filename,	// File name from which it is called
		int lineNumber);				// Line of filename from which it is called

	int CustomReportHook(
		int nRptType,					// Category of report _CRT_WARN, _CRT_ERROR or _CRT_ASSERT
		char *szMsg,					// pointer to a fully assembled report message string
		int *retVal);					// retVal specifies whether _CrtDbgReport should continue normal execution after generating the report or start the debugger.
										// (A retVal value of zero continues execution, a value of 1 starts the debugger.)


	// Undefined behavior if HANDLE is invalid or ptr is NULL
	void AddToFile(
		HANDLE &fileHandle,				// File HANDLE where we need to write
		const char *msg);				// The string we need to write

	// Singleton class
	class MemDoc
	{
		friend int CustomReportHook(int, char *, int*);
	public:
		// Static function to get instance
		static MemDoc* getInstance(bool useCustomHook = false);

		// Ad-hoc Begin A snapshot
		void begin();

		// Ad-hoc End and Dump the last Begin snapshot
		void end();

		// It will Dump since we created the instance of this class
		void dump();

		// It will return the pointer for custom Reporting
		std::FILE* getFilePtr();

	private:
		explicit MemDoc(bool useCustomHook);

		void record(_CrtMemState &state);

		void dumpSince(_CrtMemState &state);

	private:
		_CrtMemState		mBegState{};		// To monitor state since object creation
		_CrtMemState		mLastState{};		// To monitor state on ad-hoc basis
		HANDLE				mFileHandle{};
		HANDLE				mFilteredFileHandle{};
		std::FILE			*pCustomReport{};
	};

	#define INIT_DOC		MemDoc::getInstance();				// First call should be this one or next one
	#define INIT_DOC_CUST	MemDoc::getInstance(true);			// To Dump Custom Report

	#define END_DOC			MemDoc::getInstance()->dump();		// To End and create report

	// @TODO : Still have to work on reporting START
	#define START_SNAP		MemDoc::getInstance()->begin();		// Ad-hoc functions
	#define END_SNAP		MemDoc::getInstance()->end();		// Ad-hoc functions
	// @TODO : Still have to work on reporting END

#else
	// In case of release mode, 
	#define INIT_DOC	 		;
	#define INIT_DOC_CUST		;
	#define END_DOC				;
	#define START_SNAP			;
	#define END_SNAP			;

	#define	DBG_NEW				new
#endif