#include "FilePreprocess.h"

bool FilePreprocess::PreprocessFile(string file_path)
{
	fstream file;
	string encod,line,buff;
	TidyDoc tdoc;
	int ok, rc;
	bool exit=true;
	TidyBuffer errbuf;
	TidyBuffer output;
	EncaAnalyser analyser;
	EncaEncoding encoding;
	
	if(GlobalParams::GetTextCatConfigFile()!=""){
		//We detect the charset encode of the file.
		try{	
			tidyBufInit(&output);
			tidyBufInit(&errbuf);
			file.open(file_path.c_str(), ios::in);
			if(file.is_open()){
				int i=0;
				getline(file,line);
				while(!file.eof()){
					if(line.length()>0)
						buff+=line.c_str();
					getline(file,line);
					i++;
				}
				analyser=enca_analyser_alloc("__");
				encoding = enca_analyse_const(analyser, (unsigned char*)buff.c_str(), buff.length());
				encod = enca_charset_name(encoding.charset, ENCA_NAME_STYLE_ICONV);
				if (enca_charset_name(encoding.charset, ENCA_NAME_STYLE_CSTOCS) != NULL)
			      encod=enca_charset_name(encoding.charset, ENCA_NAME_STYLE_CSTOCS);
			    else
			      encod=enca_charset_name(ENCA_CS_UNKNOWN, ENCA_NAME_STYLE_CSTOCS);
		
				file.close();
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
				
				rc = tidySetErrorBuffer(tdoc, &errbuf);
				rc = tidyCleanAndRepair( tdoc );
				rc = tidyParseFile(tdoc, file_path.c_str());
				rc = tidySaveBuffer( tdoc, &output );
				
				file.open(file_path.c_str(),ios::out);
				file<<output.bp;
				file.close();
				tidyRelease( tdoc );
				enca_analyser_free(analyser);
			}
			else
				exit=false;
			
			if(&output!=NULL)
				tidyBufFree( &output );
			if(&errbuf!=NULL)
				tidyBufFree( &errbuf );
		}
		catch(...){
			exit=false;
		}
	}
	else
		throw "TextCat's configuration file has not been specified. Please, define it in the bitextor's configuration file in the section <textCatConfigFile>XXXXX</textCatConfigFile>.";
	
	return exit;
}
