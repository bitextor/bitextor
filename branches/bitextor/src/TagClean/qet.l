

etiqueta \<[^>]*\>
encabezado \<HEAD\>[^>]*\<\/HEAD\>
puntuacio [.,;:]
saltHTML &nbsp;
resta (.|\n)
espais [\ \n\t]+
%%
{encabezado}		{}
{etiqueta}		{}
{saltHTML}		{}
&aacute;		{LexerOutput("",1);}
&eacute;		{LexerOutput("",1);}
&iacute;		{LexerOutput("",1);}
&oacute;		{LexerOutput("",1);}
&uacute;		{LexerOutput("",1);}
&agrave;		{LexerOutput("",1);}
&egrave;		{LexerOutput("",1);}
&igrave;		{LexerOutput("",1);}
&ograve;		{LexerOutput("",1);}
&ugrave;		{LexerOutput("",1);}
&#8217;			{LexerOutput("'",1);}
{puntuacio}		{LexerOutput(" ",1);}	
{espais}		{LexerOutput(" ",1);}
{resta}			{ECHO;}
%%

int yyFlexLexer::yywrap() { return 1; }
