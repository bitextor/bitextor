#ifndef TRANSLATIONMEMORY_H_
#define TRANSLATIONMEMORY_H_

#include <iostream>
#include <map>
#include "WebFile.h"
#include "BitextCandidates.h"

class TranslationMemory
{
	private:
		static string dest_path;
		
		static map< wstring, pair< FILE*,int >* > uniq_files;
	
	public:
	
		static void SetDestPath(const string &path);

		static bool WriteInSameFile(WebFile* wf1, WebFile* wf2, BitextData* data);

		static bool WriteInDifferentFile(WebFile* wf1, WebFile* wf2, BitextData* data);
	
		static bool WriteTM(WebFile* wf1, WebFile* wf2, BitextData* data);
		
		static void Reset();
};

#endif /*TRANSLATIONMEMORY_H_*/
