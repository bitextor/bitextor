/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/


#include "xhtmltags.h"

//Variable that assigns the tag name to its corresponding tag identifier
const char *xhtmltagsname[kNumberTags+1]=
                          {"","a", "abbr", "acronym", "address", "applet", "area",
                           "b", "base", "basefont", "bdo", "big", "blockquote",
                           "body", "br", "button", "caption", "center", "cite",
                           "code", "col", "colgroup", "dd", "del", "dfn", "dir",
                           "div", "dl", "dt", "em", "fieldset", "font", "form",
                           "h1", "h2", "h3", "h4", "h5", "h6", "head", "hr",
                           "html", "i", "iframe", "img", "input", "ins", "isindex",
                           "kbd", "label", "legend", "li", "link", "map", "menu",
                           "meta", "noframes", "noscript", "object", "ol",
                           "optgroup", "option", "p", "param", "pre", "q", "s",
                           "samp", "script", "select", "small", "span", "strike",
                           "strong", "style", "sub", "sup", "table", "tbody",
                           "td", "textarea", "tfoot", "th", "thead", "title",
                           "tr", "tt", "u", "ul", "var"};

//Variable that assigns the tag type to the corresponding tag identifier
const unsigned short xhtmltagstype[kNumberTags+1]={kTAGTypeIrrelevant,
                           kTAGTypeContent, kTAGTypeFormat, kTAGTypeFormat, //a, abbr, acronym
                           kTAGTypeIrrelevant, kTAGTypeIrrelevant, kTAGTypeContent, //address, applet, area
                           kTAGTypeFormat, kTAGTypeIrrelevant, kTAGTypeIrrelevant, // b, base, basefont
                           kTAGTypeIrrelevant, kTAGTypeFormat, kTAGTypeStruct, //bdo, big, blockquote
                           kTAGTypeStruct, kTAGTypeIrrelevant, kTAGTypeIrrelevant, //body, br, button,
                           kTAGTypeStruct, kTAGTypeFormat, kTAGTypeFormat, //caption, center, cite
                           kTAGTypeFormat, kTAGTypeStruct, kTAGTypeStruct, //code, col, colgroup
                           kTAGTypeStruct, kTAGTypeIrrelevant, kTAGTypeFormat, //dd, del, dfn
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeStruct, //dir, div, dl
                           kTAGTypeStruct, kTAGTypeFormat, kTAGTypeContent, //dt, em, fieldset
                           kTAGTypeFormat, kTAGTypeContent, kTAGTypeStruct, //font, form, h1
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeStruct, //h2, h3, h4
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeStruct, //h5, h6, head
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeFormat, //hr, html, i
                           kTAGTypeContent, kTAGTypeContent, kTAGTypeContent, //iframe, img, input
                           kTAGTypeIrrelevant, kTAGTypeContent, kTAGTypeIrrelevant, //ins, isindex, kbd
                           kTAGTypeContent, kTAGTypeContent, kTAGTypeStruct, //label, legend, li
                           kTAGTypeIrrelevant, kTAGTypeContent, kTAGTypeStruct, //link, map, menu
                           kTAGTypeIrrelevant, kTAGTypeStruct, kTAGTypeStruct, //meta, noframes, noscript
                           kTAGTypeContent, kTAGTypeStruct, kTAGTypeStruct, //object, ol, optgroup
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeContent, //option, p, param
                           kTAGTypeFormat, kTAGTypeStruct, kTAGTypeFormat, //pre, q, s
                           kTAGTypeIrrelevant, kTAGTypeIrrelevant, kTAGTypeStruct, //samp, script, select
                           kTAGTypeFormat, kTAGTypeFormat, kTAGTypeFormat, //small, span, strike
                           kTAGTypeFormat, kTAGTypeFormat, kTAGTypeFormat, //strong, style, sub
                           kTAGTypeFormat, kTAGTypeStruct, kTAGTypeStruct, //sup, table, tbody
                           kTAGTypeStruct, kTAGTypeContent, kTAGTypeStruct, //td, textarea, tfoot
                           kTAGTypeStruct, kTAGTypeStruct, kTAGTypeContent, //th, thead, title
                           kTAGTypeStruct, kTAGTypeFormat, kTAGTypeFormat, //tr, tt, u
                           kTAGTypeStruct, kTAGTypeIrrelevant}; //ul, var
                           
unsigned short GetXHTMLTagNumber(const char *tag)
 {
  unsigned short result=0;

  switch(tag[0])
   {
    case 'a': switch(tag[1])
               {
                case '\0': result=kTAGa;
                           break;
                
                case 'b': if(tag[2]=='b' && tag[3]=='r' && tag[4]=='\0')
                            result=kTAGabbr;
                          break;
                
                case 'c': if(tag[2]=='r' && tag[3]=='o' && tag[4]=='n' &&
                             tag[5]=='y' && tag[6]=='m' && tag[7]=='\0')
                            result=kTAGacronym;
                          break;
                
                case 'd': if(tag[2]=='d' && tag[3]=='r' && tag[4]=='e' &&
                             tag[5]=='s' && tag[6]=='s' && tag[7]=='\0')
                            result=kTAGaddress;
                          break;
                
                case 'p': if(tag[2]=='p' && tag[3]=='l' && tag[4]=='e' &&
                             tag[5]=='t' && tag[6]=='\0')
                            result=kTAGapplet;
                          break;
                
                case 'r': if(tag[2]=='e' && tag[3]=='a' && tag[4]=='\0')
                            result=kTAGarea;
                          break;
               }
              break;

    case 'b': switch(tag[1])
               {
                case '\0': result=kTAGb;
                           break;
                
                case 'a': if(tag[2]=='s' && tag[3]=='e')
                           {
                            if(tag[4]=='\0')
                              result=kTAGbase;
                             else
                              {
                               if(tag[4]=='f' && tag[5]=='o' && tag[6]=='n' &&
                                  tag[7]=='t' && tag[8]=='\0')
                                 result=kTAGbasefont;
                              }
                           }
                          break;
                          
                case 'd': if(tag[2]=='o' && tag[3]=='\0')
                            result=kTAGbdo;
                          break;
                          
                case 'i': if(tag[2]=='g' && tag[3]=='\0')
                            result=kTAGbig;
                          break;
                
                case 'l': if(tag[2]=='o' && tag[3]=='c' && tag[4]=='k' &&
                             tag[5]=='q' && tag[6]=='u' && tag[7]=='o' && 
                             tag[8]=='t' && tag[9]=='e' && tag[10]=='\0')
                            result=kTAGblockquote;
                          break;
                          
                case 'o': if(tag[2]=='d' && tag[3]=='y' && tag[4]=='\0')
                            result=kTAGbody;
                          break;
                
                case 'r': if(tag[2]=='\0')
                            result=kTAGbr;
                          break;
         
                case 'u': if(tag[2]=='t' && tag[3]=='t' && tag[4]=='o' &&
                             tag[5]=='n' && tag[6]=='\0')
                            result=kTAGbutton;
                          break;                
               }
              break;
    
    case 'c': switch(tag[1])
               {
                case 'a': if(tag[2]=='p' && tag[3]=='t' && tag[4]=='i' &&
                             tag[5]=='o' && tag[6]=='n' && tag[7]=='\0')
                            result=kTAGcaption;
                          break;
                
                case 'e': if(tag[2]=='n' && tag[3]=='t' && tag[4]=='e' &&
                             tag[5]=='r' && tag[6]=='\0')
                            result=kTAGcenter;
                          break;
                          
                case 'i': if(tag[2]=='t' && tag[3]=='e' && tag[4]=='\0')
                            result=kTAGcite;
                          break;
                
                case 'o': if(tag[2]=='d' && tag[3]=='e' && tag[4]=='\0')
                            result=kTAGcode;
                           else
                            {
                             if(tag[2]=='l')
                              {
                               if(tag[3]=='\0')
                                 result=kTAGcol;
                              }
                              else
                               {
                                if(tag[3]=='g' && tag[4]=='r' && tag[5]=='o' &&
                                   tag[6]=='u' && tag[7]=='p' && tag[8]=='\0')
                                  result=kTAGcolgroup;
                               }
                            }
                          break;
               }
              break;
    
    case 'd': switch(tag[1])
               {
                case 'd': if(tag[2]=='\0')
                            result=kTAGdd;
                          break; 
               
                case 'e': if(tag[2]=='l' && tag[3]=='\0')
                            result=kTAGdel;
                          break;
                          
                case 'f': if(tag[2]=='n' && tag[3]=='\0')
                            result=kTAGdfn;
                          break;
                                
                case 'i': if(tag[2]=='r' && tag[3]=='\0')
                            result=kTAGdir;
                           else
                            {
                             if(tag[2]=='v' && tag[3]=='\0')
                               result=kTAGdiv;
                            }
                          break;
                          
                case 'l': if(tag[2]=='\0')
                            result=kTAGdl;
                          break;
                          
                case 't': if(tag[2]=='\0')
                            result=kTAGdt;
                          break;
                          
               }
              break;

    case 'e': if(tag[1]=='m' && tag[2]=='\0')
                  result=kTAGem;
              break;

    case 'f': switch(tag[1])
               {
                case 'i': if(tag[2]=='e' && tag[3]=='l' && tag[4]=='d' &&
                             tag[5]=='s' && tag[6]=='e' && tag[7]=='t' &&
                             tag[8]=='\0')
                            result=kTAGfieldset;
                          break;
                                  
                case 'o': if(tag[2]=='n' && tag[3]=='t' && tag[4]=='\0')
                            result=kTAGfont;
                           else
                            {
                             if(tag[2]=='r' && tag[3]=='m' && tag[4]=='\0')
                               result=kTAGform;
                            }
                          break;
               }
              break;

    case 'h': switch(tag[1])
               {
                case '1': if(tag[2]=='\0')
                            result=kTAGh1;
                          break;
                
                case '2': if(tag[2]=='\0')
                            result=kTAGh2;
                          break;
                
                case '3': if(tag[2]=='\0')
                            result=kTAGh3;
                          break;
                
                case '4': if(tag[2]=='\0')
                            result=kTAGh4;
                          break;
                
                case '5': if(tag[2]=='\0')
                            result=kTAGh5;
                          break;
                
                case '6': if(tag[2]=='\0')
                            result=kTAGh6;
                          break;
                
                case 'e': if(tag[2]=='a' && tag[3]=='d' && tag[4]=='\0')
                            result=kTAGhead;
                          break;                                            
                
                case 'r': if(tag[2]=='\0')
                            result=kTAGhr;
                          break;                                            
                
                case 't': if(tag[2]=='m' && tag[3]=='l' && tag[4]=='\0')
                            result=kTAGhtml;
                          break;                                            
               }
              break;
              
    case 'i': switch(tag[1])
               {
                case '\0': result=kTAGi;
                           break;
                
                case 'f': if(tag[2]=='r' && tag[3]=='a' && tag[4]=='m' &&
                             tag[5]=='e' && tag[6]=='\0')
                            result=kTAGiframe;
                          break;
                
                case 'm': if(tag[2]=='g' && tag[3]=='\0')
                            result=kTAGimg;
                          break;
                
                case 'n': if(tag[2]=='p' && tag[3]=='u' && tag[4]=='t' &&
                             tag[5]=='\0')
                            result=kTAGinput;
                           else
                            {
                             if(tag[2]=='s' && tag[3]=='\0')
                               result=kTAGins;
                            }
                          break;
                
                case 's': if(tag[2]=='n' && tag[3]=='d' && tag[4]=='e' &&
                             tag[5]=='x' && tag[6]=='\0')
                            result=kTAGisindex;
                          break;
               }
              break;
    
    case 'k': if(tag[1]=='b' && tag[2]=='d' && tag[3]=='\0')
                result=kTAGkbd;
              break;
              
    case 'l': switch(tag[1])
               {
                case 'a': if(tag[2]=='b' && tag[3]=='e' && tag[4]=='l' &&
                             tag[5]=='\0')
                            result=kTAGlabel;
                          break;
                
                case 'e': if(tag[2]=='g' && tag[3]=='e' && tag[4]=='n' &&
                             tag[5]=='d' && tag[6]=='\0')
                            result=kTAGlegend;
                          break;
                
                case 'i': if(tag[2]=='\0')
                            result=kTAGli;
                           else
                            {
                             if(tag[2]=='n' && tag[3]=='k' && tag[4]=='\0')
                               result=kTAGlink;
                            }
                          break;
               }
              break;
              
    case 'm': switch(tag[1])
               {
                case 'a': if(tag[2]=='p' && tag[3]=='\0')
                            result=kTAGmap;
                          break;
                
                case 'e': if(tag[2]=='n' && tag[3]=='u' && tag[4]=='\0')
                            result=kTAGmenu;
                           else
                            {
                             if(tag[2]=='t' && tag[3]=='a' && tag[4]=='\0')
                               result=kTAGmeta;
                            }
                          break;
               }
              break;
              
    case 'n': if(tag[1]=='o')
               {
                if(tag[2]=='f' && tag[3]=='r' && tag[4]=='a' && tag[5]=='m' &&
                   tag[6]=='e' && tag[7]=='s' && tag[8]=='\0')
                  result=kTAGnoframes;
                 else
                  {
                   if(tag[2]=='s' && tag[3]=='c' && tag[4]=='r' && tag[5]=='i' &&
                      tag[6]=='p' && tag[7]=='t' && tag[8]=='\0')
                     result=kTAGnoscript;
                  }
               }
              break;
    
    case 'o': switch(tag[1])
               {
                case 'b': if(tag[2]=='j' && tag[3]=='e' && tag[4]=='c' &&
                             tag[5]=='t' && tag[6]=='\0')
                            result=kTAGobject;
                          break;
                
                case 'l': if(tag[2]=='\0')
                            result=kTAGol;
                          break;
                
                case 'p': if(tag[2]=='t')
                           {
                            if(tag[3]=='g' && tag[4]=='r' && tag[5]=='o' &&
                               tag[6]=='u' && tag[7]=='p' && tag[8]=='\0')
                              result=kTAGoptgroup;
                             else
                              {
                               if(tag[3]=='i' && tag[4]=='o' && tag[5]=='n' &&
                                tag[6]=='\0')
                                 result=kTAGoption;
                              }
                           }
                          break;
               }
              break;
    
    case 'p': switch(tag[1])
               {
                case '\0': result=kTAGp;
                           break;
                
                case 'a': if(tag[2]=='r' && tag[3]=='a' && tag[4]=='m' &&
                             tag[5]=='\0')
                            result=kTAGparam;
                          break;
                
                case 'r': if(tag[2]=='e' && tag[3]=='\0')
                            result=kTAGpre;
                          break;
               }
              break;
    
    case 'q': if(tag[1]=='\0')
                result=kTAGq;
              break;
    
    case 's': switch(tag[1])
               {
                case '\0': result=kTAGs;
                           break;
                
                case 'a': if(tag[2]=='m' && tag[3]=='p' && tag[4]=='\0')
                            result=kTAGsamp;
                          break;
                
                case 'c': if(tag[2]=='r' && tag[3]=='i' && tag[4]=='p' &&
                             tag[5]=='t' && tag[6]=='\0')
                            result=kTAGscript;
                          break;

                case 'e': if(tag[2]=='l' && tag[3]=='e' && tag[4]=='c' &&
                             tag[5]=='t' && tag[6]=='\0')
                            result=kTAGselect;
                          break;

                case 'm': if(tag[2]=='a' && tag[3]=='l' && tag[4]=='l' &&
                             tag[5]=='\0')
                            result=kTAGsmall;
                          break;
                
                case 'p': if(tag[2]=='a' && tag[3]=='n' && tag[4]=='\0')
                            result=kTAGspan;
                          break;
                          
                case 't': if(tag[2]=='r')
                           {
                            if(tag[3]=='i' && tag[4]=='k' && tag[5]=='e' &&
                               tag[6]=='\0')
                              result=kTAGstrike;
                             else
                              {
                               if(tag[3]=='o' && tag[4]=='n' && tag[5]=='g' &&
                                  tag[6]=='\0')
                                 result=kTAGstrong;
                              }
                           }
                           else
                            {
                             if(tag[2]=='y' && tag[3]=='l' && tag[4]=='e' &&
                                tag[5]=='\0')
                               result=kTAGstyle;
                            }
                          break;
                
                case 'u': if(tag[2]=='b' && tag[3]=='\0')
                            result=kTAGsub;
                           else
                            {
                             if(tag[2]=='p' && tag[3]=='\0')
                               result=kTAGsup;
                            }
                          break;
               }
              break;
    
    case 't': switch(tag[1])
               {
                case 'a': if(tag[2]=='b' && tag[3]=='l' && tag[4]=='e' &&
                             tag[5]=='\0')
                            result=kTAGtable;
                          break;
                
                case 'b': if(tag[2]=='o' && tag[3]=='d' && tag[4]=='y' &&
                             tag[5]=='\0')
                            result=kTAGtbody;
                          break;
                
                case 'd': if(tag[2]=='\0')
                            result=kTAGtd;
                          break;

                case 'e': if(tag[2]=='x' && tag[3]=='t' && tag[4]=='a' &&
                             tag[5]=='r' && tag[6]=='e' && tag[7]=='a' &&
                             tag[8]=='\0')
                            result=kTAGtextarea;
                          break;

                case 'f': if(tag[2]=='o' && tag[3]=='o' && tag[4]=='t' &&
                             tag[5]=='\0')
                            result=kTAGtfoot;
                          break;
                
                case 'h': if(tag[2]=='\0')
                            result=kTAGth;
                           else
                            {
                             if(tag[2]=='e' && tag[3]=='a' && tag[4]=='d' &&
                                tag[5]=='\0')
                               result=kTAGthead;
                            }
                          break;
                          
                case 'i': if(tag[2]=='t' && tag[3]=='l' && tag[4]=='e' &&
                             tag[5]=='\0')
                             result=kTAGtitle;
                          break;

                case 'r': if(tag[2]=='\0')
                            result=kTAGtr;
                          break;

                case 't': if(tag[2]=='\0')
                            result=kTAGtt;
                          break;
               }
              break;
    
    case 'u': if(tag[1]=='\0')
                result=kTAGu;
               else
                {
                 if(tag[1]=='l' && tag[2]=='\0')
                   result=kTAGul;
                }
              break;
              
    case 'v': if(tag[1]=='a' && tag[2]=='r' && tag[3]=='\0')
                result=kTAGvar;
              break;
   }
  return(result);
 }

const char* GetXHTMLTag(unsigned short id)
 {
  const char *result;
  if(id>0 && id<=kNumberTags)
    result=xhtmltagsname[id];
   else
     result=NULL;
  return(result);
 }

unsigned short GetXHTMLTagType(unsigned short id)
 {
  unsigned short result;
  if(id>0 && id<=kNumberTags)
    result=xhtmltagstype[id];
   else
     result=0;
  return(result);
 }
