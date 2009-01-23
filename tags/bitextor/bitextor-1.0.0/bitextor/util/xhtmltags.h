/*
   Implemented by Enrique Sánchez Villamil
   E-mail:   esvillamil@dlsi.ua.es
   Year 2005
*/

/*
 This file contains a list containing the set of tags of XHTML, which associates them
 with a numerical identifier. This stablish a direct relation between a tag and a number
 that can be used to replace quickly tags for each associated identifiers.
 Additionally, a tag classification regarding to their function was made. According to
 this classification there are four types of tags:
  - Structural tags: blockquote, body, caption, cite, col, colgroup, dd, dfn, dir, div, 
                     dl, dt, h1, h2, h3, h4, h5, h6, head, hr, html, li, menu, noframes,
                     noscript, ol, optgroup, option, p, q, select, span, table, tbody,
                     td, tfoot, th, thead, tr, ul.
  - Format tags: abbr, acronym, b, big, center, em, font, i, pre, s, small, strike,
                 strong, style, sub, sup, tt, u.
  - Content tags: a, area, fieldset, form, iframe, img, input, isindex, label, legend,
                  map, object, param, textarea, title.
  - Irrelevant tags: address, applet, base, basefont, bdo, br, button, code, del, ins,
                     kbd, link, meta, samp, script, var. 
*/
#include <stdio.h>

#define kNumberTags 89

#define kTAGa           1
#define kTAGabbr        2
#define kTAGacronym     3
#define kTAGaddress     4
#define kTAGapplet      5
#define kTAGarea        6
#define kTAGb           7
#define kTAGbase        8
#define kTAGbasefont    9
#define kTAGbdo         10
#define kTAGbig         11
#define kTAGblockquote  12
#define kTAGbody        13
#define kTAGbr          14
#define kTAGbutton      15
#define kTAGcaption     16
#define kTAGcenter      17
#define kTAGcite        18
#define kTAGcode        19
#define kTAGcol         20
#define kTAGcolgroup    21
#define kTAGdd          22
#define kTAGdel         23
#define kTAGdfn         24
#define kTAGdir         25
#define kTAGdiv         26
#define kTAGdl          27
#define kTAGdt          28
#define kTAGem          29
#define kTAGfieldset    30
#define kTAGfont        31
#define kTAGform        32
#define kTAGh1          33
#define kTAGh2          34
#define kTAGh3          35
#define kTAGh4          36
#define kTAGh5          37
#define kTAGh6          38
#define kTAGhead        39
#define kTAGhr          40
#define kTAGhtml        41
#define kTAGi           42
#define kTAGiframe      43
#define kTAGimg         44
#define kTAGinput       45
#define kTAGins         46
#define kTAGisindex     47
#define kTAGkbd         48
#define kTAGlabel       49
#define kTAGlegend      50
#define kTAGli          51
#define kTAGlink        52
#define kTAGmap         53
#define kTAGmenu        54
#define kTAGmeta        55
#define kTAGnoframes    56
#define kTAGnoscript    57
#define kTAGobject      58
#define kTAGol          59
#define kTAGoptgroup    60
#define kTAGoption      61
#define kTAGp           62
#define kTAGparam       63
#define kTAGpre         64
#define kTAGq           65
#define kTAGs           66
#define kTAGsamp        67
#define kTAGscript      68
#define kTAGselect      69
#define kTAGsmall       70
#define kTAGspan        71
#define kTAGstrike      72
#define kTAGstrong      73
#define kTAGstyle       74
#define kTAGsub         75
#define kTAGsup         76
#define kTAGtable       77
#define kTAGtbody       78
#define kTAGtd          79
#define kTAGtextarea    80
#define kTAGtfoot       81
#define kTAGth          82
#define kTAGthead       83
#define kTAGtitle       84
#define kTAGtr          85
#define kTAGtt          86
#define kTAGu           87
#define kTAGul          88
#define kTAGvar         89

#define kTAGTypeStruct 1 //Structural tags
#define kTAGTypeFormat 2 //Format tags
#define kTAGTypeContent 3 //Content tags
#define kTAGTypeIrrelevant 4 //Irrelevant tags

/** Function that given a XHTML tag returns its associated identifier
 *  This function processes the input tag and returns the numerical identifier 
 *  which is associated to that tag.
 * 
 * @param _tag Tag whose identifier is required
 * @returns The numerical identifier of the tag or 0 if the tag does not exist
 */
unsigned short GetXHTMLTagNumber(const char* _tag);

/** Function that given the tag identifier returns its name
 * This function returns a pointer to the name of the tag associated with
 * the input identifier.
 *
 * @param _id Identifier of the tag
 * @returns the name of the tag associated with the identifier or NULL if there is an error
 */
const char* GetXHTMLTag(unsigned short _id);

/** Function that given a tag identifier returns the type of the associated tag
 *  This function returns the tag type associated to the input tag identifier.
 *
 * @param _id Identifier of the tag
 * @returns The tag type or 0 if there is an error
 */
unsigned short GetXHTMLTagType(unsigned short _id);
