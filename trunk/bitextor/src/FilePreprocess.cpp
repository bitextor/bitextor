/*
 * Autor: Miquel Esplà i Gomis [miquel.espla@ua.es]
 * Any: 2009 
 */

#include "FilePreprocess.h"
#include "GlobalParams.h"
#include <buffio.h>
#include <enca.h>
#include <libtagaligner/ConfigReader.h>

bool FilePreprocess::PreprocessFile(const string &file_path)
{
	fstream file;
	wstring line;
	string encod;
	TidyDoc tdoc;
	int ok, rc;
	bool exit=true;
	TidyBuffer errbuf;
	TidyBuffer output;
	EncaAnalyser analyser;
	EncaEncoding encoding;
	FILE* fin;
	unsigned char *buffer;
	size_t buflen;
	int length;

	if(GlobalParams::GetTextCatConfigFile()!=L""){
		//We detect the charset encode of the file.
		try{	
			tidyBufInit(&output);
			tidyBufInit(&errbuf);
			fin=fopen(file_path.c_str(),"r");
			if (!fin) {//There were errors opening the first input file
				exit=false;
				GlobalParams::WriteLog(L"File "+Config::toWstring(file_path)+L" couldn't be opened.");
			} else {
				fseek(fin,0,SEEK_END);
				length=ftell(fin);
				buffer = (unsigned char*)malloc((length+1)*sizeof(char));
				rewind(fin);
				buflen = fread(buffer, 1, (length+1), fin);
				fclose(fin);

				analyser=enca_analyser_alloc("__");
				encoding = enca_analyse_const(analyser, buffer, buflen);
				encod = enca_charset_name(encoding.charset, ENCA_NAME_STYLE_ICONV);
				if (enca_charset_name(encoding.charset, ENCA_NAME_STYLE_CSTOCS) != NULL)
			    	encod=enca_charset_name(encoding.charset, ENCA_NAME_STYLE_CSTOCS);
			    else
			    	encod=enca_charset_name(ENCA_CS_UNKNOWN, ENCA_NAME_STYLE_CSTOCS);
				file.close();
				free(buffer);
				if(encod=="???")
					encod="ascii";
				//Now we will clean the HTML file
				tdoc = tidyCreate();
				rc = -1;

				ok = tidyOptSetValue(tdoc, TidyInCharEncoding, encod.c_str());
				ok = tidyOptSetValue(tdoc, TidyOutCharEncoding, "utf8");
				ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
				ok = tidyOptSetBool(tdoc, TidyUpperCaseTags, yes);
				ok = tidyOptSetBool(tdoc, TidyHideComments, yes);
				ok = tidyOptSetBool(tdoc, TidyLogicalEmphasis, yes);
				ok = tidyOptSetBool(tdoc, TidyDropEmptyParas, yes);
				ok = tidyOptSetBool(tdoc, TidyMakeBare, yes);
				ok = tidyOptSetBool(tdoc, TidyDropPropAttrs, yes);
				ok = tidyOptSetBool(tdoc, TidyPunctWrap, yes);
				ok = tidyOptSetBool(tdoc, TidyHideComments, yes);
				ok = tidyOptSetValue(tdoc, TidyBodyOnly, "1");
				
				rc = tidySetErrorBuffer(tdoc, &errbuf);
				rc = tidyCleanAndRepair( tdoc );
				rc = tidyParseFile(tdoc, file_path.c_str());
				rc = tidySaveBuffer( tdoc, &output );
				
				file.open(file_path.c_str(),ios::out);
				file<<output.bp;
				file.close();
				tidyRelease( tdoc );
				enca_analyser_free(analyser);
				GlobalParams::WriteLog(L"File "+Config::toWstring(file_path)+L" preprocessed correctly (CharSet Codification: "+Config::toWstring(encod.c_str())+L").");
			}
			
			if(&output!=NULL)
				tidyBufFree( &output );
			if(&errbuf!=NULL)
				tidyBufFree( &errbuf );
		}
		catch(...){
			exit=false;
			GlobalParams::WriteLog(L"Unknown error while opening "+Config::toWstring(file_path));
		}
	}
	else
		throw "TextCat's configuration file has not been specified. Please, define it in the bitextor's configuration file in the section <textCatConfigFile>XXXXX</textCatConfigFile>.";

	return exit;
}
