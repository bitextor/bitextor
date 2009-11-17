#ifndef SAXPARSER_H_
#define SAXPARSER_H_

#include <iostream>
#include <libxml/parser.h>

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
		xmlSAXHandler saxHandler;

		stack<wstring> readen_tags;

		Url *url;

		string path;

		BitextCandidates *bitext;
		
		bool reading_tag;
		
		string readen_path;
		
		int readen_size;
		
		wstring readen_lang;
		
		Url *readen_url;
		
		vector<int> readen_fingerprint;

	public:
		SaxParser();
	
		void startElement(void *data, const xmlChar *fullname, const xmlChar **ats);

		void endElement(void *data, const xmlChar *fullname);

		void characters(void *data, const xmlChar *ch, int len);

		void ProcessURLListXML(char *path);
};

#endif /*BITEXT_H_*/
