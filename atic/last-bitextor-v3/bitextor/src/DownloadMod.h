#ifndef DOWNLOADMOD_
#define DOWNLOADMOD_

#include <iostream>

using namespace std;

/**
 * @class DownloadMod
 * @brief Downloading module.
 * 
 * 
 * This class manages the website downloading process. It uses the application HTTrack (www.httrack.com) to download
 * the websites filtering only the html files. The application is called with a <code>system</code> command and 
 * somme parametters can be modiffied to improve the downloading process.
 * 
 * @author Miquel Esplà i Gomis
 */
class DownloadMod
{
private:
	/**
	 * This parametter set the maximum downloading size in bytes.
	 */
	long max_downloaded_size;
	 
	/**
	 * Directory where the website will be downloaded.
	 */
	wstring dest_path;

public:  
	/**
	 * Default costructor of the class.
	 */
	DownloadMod();
	   
	/**
	 * Default destructor of the class.
	 */
	~DownloadMod();
	
	/**
	 * This method sets the maximum download size in bytes.
	 * @param mdt Maximum bytes to download.
	 */
	void SetMaxDownloadedSize(const long &mdt);

	/**
	 * Method to set the destinantion path where the website will be downloaded.
	 * @param path Path where the website will be downloaded.
	 */
	void SetDestPath(const wstring &path);

	/**
	 * This method performs the download of the website with the specified parametters.
	 * @param website URL of the website to download.
	 * @return Returns <code>true</code> if the downloading is performed successfuly and <code>false</code> in other case.
	 */
	bool StartDownload(const wstring &website);
};

#endif /*DOWNLOADMOD_*/
