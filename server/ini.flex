%option noyywrap

%{
#include <list.h>
#define YY_DECL int generateServerProxy(List* list)
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <files.h>
#include <memory.h>
%}


FUNCTION  ^.+(?:\;|\})$

%%

{FUNCTION}	{
				//do stuff with yytext
			}

;.*	
.	       printf( "Unrecognized character: %s\n", yytext );	   
%%
