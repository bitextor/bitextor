/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

#ifndef heavyheuristics_H
#define heavyheuristics_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <util/xhtmltags.h>
#include <util/sentencesplitter.h>


//#define TRAZANDO_HEAVYHEURISTICS
#define kMaxSizeTag 2000

#define kFirstLength 1000 //Number that is added to a text segment to distinguish it from the tags
#define kBigValue 999999999

#define kSizeAuxStrings 3000
#define kSizeHash 500000
#define kRedisperseHash 10001
#define kIncSizeTree 10000
#define kFirstSizeQueue 25000

/** Class that represents a set of tools for the application of heavy heuristics in terms of time
 *  This class is used to calculate heavy heuristics like edit distance to determine if two webpages
 *  are similar enough to be considered bitexts. Heuristics are based in the structure of the tag
 *  tree and also in the frequency of certain tags.
 */
class HeavyHeuristics
 {//Edit distance weight variables
  double EDInsStructural; /** Insertion cost of a structural tag */
  double EDInsFormat; /** Insertion cost of a format tag */
  double EDDelStructural; /** Deletion cost of a structural tag */
  double EDDelFormat; /** Deletion cost of a format tag */
  double EDSubsStrucStruc; /** Substitution cost of a structural cost for a different structural tag */
  double EDSubsStrucFormat; /** Substitution cost of a structural tag for a format tag and vice versa */
  double EDSubsFormatFormat; /** Substitution cost of a format tag for a different format tag */
  double EDWeightperText; /** Substitution cost of each byte of text of the sizes */
  
  double EDLightIns; /** Insertion cost of the light edit distance */
  double EDLightDel; /** Deletion cost of the light edit distance */
  double EDLightSubs; /** Substitution cost of the light edit distance */
  
  unsigned int ALtextsizemin; /** Diferencia minima de tamaño entre segmentos de texto para poder descartar por grandes diferencias de tamaño */
  float ALtextsizeminrelation; /** Diferencia maxima de porcentajes de tamaño entre segmentos de texto para poder considerarlos paralelos */

  
  public:
      
    /** Class constructor
     * Initializes all attributes to their default values
     */
    HeavyHeuristics();
    
    /** Class destructor
     *  Frees all reserved memory of the object
     */
    ~HeavyHeuristics();
        
    /** This function returns the contained tag sequence discarding text segments
     *  This function builds a text string that contains only the tags contained
     *  in the input text, deleting anything else. Symbols '<' and '>' and attributes
     *  are also deleted.
     *
     * @param _text The input text to process
     * @returns the text after removing all but tags or NULL if there is an error
     */
    char* ObtainTagSequence(const char* _text);
    
    /** This function returns the contained tag sequence discarding text segments
     *  This function builds a text string that contains only the tags contained
     *  in the input text, deleting anything else. Symbols '<' and '>' and attributes
     *  are also deleted. Tags are returned in a numerical format to allow quicker
     *  comparisons. A negative number represents closing the tag whose identifier is
     *  its absolute number.
     *
     * @param _text The input text to process
     * @param _ntags The number of tags found (and therefore the size of the resulting vector)
     * @returns a vector with the tags in a numerical format or NULL if there is an error
     */
    short* ObtainNumericTagSequence(const char* _text,unsigned int* _ntags);
    
    /** This function filters a sequence of tags, keeping only the relevant ones
     *  This function process a numerical tag sequence and keeps only structural and format tags.
     *
     * @param _ts the numerical tag sequence to filter
     * @param _tssize the size of the tag sequence
     * @param _tssizeresult The size of the resulting filtered tag sequence
     * @returns the filtered tag sequence or NULL if there is an error
     */
    short* FilterTagSequence(const short* _ts,unsigned int _tssize,unsigned int* _tssizeresult);
    
    /** This function counts the number of tag of each type contained in an input tag sequence
     *  This function processes a tag sequence and counts the number of occurences of
     *  each type of element. The count of all closing tags (which should be the total number
     *  of tags) is stored in the first position of the resulting vector.
     *
     * @param _ts The numerical tag sequence to process
     * @param _tssize The size of the numerical tag sequence to process
     * @returns The number of times that appear each tag
     */
    unsigned short* CountTagsFromSequence(const short* _ts,unsigned int _tssize);
    
    /** This function calculates the Euclidean distance between two input tag counts
     *  This function processes two counts of tags, that is the number of times that
     *  each tag appeared in a certain text, and calculate the Euclidean distance
     *  between them.
     *
     * @param _count1 the first count tag sequence
     * @param _count2 the second count tag sequence
     * @returns the Euclidean distance between both vectors or -1 if there is an error
     */
    double CountsDistance(const unsigned short* _count1,const unsigned short* _count2);
    
    /** This function calculates a simple edit distance without substitutions
     *  This function evaluates how many insertions and deletions are required to
     *  convert the first text into the second one. This is a simplified version
     *  of the edit distance.
     * 
     * @param _text1 the first text
     * @param _text2 the second text
     * @returns the edit distance between these texts.
     */
    double LightEditDistance(const char* _text1,const char* _text2);
        
    /** This function calculates the edit distance between the sequence of tags inside the input texts
     *  This function firstly turns the input texts into numerical tag sequences and, secondly,
     *  calculates the edit distance between them according to the established weights. The
     *  input parameter _maxvalue allows to stop the calculation if the edit distance is higher
     *  than _maxvalue, and returns the value of _maxvalue
     *
     * @param _text1 the first input text
     * @param _text2 the second input text
     * @param _maxvalue the value that allow to stop the process if the edit distance is high
     * @returns The edit distance between the texts or -1 if there is an error
     */
    inline double EditDistanceTagSequence(const char* _text1,const char* _text2,double _maxvalue);
        
    /** This function returns the edit distance between two numerical tag sequences
     *  This function calculates the edit distance cost between the input sequences according
     *  to the established weights. The input parameter _maxvalue allows to stop the
     *  calculation if the edit distance is higher than _maxvalue, and returns the value of
     *  _maxvalue.
     *
     * @param _ts1 the first numerical tag sequence
     * @param _tssize1 the size of the first numerical tag sequence
     * @param _ts2 the second numerical tag sequence
     * @param _tssize2 the size of the second numerical tag sequence
     * @param _maxvalue the value that allow to stop the process if the edit distance is high
     * @returns The edit distance between the texts or -1 if there is an error
     */
    double EditDistanceNumTagSeq(const short* _ts1,unsigned int _tssize1,const short* _ts2,unsigned int _tssize2,double _maxvalue);
    
    /** Function that given two tag and text sequences, returns the sequence of steps of the minimal path of edition cost
     *  This function calculates the minimal edit distance path according to the established
     *  weights. If at any moment the minimal edit distance is over the value of _maxvalue,
     *  the process is stopped and NULL is returned. The sequence of steps is returned as
     *  a sequence of characters in which:
     *  	's': substitution
     *		'd': deletion
     *		'i': insertion
     *
     *  This function only takes into account estructural and format tags, and if
     *  the sequences contain numbers higher than kFirstLength, they are considerated
     *  as text segment sizes instead of tags. Text segments can not be substituted
     *  for tags, only for another text segment, and its cost is the difference between
     *  their sizes.
     * 
     * @param _ts1 the first of the sequences
     * @param _tssize1 the size of the first sequence
     * @param _ts2 the second of the sequences
     * @param _tssize2 the size of the second sequence
     * @param _maxvalue the maximum value that allow to stop the processs if the edit distance is high
     * @returns The sequence of characters that indicate the minimal edit distance path or NULL if there is an error or it was stopped because of _maxvalue
     */
    char* EditDistancePathNumTagSeq(const short* _ts1,unsigned int _tssize1,const short* _ts2,unsigned int _tssize2,double _maxvalue);
    
    /** Function that given two text sequences, returns the sequence of steps of the minimal path of edition cost
     *  This function calculates the minimal edit distance path according to the established
     *  weights. If at any moment the minimal edit distance is over the value of _maxvalue,
     *  the process is stopped and NULL is returned. The sequence of steps is returned as
     *  a sequence of characters in which:
     *          's': substitution
     *          'd': deletion
     *          'i': insertion
     *
     *  This function only takes into account the sizes of the text segments. Text segments can not be substituted
     *  between them if the difference between their distances is higher than ALtextsizeminrelation and the distance
     *  of one of them is at least ALtextsizemin.
     * 
     * @param _ts1 the first of the text sequences
     * @param _tssize1 the size of the first text sequence
     * @param _ts2 the second of the text sequences
     * @param _tssize2 the size of the second text sequence
     * @param _maxvalue the maximum value that allow to stop the processs if the edit distance is high
     * @returns The sequence of characters that indicate the minimal edit distance path or NULL if there is an error or it was stopped because of _maxvalue
     */
    char* EditDistancePathSizeText(const short* _ts1,unsigned int _tssize1,const short* _ts2,unsigned int _tssize2,double _maxvalue);
    
    /** This function aligns two texts regarding to their sentence sizes
     *  This function segments the input texts in sentences and after that performs
     *  the alignment between them. The number of segments is returned in the output
     *  parameter _n_segm and the alignment is returned as a string vector in which
     *  the _n_segm first strings are the left segments and the following _n_segm
     *  are the right ones.
     *
     * @param _ltext The first of the input texts
     * @param _rtext The second of the input texts
     * @param _n_segm The number of segments that have been aligned
     * @returns the result of the alignment as a string vector
     */
    char** AlignText(const char* _ltext,const char* _rtext,unsigned int* _n_segm);
    
    
    /** This Function divides a text into sentences
     *  This function splits the text each time that a character '.',  '!' or '?' is
     *  found.
     *
     *  @param _text The text to process
     *  @param _n_segments The number of segments in which the text was split
     *  @returns the segments that were found
     */
   // char** SentenceSplitter(const char* _text,unsigned int* _n_segments);
 };

#endif
