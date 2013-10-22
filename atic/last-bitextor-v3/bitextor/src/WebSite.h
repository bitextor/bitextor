#ifndef WEBSITE_H_
#define WEBSITE_H_

#include "WebFile.h"
#include "GlobalParams.h"
#include "Heuristics.h"
#include "BitextCandidates.h"
#include <iostream>
#include <vector>

/**
 * @class WebSite
 * @brief This class contains the elements to represent a downloaded website inside which de system will search bitexts.
 * 
 * This class contains the information about all the files in a downloaded website. From this class the search for bitexts
 * can be thrown.
 * 
 * @author Miquel Esplà i Gomis. 
 */
class WebSite
{
private:

	/**
	 * Path in the system where the website is placed.
	 */
	string base_path;

public:
	/**
	 * Class constructor.
	 * @param path The path in the system where the website is placed.
	 */
	WebSite(const string &path);
	
	/**
	 * Class destructor.
	 */
	~WebSite();
	
	/**
	 * This method returns the path in the system where the website is placed.
	 * @return Returns the path  in the system where the website is placed.
	 */
	string GetBasePath();

	/**
	 * This method returns the name of a file from its complete path.
	 * @param path Complete path from which the file name is obtained.
	 * @return Returns the file name.
	 */
	static string GetFileName(string path);
	
	/**
	 * This method reads the files in the downloaded website and compares the files. From this comparison, it obtains the
	 * best candidates to be bitexts and generates the corresponding translation memories.
	 * @param dest_path The path in the system where the website is placed.
	 * @return Returns <code>true</code> if the TMs are generated successfuly and <code>false</code> if it is not.
	 */
	bool GenerateBitexts(const string &dest_path);
	
	/**
	 * This method compares the files in a list of BitextCandidates and obtains the best candidates to generate TM from
	 * them. The list of candidates is a two dimensional matrix in which the files are organised by levels in the
	 * directories tree.
	 * @param dest_dir The path in the system where the website is placed.
	 * @param file_list List of BitextCandidates.
	 * @param size Size of the file list vector.
	 * @return Returns <code>true</code> if the process is developed successfuly and <code>false</code> if not.
	 */
	bool GetMatchedFiles(const string &dest_dir, vector< BitextCandidates* > **file_list, unsigned int size);
};

#endif /*WEBSITE_H_*/
