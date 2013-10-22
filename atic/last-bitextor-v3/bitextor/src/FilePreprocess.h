#ifndef FILEPREPROCESS_H_
#define FILEPREPROCESS_H_

#include <iostream>

using namespace std;

/**
 * @class FilePreprocess
 * @brief Mòdul de preprocessament de fitxers.
 * 
 * This class contains the method to preprocess the input files. It is based on the LibTidyHTML and LibEnca libraries .
 * The firstone converts the HTML file to XHTML and converts the character encoding to UTF-8. The second one detects
 * the character encoding to perform the conversion. Once all these things 
 * 
 * @author Miquel Esplà i Gomis. 
 */
class FilePreprocess
{
public:
	/**
	 * This method performs the web files pre-processing. Before this runing this method, the input file
	 * the file will be in correct XHTML format and encoded with UTF-8.
	 * @param file_path The path of the file to process in the system.
	 * @return Return <code>true</code> if the process is performed successfuly and <code>false</code> in other case.
	 */
	static bool PreprocessFile(const string &file_path);
};

#endif /*FILEPREPROCESS_H_*/

