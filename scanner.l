/** \file scanner.ll Define the lazyNutOutput Flex lexical scanner */

%{ /*** C/C++ Declarations ***/

#include <string>

#include "scanner.h"

/* import the parser's token type into a local typedef */
typedef lazyNutOutput::Parser::token token;
typedef lazyNutOutput::Parser::token_type token_type;

/* By default yylex returns int, we use token_type. Unfortunately yyterminate
 * by default returns 0, which is not of token_type. */
#define yyterminate() return token::END

/* This disables inclusion of unistd.h, which is not available under Visual C++
 * on Win32. The C++ scanner uses STL streams instead. */
#define YY_NO_UNISTD_H

%}

 /*** Flex Declarations and Options ***/

 /* enable c++ scanner class generation */
%option c++

 /* change the name of the scanner class. results in "LazyNutOutputFlexLexer" */
%option prefix="LazyNutOutput"

 /* the manual says "somewhat more optimized" */
%option batch

 /* enable scanner to generate debug output. disable this for release
 * versions. */
%option debug

 /* no support for include files is planned */
%option yywrap nounput 

 /* enables the use of start condition stacks */
%option stack

 /* The following paragraph suffices to track locations accurately. Each time
 * yylex is invoked, the begin position is moved onto the end position. */
%{
#define YY_USER_ACTION  yylloc->columns(yyleng);
%}

 /* exclusive condition for lines to be tokenized */
%x line

 /* exculsive condition for "quoted strings" REMOVED */
 /* %x str */

 /* exculsive condition for unmatched ")" in "ANSWER: 1) "*/
%x unmatched_right_bracket

 /* exculsive condition for ".. is a type/subtype:" */
%x is_a

%%
 /*** Regular Expressions Part ***/

 /* code to place at the beginning of yylex() */
%{
    // reset location
    yylloc->step();
%}

 /*** lazyNutOutput lexer rules below ***/

 /* manage unmatched brackets in "ANSWER: 1)" */

^ANSWER:[ ]+/[0-9]+")" 	{
			BEGIN(unmatched_right_bracket);
			return token::ANSWER;
			} 

<unmatched_right_bracket>[0-9]+/")" 	{
			yylval->integerVal = atoi(yytext);
    			return token::INTEGER;
			}

<unmatched_right_bracket>")"		{
			BEGIN(line);
			return static_cast<token_type>(':');
			}



 /* keywords triggering <line> context  */

^"BEGIN: query"	{
		BEGIN(line);
		return token::BEGIN_QUERY; 
		}

^ANSWER: {
	BEGIN(line);
	return token::ANSWER;
	}

^"END: query"	{
		BEGIN(line);
		return token::END_QUERY;
		}

.  {
    /* ignore anything until new line */	
    yylloc->step();
}

\n {
    yylloc->lines(yyleng); yylloc->step();
}



 /* manage "quoted strings" ----- REMOVED
<line>\" {
   	BEGIN(str);
 }

<str>[^\"]+ {
    yylval->wordVal = new std::string(yytext, yyleng);
    return token::QUOTED;	
}


<str>\" {
   	BEGIN(line);
}

 */


 /* new line ends <line> context */
<line>\n {
    BEGIN(INITIAL);    	
    yylloc->lines(yyleng); yylloc->step();
 }



 /* keywords */


<line>"is a"		{
		BEGIN(is_a);
		return token::IS_A;
		}

<is_a>[A-Za-z_\$][A-Za-z0-9_.?-]*/"/" {
    yylval->wordVal = new std::string(yytext, yyleng);
    return token::OBJECT_TYPE;
}

<is_a>"/" {
	yylloc->step();
 }

<is_a>[A-Za-z_\$][A-Za-z0-9_.?-]*/":" {
    yylval->wordVal = new std::string(yytext, yyleng);
    return token::OBJECT_SUBTYPE;
}

<is_a>":"[ \t]*$ {
		BEGIN(line);
		yylloc->step();
 }

<is_a>":"[ \t]*/[^ \t] {
		BEGIN(line);
		return static_cast<token_type>(';');
		}


<line>"list of"	{
		return token::LIST_OF;
		}


<line>description|recently_modified|setters|clear_recently_modified|clear_recently_created|recently_created|subtypes|list|report {
    yylval->wordVal = new std::string(yytext, yyleng);
    return token::QUERY_TYPE;
 }

 /* basic tokens */

<line>"-"?[0-9]+ {
    yylval->integerVal = atoi(yytext);
    return token::INTEGER;
 }

<line>"-"?[0-9]+"."[0-9]* {
    yylval->doubleVal = atof(yytext);
    return token::DOUBLE;
 }

 /* note: double and single quotes are not special characters */
<line>[A-Za-z_"\$][A-Za-z0-9_"'.?-]* {
    yylval->wordVal = new std::string(yytext, yyleng);
    return token::WORD;
 }

<line>":\\"|":/" {
    return token::COLON_BACKSLASH;
 }


 /* gobble up white-spaces  and dots*/
<line>[ \t\r.]+ {
    yylloc->step();
 }


 /* pass all other characters up to bison */
<line>. {
    return static_cast<token_type>(*yytext);
 }

 /* end <line> */

 /*** lazyNutOutput lexer rules above ***/

%% /*** Additional Code ***/

namespace lazyNutOutput {

Scanner::Scanner(std::istream* in,
		 std::ostream* out)
    : LazyNutOutputFlexLexer(in, out)
{
}

Scanner::~Scanner()
{
}

void Scanner::set_debug(bool b)
{
    yy_flex_debug = b;
}

}

 /* This implementation of LazyNutOutputFlexLexer::yylex() is required to fill the
 * vtable of the class LazyNutOutputFlexLexer. We define the scanner's main yylex
 * function via YY_DECL to reside in the Scanner class instead. */

#ifdef yylex
#undef yylex
#endif

int LazyNutOutputFlexLexer::yylex()
{
    std::cerr << "in LazyNutOutputFlexLexer::yylex() !" << std::endl;
    return 0;
}

 /* When the scanner receives an end-of-file indication from YY_INPUT, it then
 * checks the yywrap() function. If yywrap() returns false (zero), then it is
 * assumed that the function has gone ahead and set up `yyin' to point to
 * another input file, and scanning continues. If it returns true (non-zero),
 * then the scanner terminates, returning 0 to its caller. */

int LazyNutOutputFlexLexer::yywrap()
{
    return 1;
}

