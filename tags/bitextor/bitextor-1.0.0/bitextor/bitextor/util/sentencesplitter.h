/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

#ifndef SENTENCESPLITTER_H
#define SENTENCESPLITTER_H

#define kMaxSizeSentences 100000 //The maximum size of a single sentence

/*
 This file contains a set of functions related with the sentence
 splitting of a certain text. It combines functions of processing
 words and groups of words. The aim is to improve the results of a
 basic segmentation algorithm that consisted in splitting by ".",
 "?" and "!".
*/
#include <stdio.h>
#include <string>

/** Function that indicates if a character is alphanumeric or not
 *  This function returns true when the character that receives is
 *  between the ranges 'A'-'Z' or 'a'-'z' or '0'-'9'. Additionally
 *  it would return true if the character is one of the following:
 *  '¢', '£', '¤', '¥', 'À', 'Á', 'Ç', 'È', 'É', 'Ì', 'Í', 'Ò', 'Ó',
 *  'Ù', 'Ú', 'Ü', 'à', 'á', 'ç', 'è', 'é', 'ì', 'í', 'ñ', 'ò', 'ó',
 *  'ù', 'ú', 'ü',
 *  Otherwise it returns false.
 *
 * @param _character The character to evaluate
 * @returns True when the character is alphanumeric or False otherwise
 */
bool IsAlphanumeric(const char _character);

/** Function that indicates if the first character of the input string is a blank or not
 *  This function returns true when the first character of the input string
 *  is a blank space, that is, a character ' ', '\n', '\t' or the sequence
 *  "&#160;" (which represents also a blank space). Additionally if the string points
 *  to NULL, it will also be considered as blank space. Otherwise it will return false
 *
 * @param _str The string to evaluate
 * @returns True if _str starts with a blank space or False otherwise
 */
inline bool IsBlankSpace(const char* _str);

/** Function that indicates if a character is in capital letter or not
 *  This function returns true when the character that receives is between
 *  the ranges 'A'-'Z' or is one of the following: 
 *  'À', 'Á', 'Ç', 'È', 'É', 'Ì', 'Í', 'Ò', 'Ó', 'Ù', 'Ú', 'Ü'
 *  Otherwise it returns false.
 *
 * @param _character The character to evaluate
 * @returns True when the character is in capital letter or False otherwise
 */
bool IsCapitalLetter(const char _character);

/** Function that indicates if a character is not in capital letters
 *  This function returns true when the character that receives is between
 *  the ranges 'a'-'z' or is one of the following: 
 *  'à', 'á', 'ç', 'è', 'é', 'ì', 'í', 'ò', 'ó', 'ù', 'ú', 'ü'
 *  Otherwise it returns false.
 *
 * @param _character The character to evaluate
 * @returns True when the character is alphabetic and in capital letter or False otherwise
 */
bool IsNonCapitalLetter(const char _character);

/** Function that given a word checks if the word is in capital letters or not
 *  This function returns True when the word that receives is in capital letters
 *  or False otherwise.
 * 
 * @param _word The word to analyze
 * @returns True when the word is completely in capital letters and false otherwise
 */
bool InCapitalLetters(const char* _word);
 
/** Function that given a word analyzes if it is a number or not
 *  This function returns True when the word that receives is a numerical value or
 *  False otherwise.
 *
 * @param _word The word to analyze
 * @returns True when the word is a numerical value and false otherwise
 */
bool IsNumericWord(const char* _word);

/** Function that given a text turns all blank spaces into ' ' and removes consecutive blank spaces
 *  This function receives a text and returns all words of the text separated
 *  by a single blank space. This is perform by turning all tabs and ends of
 *  line into spaces and then removing consecutive ones.
 *
 * @param _text The text to trim
 * @returns The text without repeated spaces and ends of line and tabs
 */
char* TrimText(const char* _text);

/** Function that given a text string returns its size counting special characters &...;
 *  This function receives an input text string and processes it looking for 
 *  special HTML entities which will be counted as the number of real 
 *  characters they represent. Then, the resulting length is returned. If
 *  the string is NULL or empty the result will be zero.
 *
 * @param _text The input text to be processed
 * @returns The length of the input string
 */
unsigned int TextLength(const char* _text);

/** Function that split a text into sentences using complex heuristics
 *  This function detects the possible split places ('.') of a text and
 *  splits if the dot does not verify one of the following exceptions:
 * 
 *  - It is succeeded by a number
 *  - It is succeeded by an alphabetic character not in capital letters
 *  - It is preceded by an alphabetic capital letter
 *
 *  @param _text The text to process
 *  @param _n_segments The number of segments in which the text was split
 *  @returns the segments that were found
 */
char** DeepSentenceSplitter(const char* _text,unsigned int* _n_segments);

/** Function that split a text into sentences using simple but liable heuristics
 *  This function splits the text each time a character "!", "?" is found.
 *  Furthermore if a word that ends in "." and the following character are a
 *  blank space and a capital letter, it will also split using that dot.
 *
 *  @param _text The text to process
 *  @param _n_segments The number of segments in which the text was split
 *  @returns the segments that were found
 */
char** QuickSentenceSplitter(const char* _text,unsigned int* _n_segments);

 
/** This Function divides a text into sentences
 *  This function splits the text each time that a character '.',  '!' or '?' is
 *  found.
 *
 *  @param _text The text to process
 *  @param _n_segments The number of segments in which the text was split
 *  @returns the segments that were found
 */
char** SentenceSplitter(const char* _text,unsigned int* _n_segments);

#endif
