#ifndef SAXPARSER_H_
#define SAXPARSER_H_

#include <iostream>
#include <stack>
#include <libxml/parser.h>
#include "Url.h"
#include "BitextCandidates.h"

using namespace std;

typedef struct userData_t
{
	uint32_t events;
} userData;

/**
 * @class SaxParser
 * @brief 
 * 
 * @author Miquel Esplà i Gomis
 */
class SaxParser
{
	private:
		static xmlSAXHandler saxHandler;

		static xmlSAXHandler saxHandlerCompare;

		static stack<wstring> readen_tags;

		static Url *url;
		
		static wstring lang;

		static string path;

		static BitextCandidates *bitext;
		
		static bool correct_tag;
		
		static string readen_path;
		
		static int readen_size;
		
		static wstring readen_lang;
		
		static Url *readen_url;
		
		static vector<int> readen_fingerprint;
		
		static WebFile *wf;
		
		static vector<Bitext> bitext_vector;

	public:
	
		static void startElement(void *data, const xmlChar *fullname, const xmlChar **ats);

		static void startElementCompare(void *data, const xmlChar *fullname, const xmlChar **ats);

		static void endElement(void *data, const xmlChar *fullname);

		static void endElementCompare(void *data, const xmlChar *fullname);

		static void characters(void *data, const xmlChar *ch, int len);
		
		static void charactersCompare(void *data, const xmlChar *ch, int len);

		static void ProcessURLListXML(const char *path);
		
		static void FindCandidatesInURLListXML();
};

#endif /*BITEXT_H_*/
