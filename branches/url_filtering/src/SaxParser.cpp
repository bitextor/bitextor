#include "SaxParser.h"
#include <libtagaligner/ConfigReader.h>
#include <stack>
#include <memory.h>

xmlSAXHandler SaxParser::saxHandler;

xmlSAXHandler SaxParser::saxHandlerCompare;

stack<wstring> SaxParser::readen_tags;

Url *SaxParser::url=NULL;

Url *SaxParser::readen_url=NULL;

string SaxParser::path;

BitextCandidates *SaxParser::bitext=NULL;

bool SaxParser::correct_tag=false;

string SaxParser::readen_path;

int SaxParser::readen_size;

wstring SaxParser::readen_lang;

wstring SaxParser::lang;

vector<int> SaxParser::readen_fingerprint;

WebFile *SaxParser::wf;
		
vector<Bitext> SaxParser::bitext_vector;

void SaxParser::startElement(void *data, const xmlChar *fullname, const xmlChar **ats)
{
	unsigned int i;
	wstring tag_name=Config::xmlToWstring(fullname);
	readen_tags.push(tag_name);
	if(tag_name==L"file"){
		for(i=0;ats[i];i++){
			if(Config::xmlToWstring(ats[i])==L"url" && ats[i+1]!=NULL)
				SaxParser::url=new Url(Config::xmlToWstring(ats[i+1]));
			else if(Config::xmlToWstring(ats[i])==L"lang" && ats[i+1]!=NULL)
				SaxParser::lang=Config::xmlToWstring(ats[i+1]);
		}
	} else if(tag_name==L"fragment"){
		SaxParser::readen_fingerprint.push_back(atoi(Config::toString(Config::xmlToWstring(ats[1])).c_str()));
	} else if(readen_tags.top()==L"text_size"){
		SaxParser::readen_size=atoi(Config::toString(Config::xmlToWstring(ats[1])).c_str());
	}
}

void SaxParser::startElementCompare(void *data, const xmlChar *fullname, const xmlChar **ats)
{
	unsigned int i;
	wstring tag_name=Config::xmlToWstring(fullname);
	if(correct_tag){
		if(tag_name==L"fragment"){
			SaxParser::readen_fingerprint.push_back(atoi(Config::toString(Config::xmlToWstring(ats[1])).c_str()));
		} else if(readen_tags.top()==L"text_size"){
			SaxParser::readen_size=atoi(Config::toString(Config::xmlToWstring(ats[1])).c_str());
		}
		readen_tags.push(tag_name);
	}
	if(tag_name==L"file"){
		for(i=0;ats[i];i++){
			if(Config::xmlToWstring(ats[i])==L"url" && ats[i+1]!=NULL)
				SaxParser::readen_url=new Url(Config::xmlToWstring(ats[i+1]));
			else if(Config::xmlToWstring(ats[i])==L"lang" && ats[i+1]!=NULL)
				SaxParser::readen_lang=Config::xmlToWstring(ats[i+1]);
		}
		if(readen_url->Differences(url)==1 && lang!=readen_lang){
			correct_tag=true;
			readen_tags.push(tag_name);
		}
		else{
			correct_tag=false;
			delete readen_url;
		}
	}
}

void SaxParser::endElement(void *data, const xmlChar *fullname)
{
	wstring tag_name=Config::xmlToWstring(fullname);
	if(tag_name==L"file"){
		wf=new WebFile(SaxParser::readen_path, SaxParser::lang, SaxParser::readen_fingerprint, SaxParser::url);
		SaxParser::readen_fingerprint.clear();
		SaxParser::bitext=new BitextCandidates(SaxParser::wf);
		//FindCandidatesInURLListXML();
		SaxParser::bitext->GenerateBitexts();
		delete bitext;
	}
	if(tag_name==readen_tags.top())
		readen_tags.pop();
}

void SaxParser::endElementCompare(void *data, const xmlChar *fullname)
{
	if(correct_tag){
		wstring tag_name=Config::xmlToWstring(fullname);
		if(tag_name==L"file"){
			wf=new WebFile(SaxParser::readen_path, SaxParser::readen_lang, SaxParser::readen_fingerprint, SaxParser::readen_url);
			/*for(unsigned int j=0;j<wf->GetTagArray()->size();j++)
				wcout<<wf->GetTagArray()->at(j)<<L";";
			wcout<<endl<<endl<<endl;*/
			//wcout<<L" PATH: "<<Config::toWstring(wf->GetPath())<<L" SIZE: "<<wf->GetTextSize()<<L" LANG: "<<wf->GetLang()<<endl<<endl<<endl;
			if(!SaxParser::bitext->Add(new BitextCandidates(wf)))
				delete wf;
			SaxParser::readen_fingerprint.clear();
		}
		if(tag_name==readen_tags.top())
			readen_tags.pop();
	}
}

void SaxParser::characters(void *data, const xmlChar *ch, int len)
{
	if(readen_tags.top()==L"path"){
		SaxParser::readen_path=Config::toString(Config::xmlToWstring(xmlStrsub(ch,0,len)));
	}
}

void SaxParser::charactersCompare(void *data, const xmlChar *ch, int len)
{
	if(correct_tag && readen_tags.top()==L"path"){
		SaxParser::readen_path=Config::toString(Config::xmlToWstring(xmlStrsub(ch,0,len)));
	}
}

void SaxParser::ProcessURLListXML(const char *path)
{
	xmlSubstituteEntitiesDefault(1);
	SaxParser::path=path;
	//memset(&saxHandler, 0, sizeof(xmlSAXHandler));
	saxHandler.initialized = XML_SAX2_MAGIC;
	saxHandler.endElement = SaxParser::endElement;
	saxHandler.startElement = SaxParser::startElement;
	saxHandler.characters = SaxParser::characters;

	try{
		/*if (!*/xmlSAXUserParseFile(&saxHandler, NULL, path)/*)*/;
			//throw "XML file could not be processed";
	}
	catch(...){
		throw "XML file could not be processed";
	}
}

void SaxParser::FindCandidatesInURLListXML()
{
	try{
		//saxHandlerCompare._private=saxHandler._private;
		//memset(&saxHandlerCompare, 0, sizeof(xmlSAXHandler));
		saxHandlerCompare.endElement = SaxParser::endElementCompare;
		saxHandlerCompare.startElement = SaxParser::startElementCompare;
		saxHandlerCompare.characters = SaxParser::charactersCompare;
		//saxHandlerCompare.setDocumentLocatorSAXFunc=saxHandler.setDocumentLocatorSAXFunc;
		xmlSAXUserParseFile(&saxHandlerCompare, NULL, SaxParser::path.c_str());
/*			throw "XML file could not be processed";*/
	}
	catch(...){
		throw "XML file could not be processed";
	}
}
