#include "SaxParser.h"
#include <libtagaligner/ConfigReader.h>
#include <stack>

SaxParser::SaxParser(){
	url=NULL;
	bitext=NULL;
	reading_tag=false;
}

void SaxParser::startElement(void *data, const xmlChar *fullname, const xmlChar **ats)
{
	wstring tag_name=Config::xmlToWstring(fullname);
	readen_tag.push(tag_name);
	if(tag_name==L"file")
		reading_tag=true;
	else if(reading_tag){
		if(tag_name==L"path")
	}
}

void SaxParser::endElement(void *data, const xmlChar *fullname)
{
	wstring tag_name=Config::xmlToWstring(fullname);
	if(tag_name==L"file"){
		this->bitext=new BitextCandidates(this->wf);
		FindCandidatesInURLListXML();
	}
}

void SaxParser::characters(void *data, const xmlChar *ch, int len)
{

}

void SaxParser::ProcessURLListXML(char *path)
{
	this->path=path;
	//memset(&saxHandler, ‘\0′, sizeof(xmlSAXHandler));
	saxHandler.endElement = SaxParser::endElement;
	saxHandler.startElement = SaxParser::startElement;
	saxHandler.characters = SaxParser::characters;

	try{
		if (!xmlSAXUserParseFile(&saxHandler, NULL, path))
			throw "XML file could not be processed";
	}
	catch(...){
		throw "XML file could not be processed";
	}
}

void SaxParser::FindCandidatesInURLListXML()
{
	//memset(&saxHandler, ‘\0′, sizeof(xmlSAXHandler));
	saxHandler.endElement = SaxParser::endElement;
	saxHandler.startElement = SaxParser::startElement;
	saxHandler.characters = SaxParser::characters;

	try{
		if (!xmlSAXUserParseFile(&saxHandler, NULL, this->path))
			throw "XML file could not be processed";
	}
	catch(...){
		throw "XML file could not be processed";
	}
}
