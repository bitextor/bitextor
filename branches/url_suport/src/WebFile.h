#ifndef WEBFILE_H_
#define WEBFILE_H_

#include "GlobalParams.h"
#include "FilePreprocess.h"
#include "Url.h"
#include <string>
#include <iostream>
#include <vector>


extern "C"{
#include <textcat.h>
}
using namespace std;

/**
 * @class WebFile
 * @brief This class represents a web file (HTML file).
 * 
 * This class contains the relevant elements from a web file downloaded
 * from a website with the DownloadModule.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class WebFile
{
private:
	/**
	 * Path where the file is placed in the system.
	 */
	string path;
	
	/**
	 * Language in which the plain text of the file is written.
	 */
	wstring lang;
	
	/**
	 * Data save in the file.
	 */
	string file_type;
	
	/**
	 * File's fingerprint. These fingerprints are strings of integers which represents the content of a file. The
	 * negative integers respresents the XHTML tags. In the class GlobalParams there is a HashMap with the correspondence
	 * between XHTML tag names and their integer code. The positive numbers represents text bloks (concretly, the
	 * positive numbers - included the 0 - reperesnts the length of these text bloks in characters). 
	 */
	vector<int> file;

	/**
	 * Flag which determines if the object has been initialized or not.
	 */
	bool initialized;
	
	/**
	 * Vector de nombres enters trobats al text
	 */
	//vector<int> numbers_vec;

	/**
	 * Length of the plain text (in characters).
	 */
	unsigned int text_size;
	
	/**
	 * URL of the file.
	 */
	Url *url;

	/**
	 * This method can detect if a given character is alphabetic or not by using the regular expressions defined by the
	 * user in the configuration file.
	 * @param car Character to analyze.
	 * @return Returns <code>true</code> if the character is alphabetic and <code>false</code> if it is not.
	 */
	bool IsAlphabetic(const wchar_t& car);
	
	/**
	 * This method obtains the webfile's url from a comment in the HTML code (if it has been downloaded by using HTTrack).
	 */
	void ObtainURL();

public:

	/**
	 * Default constructor.
	 */
	WebFile();
	
	/**
	 * Class destructor.
	 */
	~WebFile();
	
	/**
	 * This method initializes the class. It processes the file to obtain the language, size (in bytes), length (in
	 * characters), URL and the file's fingerprint.
	 * @param path Path where the file is placed in the system.
	 * @throw char* This method throws an exception if it can not find the LibTextCat configuration file path.
	 */
	bool Initialize(const string &path);
	
	/**
	 * This method returns the file language.
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Returns the language code corresponding to the plain text of the file (obtained with LibTextCat).
	 */
	wstring GetLang();
	
	/**
	 * This method returns the file path in the system.
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Returns the file path in the system.
	 */
	string GetPath();
	
	/**
	 * Mètode que permet obtenir el valor del paràmetre sobre el tipus de contingut (extensió) del fitxer.
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Retorna l'extensió del fitxer.
	 */
	//string GetFileType();
	
	/**
	 * This method indicates if the object has been initialized successfuly.
	 * @return Return <code>true</code> if the file has been initialized succesfuly and <code>false</code> if not.
	 */
	bool IsInitialized();

	/**
	 * Mètode que retorna un punter al vector d'enters continguts al text.
	 * @return Retorna un punter al vector d'enters continguts al text.
	 */
	//vector<int>* GetNumbersVector();

	/**
	 * This method returns the file size (in bytes).
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Returns the file size (in bytes).
	 */
	unsigned int GetTextSize();

	/**
	 * This method returns the file's fingerprint.
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Returns the file's fingerprint.
	 */
	vector<int>* GetTagArray();

	/**
	 * This method Mètode que carrega a l'array d'enters els números enters continguts al text.
	 * @param text Text del qual s'hi volen extreure els números.
	 */
	//void GetNonAplha(wstring text);

	/**
	 * This method returns the URL from where the file has been downloaded.
	 * @throw char* This method throws an exception if it is called and the object has not been initialized.
	 * @return Returns the URL from where the file has been downloaded.
	 */
	Url* GetURL();
};

#endif /*WEBFILE_H_*/
