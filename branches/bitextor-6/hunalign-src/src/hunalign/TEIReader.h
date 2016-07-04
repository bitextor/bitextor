/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_TEIREADER_TEIREADER_H
#define __HUNGLISH_TEIREADER_TEIREADER_H

#include "words.h"

#include <xercesc/dom/DOM.hpp>

#include <iosfwd>

namespace Hunglish
{

std::string toString( const XMLCh* wstr );

std::ostream& operator<<( std::ostream& os, const XMLCh* wstr );

void traverseDOMTree( const XERCES_CPP_NAMESPACE::DOMNode* doc, int depth );

void trivialSerializeSubTree( const XERCES_CPP_NAMESPACE::DOMNode* node, std::ostream& os );

const XERCES_CPP_NAMESPACE::DOMNode* findFirstSubTree( const XERCES_CPP_NAMESPACE::DOMNode* parent, const String& key );

const XERCES_CPP_NAMESPACE::DOMNode* findNextSubTree( const XERCES_CPP_NAMESPACE::DOMNode* root, const XERCES_CPP_NAMESPACE::DOMNode* current, String& key );

void buildWordFromDOMTree_Hu( const XERCES_CPP_NAMESPACE::DOMNode* parent, Word& word, bool lemma );

void buildWordFromDOMTree_En( const XERCES_CPP_NAMESPACE::DOMNode* parent, Word& word, bool lemma );

String getIdOfSentence( const XERCES_CPP_NAMESPACE::DOMNode* parent );

void buildSentenceFromDOMTree_Hu( const XERCES_CPP_NAMESPACE::DOMNode* parent, Sentence& sentence );

void buildSentenceFromDOMTree_En( const XERCES_CPP_NAMESPACE::DOMNode* parent, Sentence& sentence );

void buildSentenceFromDOMTree( const XERCES_CPP_NAMESPACE::DOMNode* parent, Sentence& sentence, bool english );

void buildSentenceListFromDOMTree( const XERCES_CPP_NAMESPACE::DOMNode* parent, SentenceList& seg, bool english );

// If this interface was a class, parseTEI would be its only public method:
int parseTEI( const char* xmlFile, bool english, SentenceList& sentenceList );

// ...And this would be the test for the class:
int main_TEIReader( int argC, char* argV[] );

} // namespace Hunglish

#endif // #define __HUNGLISH_TEIREADER_TEIREADER_H
