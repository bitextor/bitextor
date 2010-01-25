#ifndef HEURISTICS_H_
#define HEURISTICS_H_

#include "GlobalParams.h"
#include "WebFile.h"
#include <libtagaligner/ConfigReader.h>
#include <libtagaligner/EditDistanceTools.h>

/**
 * @class Heuristics
 * @brief This class contains all the heuristics of the application.
 * 
 * Static class which contains all the methods related with the heurisitcs used by the applicaiton to
 * find bitexts in a website. These heuristics are:
 * -Edit distance between web page fingerprints (strings composed by XHTML tags and lengths of text bloks).
 * -File sizes comparison (in bytes).
 * -Plane text length (in characters).
 * -Differences in the URL of the files.
 * 
 * @author Miquel Esplà i Gomis
 */
class Heuristics
{
public:
	/**
	 * This method indicates if two files have the same filename extenssion.
	 * @param wf1 First web file to compare.
	 * @param wf2 Second web files to compare.
	 * @return Returns <code>true</code> if the filename extension is the same and <code>false</code> if is different.
	 */
	static bool HaveTheSameExtension(WebFile *wf1, WebFile *wf2);
	
	/**
	 * This method compares the size of a pair of files. If the propotion between the file sizes is higher than the
	 * one defined in the configuration file, the method will return false and, in other case, it will return true. The
	 * proportion between sizes is calculated with the expression ABS(SIZE(wf1)-SIZE(wf2))/MAX(SIZE(wf1),SIZE(wf2)).
	 * @param wf1 First web file to compare.
	 * @param wf2 Second web files to compare.
	 * @param result This is an output parametter which returns the proportion between the file sizes.
	 * @return Returns <code>true</code> if the proportion of difference is lower than the set threshold in the configu-
	 * ration file, or <code>false</code> if it is higher. If the threshold is not defined in the configuration file,
	 * the method returns <code>true</code> for any pair of files.
	 */
	static bool HaveAcceptableSizeDifference(WebFile *wf1, WebFile *wf2=NULL, double* result=NULL);
	
	/**
	 * This method applies the Levheinstein's edit distance algorithm on a pair of web file fingerprints. These
	 * fingerprints are strings of integers which represents the content of a file. The negative integers respresents
	 * the XHTML tags. In the class GlobalParams there is a HashMap with the correspondence between XHTML tag names and
	 * their integer code. The positive numbers represents text bloks (concretly, the positive numbers - included the 0 -
	 * reperesnts the length of these text bloks in characters). The edit distance algorithm applied assigns 1 as the
	 * cost of insertion, deletion and substitution for XHTML tags. For text bloks, the cost of insertion and deletion
	 * is 1 and the cost of substitution is MIN(LENGTH(text_blok1), LENGTH(text_blok2))/MAX(LENGTH(text_blok1), LENGTH(text_blok2)).
	 * This value of edit distance is returned to use it as a comparison value, but for the comparison of files in the
	 * method, another method is calculated. The user can define a maximum permited difference of text-bloks differnece
	 * of length. This parameter is defined in the configuration file and is used to determine if a pair of text bloks
	 * can be parallel (a tranlation) or not. So, the value used for comparsion between fingerprint is this. In this way,
	 * if the substitution cost between text bloks is higher than this threshold, the assigned cost is 1 and, if it is
	 * lower, the cost is 0. The resultaign value of this process is compared with the maximum edit-distance value set
	 * by the user in the configuration file. If the result is lower or equal than this value, the method returns true
	 * and, if it is not, the method returns false.
	 * @param wf1 First web file to compare.
	 * @param wf2 Second web files to compare.
	 * @param pathdistance This is an output parameter which returns the optimal path in the edit-distance algorithm
	 * grid.
	 * @param result Value of edit-distance using the difference proportion in text-bloks substitution operation.
	 * @return Returns <code>true</code> if the edit distance is lower than the threshold defined in the configuration
	 * file and <code>false</code> in other case. If there is no threshold defined in the configuration file, the method
	 * will return always <code>true</code>.
	 */
	static bool HaveAcceptableEditDistance(WebFile *wf1, WebFile *wf2, wstring* pathdistance, double* result=NULL);

	/**
	 * Method wich calculates the cost in the edit distance function HTML tag vs. HTML tag.
	 * @param op Code of the operation wich will be performed (deletion, insertion, substitution).
	 * @param ctag1 First operand.
	 * @param ctag2 Second operand.
	 * @return Cost of the operation. 
	 */
	//static double Cost(const short &op, const int &ctag1, const int &ctag2);
	
	/**
	 * Method wich calculates the cost in the edit distance function HTML tag vs. HTML tag.
	 * @param op Code of the operation which will be performed (deletion, insertion, substitution).
	 * @param ctag1 First operand.
	 * @param ctag2 Second operand.
	 * @return Cost of the operation. 
	 */
	static double CostTextAlignment(const short &op, const int &ctag1, const int &ctag2);

	/**
	 * Method which calculates the edit distance between two arrays of integers (web file fingerprints).
	 * @param wf1 First web file to compare.
	 * @param wf2 Second web files to compare.
	 * @param result Resulting numeric value of the algorithm of edit distance.
	 * @return Returns <code>true</code> if edit distance si lower than the set threshold and <code>false</code> if not.
	 */
	static bool DistanceInNumericFingerprint(WebFile &wf1, WebFile &wf2, double *result=NULL);

	/**
	 * Method which calculates the cost of an operation in the edit distance algorithm for a pair of integers.
	 * @param op Code of the operation which will be performed (deletion, insertion, substitution).
	 * @param c1 First operand.
	 * @param c2 Second operand.
	 * @return Cost of the operation.
	 */
	static double CostNumbers(const short &op, const int &c1, const int &c2);

	/**
	 * This method compares the length of the plain text of a pair of web files. It uses a threshold defined by the user
	 * in the configuration file and compares it with the differene between the compared files:
	 * ABS(SIZE(wf1)-SIZE(wf2))/MAX(SIZE(wf1),SIZE(wf2)). If the threshold is highter, the method considers that the
	 * files are similar enougth.
	 * @param wf1 First web file to compare.
	 * @param wf2 Second web files to compare.
	 * @param value This method returns the difference proportion between the compared files.
	 * @return If the between the compared files is lower or equal than the threshold deffined in the configuration file, the
	 * method returns <code>true</code> and it returns <code>false</code> if it is not. If the threshold is not deffined,
	 * this method will return <code>true</code> always.
	 */
	static bool NearTotalTextSize(WebFile &wf1, WebFile &wf2, double *value=NULL);
	
	//static double GetPhraseVariance(WebFile &wf1, WebFile &wf2, const wstring &pathdistance);
	
	//static double GetPhraseVarianceDesviation(WebFile &wf1, WebFile &wf2, const wstring &pathdistance, const double &prhasevariance);
};

#endif /*HEURISTICS_H_*/
