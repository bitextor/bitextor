/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

/*************************************************************************
*                                                                        *
*  This code uses a very small amount of trivial code fragments of the   *
*  Xerces DOMPrint example. See below for the DOMPrint license:          *
*                                                                        *
*************************************************************************/

/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */


#pragma warning ( disable : 4786 )

#include "TEIReader.h"

#include "words.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "DOMTreeErrorReporter.hpp"

#include <serializeImpl.h>

#include <iostream>
#include <string>
#include <vector>

// Ezt csak valami ideiglenes loggolashoz hasznaltam:
#include <fstream>


// Ez release-ben is befordul:
#define massert(e) if (!(e)) { std::cerr << #e << " failed" << std::endl; throw "assert"; }

using namespace XERCES_CPP_NAMESPACE;


namespace Hunglish
{

std::string tabulate( int a )
{
  std::string s;
  for ( int i=0; i<a; ++i )
  {
    s += " ";
  }

  return s;
}

std::string toString( const XMLCh* wstr )
{

//x   while ( *wstr )
//x   {
//x     if ((*wstr>>8)==0)
//x       s += * ( (const char*)wstr );
//x     else
//x     {
//x       s += ".";
//x     }
//x 
//x     ++wstr;
//x   }

  char* str = XMLString::transcode(wstr);

  std::string s(str);

  XMLString::release(&str);

  return s;
}

std::ostream& operator<<( std::ostream& os, const XMLCh* wstr )
{
  os << toString(wstr);
  return os;
}

void traverseDOMTree( const DOMNode* doc, int depth )
{
  const DOMNodeList* lst = doc->getChildNodes();

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* node = lst->item(i);

    std::cout << tabulate(depth) << node->getNodeType() << " ";
    std::cout << node->getNodeName() << " ";

    if ( node->getNodeType() == DOMNode::TEXT_NODE )
    {
      std::cout << node->getNodeValue() ;
    }

    std::cout << std::endl ;

    traverseDOMTree( node, depth+1 );
  }
}


void trivialSerializeSubTree( const DOMNode* node, std::ostream& os )
{
  os << node->getNodeType() << " ";
  os << node->getNodeName() << " ";
  // std::cout << node->getNodeValue() ;

  if ( node->getNodeType() == DOMNode::TEXT_NODE )
  {
    os << node->getNodeValue() ;
  }

  os << std::endl ;

  const DOMNodeList* lst = node->getChildNodes();

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* child = lst->item(i);
    trivialSerializeSubTree( child, os );
  }
}

const DOMNode* findFirstSubTree( const DOMNode* parent, const String& key )
{
  const DOMNodeList* lst = parent->getChildNodes();

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* node = lst->item(i);

    const std::string nodeName = toString(node->getNodeName());

    if (nodeName == key)
    {
      return node;
    }
    else
    {
      const DOMNode* next = findFirstSubTree( node, key );
      if (next)
        return next;
    }
  }

  return 0;
}

// A root alattiak kozul valaszjuk azt, amelyik az infix rendezesben hatarozottan
// nagyobb a current-nel, es key a neve. Ezek kozul az infix rendezesre nezve legkisebbet.
// Ha ilyen nincs, NULL az eredmeny. Ha current nem siblingje a root-nak, kiakad a fuggveny.

// Tervezett hasznalat:
// c = findFirstSubTree( root, key );
// while (c)
// {
//   c = findNextSubTree( root, c, key );
// }
// infix rendben vegigiteral a root alatti key kulcsuak kozott.
//
const DOMNode* findNextSubTree( const DOMNode* root, const DOMNode* current, String& key )
{
  // Az utodjai kozul az elso.
  const DOMNode* subAttempt = findFirstSubTree( current, key );
  if (subAttempt)
    return subAttempt;

  // Ezt raer maskor megcsinalni.

  throw "unimplemented";

  return 0;
}

void buildWordFromDOMTree_Hu( const DOMNode* parent, Word& word, bool lemma )
{
  massert( toString(parent->getNodeName()) == "w" );

  const DOMNodeList* lst = parent->getChildNodes();

  massert( lst->getLength() > 0 );

  if (!lemma)
  {
    const DOMNode* wordTextNode = lst->item(0);

    massert( wordTextNode->getNodeType() == DOMNode::TEXT_NODE );

    // std::cout << "FOUND WORD: " << wordTextNode->getNodeValue() ;

    word = toString(wordTextNode->getNodeValue());
  }
  else
  {
    const DOMNode* lemmaNode = findFirstSubTree(parent, (std::string)"lemma");

    const DOMNodeList* lst = lemmaNode->getChildNodes();

    if ( lst->getLength() > 0 )
    {
      const DOMNode* wordTextNode = lst->item(0);

      massert( wordTextNode->getNodeType() == DOMNode::TEXT_NODE );

      // std::cout << "FOUND LEMMA: " << wordTextNode->getNodeValue() << std::endl;

      word = toString(wordTextNode->getNodeValue());
    }
    else
    {
      // Megtortenik (bar csak ket precedens van ra),
      // hogy ilyen ronda a lemma: <lemma></lemma>
      // Egyebkent ez valami bug, amit egyfajta kotojel-hasznalatnal lathatunk.

      // std::cout << "EMPTY LEMMA" << std::endl;
      word = "<>";
    }
  }
}

void buildWordFromDOMTree_En( const DOMNode* parent, Word& word, bool lemma )
{
  massert( toString(parent->getNodeName()) == "w" );

  const DOMNodeList* lst = parent->getChildNodes();

  massert( lst->getLength() > 0 );

  if (!lemma)
  {
    const DOMNode* wordTextNode = lst->item(0);

    massert( wordTextNode->getNodeType() == DOMNode::TEXT_NODE );

    // std::cout << "FOUND WORD: " << wordTextNode->getNodeValue() << std::endl;

    word = toString(wordTextNode->getNodeValue());
  }
  else
  {
    const char lemmaName[] = "l\0e\0m\0m\0a\0\0\0";
    const DOMNamedNodeMap* attrMap = parent->getAttributes();
    const DOMNode* lemmaNode = attrMap->getNamedItem((const unsigned short*)lemmaName);

    massert(lemmaNode);

    const DOMNodeList* lst = lemmaNode->getChildNodes();

    massert( lst->getLength() > 0 );

    const DOMNode* wordTextNode = lst->item(0);

    massert( wordTextNode->getNodeType() == DOMNode::TEXT_NODE );

    // std::cout << "FOUND LEMMA: " << wordTextNode->getNodeValue() << std::endl;

    word = toString(wordTextNode->getNodeValue());
  }
}

String getIdOfSentence( const DOMNode* parent )
{
  const char idName[] = "i\0d\0\0\0";
  const DOMNamedNodeMap* attrMap = parent->getAttributes();
  const DOMNode* idNode = attrMap->getNamedItem((const unsigned short*)idName);

  massert(idNode);

  const DOMNodeList* lst = idNode->getChildNodes();

  massert( lst->getLength() > 0 );

  const DOMNode* wordTextNode = lst->item(0);

  massert( wordTextNode->getNodeType() == DOMNode::TEXT_NODE );

  // std::cout << "FOUND ID: " << wordTextNode->getNodeValue() << std::endl;

  String id = toString(wordTextNode->getNodeValue());

  return id;
}

void buildSentenceFromDOMTree_Hu( const DOMNode* parent, Sentence& sentence )
{
  const DOMNodeList* lst = parent->getChildNodes();

  if ( toString(parent->getNodeName()) == "s" )
  {
    // std::cout << std::endl;

    sentence.id = getIdOfSentence(parent);

    massert( lst->getLength() > 0 );

    const DOMNode* sentenceTextNode = lst->item(0);

    massert( sentenceTextNode->getNodeType() == DOMNode::TEXT_NODE );

    // std::cout << "FOUND SENTENCE: " << sentenceTextNode->getNodeValue() ;

    sentence.sentence = toString(sentenceTextNode->getNodeValue());
  }

  WordList& wordList = sentence.words;

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* node = lst->item(i);

    const std::string nodeName = toString(node->getNodeName());

    if (nodeName == "w")
    {
      Word word;
      buildWordFromDOMTree_Hu( node, word, true/*lemma*/ );
      wordList.push_back(word);
    }
    else
    {
      buildSentenceFromDOMTree_Hu( node, sentence );
    }
  }
}

// Itt nincsen sentence.sentence field.
void buildSentenceFromDOMTree_En( const DOMNode* parent, Sentence& sentence )
{
  const DOMNodeList* lst = parent->getChildNodes();

  if ( toString(parent->getNodeName()) == "s" )
  {
    // std::cout << std::endl;
    sentence.id = getIdOfSentence(parent);
  }

  WordList& wordList = sentence.words;

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* node = lst->item(i);

    const std::string nodeName = toString(node->getNodeName());

    if (nodeName == "w")
    {
      Word word;
      buildWordFromDOMTree_En( node, word, true/*lemma*/ );
      wordList.push_back(word);
    }
    else
    {
      buildSentenceFromDOMTree_En( node, sentence );
    }
  }
}

void buildSentenceFromDOMTree( const DOMNode* parent, Sentence& sentence, bool english )
{
  if (english)
  {
    buildSentenceFromDOMTree_En( parent, sentence );
  }
  else
  {
    buildSentenceFromDOMTree_Hu( parent, sentence );
  }
}


void buildSentenceListFromDOMTree( const DOMNode* parent, SentenceList& seg, bool english )
{
  const DOMNodeList* lst = parent->getChildNodes();

  for ( unsigned int i=0; i<lst->getLength(); ++i )
  {
    const DOMNode* node = lst->item(i);

    const std::string nodeName = toString(node->getNodeName());

    if (nodeName == "s")
    {
      Sentence sentence;
      buildSentenceFromDOMTree( node, sentence, english );
      seg.push_back(sentence);
    }
    else
    {
      buildSentenceListFromDOMTree( node, seg, english );
    }
  }
}

int parseTEI( const char* xmlFile, bool english, SentenceList& sentenceList )
{
  try
  {
    XMLPlatformUtils::Initialize();
  }

  catch(const XMLException &toCatch)
  {
    std::cerr << "Error during Xerces-c Initialization.\n"
         << "  Exception message:"
         << StrX(toCatch.getMessage()) << std::endl;
    return 1;
  }

  //
  //  Create our parser, then attach an error handler to the parser.
  //  The parser will call back to methods of the ErrorHandler if it
  //  discovers errors during the course of parsing the XML document.
  //
  XercesDOMParser *parser = new XercesDOMParser;
  parser->setValidationScheme(XercesDOMParser::Val_Never);
  parser->setDoNamespaces(false);
  parser->setDoSchema(false);
  parser->setValidationSchemaFullChecking(false);
  parser->setCreateEntityReferenceNodes(false);

  DOMTreeErrorReporter *errReporter = new DOMTreeErrorReporter();
  parser->setErrorHandler(errReporter);

  //
  //  Parse the XML file, catching any XML exceptions that might propogate
  //  out of it.
  //
  bool errorsOccured = false;
  try
  {
    parser->parse(xmlFile);
  }
  catch (const XMLException& e)
  {
    std::cerr << "An error occurred during parsing\n   Message: "
         << StrX(e.getMessage()) << std::endl;
    errorsOccured = true;
  }
  catch (const DOMException& e)
  {
    const unsigned int maxChars = 2047;
    XMLCh errText[maxChars + 1];

    std::cerr << "\nDOM Error during parsing: '" << xmlFile << "'\n"
         << "DOMException code is:  " << e.code << std::endl;

    if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, maxChars))
         std::cerr << "Message is: " << StrX(errText) << std::endl;

    errorsOccured = true;
  }
  catch (...)
  {
    std::cerr << "An error occurred during parsing\n " << std::endl;
    errorsOccured = true;
  }

  if (errorsOccured || errReporter->getSawErrors())
    return -1;

  // get the DOM representation
  DOMNode *doc = parser->getDocument();

  sentenceList.clear();
  buildSentenceListFromDOMTree(doc,sentenceList,english);

  delete errReporter;

  // Delete the parser itself.  Must be done prior to calling Terminate, below.
  delete parser;

  // And call the termination method
  XMLPlatformUtils::Terminate();

  return 0;
}

int main_TEIReader( int argC, char* argV[] )
{
  const std::string usage = "Usage: TEIReader [ -hu (default) | -en ] data.xml";

  const char* xmlFile = 0;

  bool english(false);

  if (argC == 3)
  {
    xmlFile = argV[2];

    if ((std::string)argV[1]=="-hu")
    {
      english = false;
    }
    else if ((std::string)argV[1]=="-en")
    {
      english = true;
    }
    else
    {
      std::cerr << usage << std::endl;
      return -1;
    }
  }
  else if (argC == 2)
  {
    english = false;
    xmlFile = argV[1];
  }
  else
  {
    std::cerr << usage << std::endl;
    return -1;
  }

  Hunglish::SentenceList sentenceList;
  Hunglish::parseTEI( xmlFile, english, sentenceList );

  for ( int i=0; i<sentenceList.size(); ++i )
  {
    const Sentence& s = sentenceList[i];
    std::cout << s.id << "\t";

    // TODO Ez nem fordul, es nem ertem, hogy miert:
    // std::cout << s.words << std::endl;

    // Workaround:
    for (int j=0; j<s.words.size(); ++j )
    {
      std::cout << s.words[j] ;
      if (j+1 == s.words.size() )
        std::cout << " ";
    }
  }

  return 0;
}

} // namespace Hunglish
