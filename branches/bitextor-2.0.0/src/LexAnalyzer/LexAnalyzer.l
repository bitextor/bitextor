%{
	#include "GlobalParams.h"
	string my_lex_text;
	std::vector<int> my_lex_tag_list;
	int lex_aux_tag, lex_text_length=0;
	char* lex_aux_cad;
%}
%option c++

tag_starting \<[[:space:]]*
heading [hH][eE][aA][dD][[:space:]]*\>[^<]*\<[[:space:]]*\/[hH][eE][aA][dD][[:space:]]*\>
opening_tag [^>/![:space:]]+
close_tag \/[^>]*\>
close_comment --\>
open_comment !--
sp [[:space:]]+
resta .

%x HTML
%x TAG
%x TAGPARAMS
%x COMMENT
%%
.							{unput(yytext[0]); my_lex_tag_list.clear(); my_lex_text=""; BEGIN(HTML);}
<HTML>{tag_starting}		{if(lex_text_length>0) {my_lex_tag_list.push_back(lex_text_length); lex_text_length=0; } BEGIN(TAG);}
<TAG>{heading}				{BEGIN(HTML);}
<TAG>{opening_tag}			{lex_aux_cad=strndup(yytext,yyleng); lex_aux_tag=GlobalParams::GetHTMLTagValue(lex_aux_cad); free(lex_aux_cad); if(lex_aux_tag<0){ my_lex_tag_list.push_back(lex_aux_tag);} BEGIN(TAGPARAMS);}
<TAG>{close_tag}			{BEGIN(HTML);}
<TAG>![^-]					{BEGIN(TAGPARAMS);}
<TAG>{open_comment}			{BEGIN(COMMENT);}
<HTML>{sp}					{my_lex_text+=" ";}
<HTML>{resta}				{lex_aux_cad=strndup(yytext,yyleng); my_lex_text+=lex_aux_cad; free(lex_aux_cad); lex_text_length++;}
<TAGPARAMS>[^>]*>			{BEGIN(HTML);}
<COMMENT>{close_comment}	{BEGIN(HTML);}
<COMMENT>[^-]+				{}
<COMMENT>-+					{}
%%

int yyFlexLexer::yywrap() { return 1; }
