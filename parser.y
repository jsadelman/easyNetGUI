/** \file parser.y Contains the lazyNutOutput Bison parser source */

%{ /*** C/C++ Declarations ***/

#include <stdio.h>
#include <string>
#include <vector>
#include <QDebug>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QAbstractItemModel>
#include <QString>


#include "treeitem.h"
//#include "treemodel.h"
#include "querycontext.h"

%}

/*** yacc/bison Declarations ***/

/* Require bison 2.3 or later */
%require "2.3"

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug

/* start symbol is named "start" */
%start start

/* write out a header file containing the token defines */
%defines

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* namespace to enclose parser in */
%name-prefix="lazyNutOutputParser"

/* set the parser's class identifier */
%define "parser_class_name" "Parser"

/* keep track of the current position within the input */
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
};

/* The driver is passed by reference to the parser. This
 * provides a simple but effective pure interface, not relying on global
 * variables. */
%parse-param { class Driver& driver }

/* verbose error messages */
%error-verbose

 /*** BEGIN LAZYNUTOUTPUT GRAMMAR TOKENS ***/

%union {
    int  			integerVal;
    double 			doubleVal;
    std::string*		wordVal;
    std::string*		stringVal;
    class TreeItem*		treeItemVal;	
}

%token			END	     0	"end of file"
/*%token			EOL		"end of line" */
%token <integerVal> 	INTEGER		"integer"
%token <doubleVal> 	DOUBLE		"double"
%token <wordVal> 	WORD		"word"
/* %token <wordVal> 	QUOTED		"quoted" */
%token <wordVal> 	OBJECT_TYPE	"object type"
%token <wordVal> 	OBJECT_SUBTYPE	"object subtype"
%token <wordVal> 	QUERY_TYPE	"query type"

%token BEGIN_QUERY ANSWER END_QUERY  IS_A LIST_OF COLON_BACKSLASH       



%right  ':'
%left	';'
%left	'\\' '/'


%glr-parser

%type <stringVal> string number text  fullpath
%type <treeItemVal>	label_content

%destructor { delete $$; } WORD
%destructor { delete $$; } string number text
%destructor { delete $$; } label_content


 /*** END LAZYNUTOUTPUT GRAMMAR TOKENS ***/

%{

#include "driver.h"
#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex

%}

%glr-parser
/*%expect-rr 2 */

%% /*** Grammar Rules ***/



/* for the moment numbers are just converted back into strings */
number 	: INTEGER
           {
		$$ = new std::string(std::to_string($1));		
	   }
         | DOUBLE
           {       
		$$ = new std::string(std::to_string($1));	
	   }





string	: WORD
     	   {
		$$ = $1;
	   }
/*	| QUOTED
	   {
     	     $$ = new std::string("\"" + (*$1) + "\"");
     	     delete $1;
           }
*/	| number
     	   {
		$$ = $1;
	   }
	| QUERY_TYPE
	   {
		$$ = $1;
	   }


fullpath	: string '\\' string
     	  	{
	  	$$ = new std::string((*$1) + "\\" + (*$3));
		delete $1;
		delete $3;
	  	}
		| string '/' string
     	  	{
	  	$$ = new std::string((*$1) + "/" + (*$3));
		delete $1;
		delete $3;
	  	}
		| string COLON_BACKSLASH  string
     	  	{
	  	$$ = new std::string((*$1) + ":\\" + (*$3));
		delete $1;
		delete $3;
	  	}
		| fullpath '\\' string
		{
	  	$$ = new std::string((*$1) + "\\" + (*$3));
		delete $1;
		delete $3;
	  	}
		| fullpath '/' string
		{
	  	$$ = new std::string((*$1) + "/" + (*$3));
		delete $1;
		delete $3;
	  	}

text	: string
     	   {
		$$ = $1;
	   }
	| fullpath
     	   {
		$$ = $1;
	   }
	| text string
	   {
		$$ = new std::string((*$1) + " " + (*$2));
		delete $1;
		delete $2;
	   }
	| text fullpath
	   {
		$$ = new std::string((*$1) + " " + (*$2));
		delete $1;
		delete $2;
	   }		
	| text ','
	   {
	       	$$ = new std::string((*$1) + ",");
		delete $1;
           }
        | '(' text ')'
           {
     	     $$ = new std::string("(" + (*$2) + ")");
     	     delete $2;
           }
        | text '(' text ')'
           {
     	     $$ = new std::string((*$1) + " " + "(" + (*$3) + ")");
     	     delete $1;
	     delete $3;
           }
        | '[' text ']'
           {
     	     $$ = new std::string("[" + (*$2) + "]");
     	     delete $2;
           }
        | text '[' text ']'
           {
     	     $$ = new std::string((*$1) + " " + "[" + (*$3) + "]");
     	     delete $1;
	     delete $3;
           }

label_content : text ':' text
		{
                  QVector<QVariant> lc;
                  lc << QString::fromStdString(*$1) << QString::fromStdString(*$3);
                  $$ = new TreeItem(lc);
                  delete $1;
                  delete $3;
		}
	      | text ':' number
		{
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*$1) << QString::fromStdString(*$3);
                  $$ = new TreeItem(lc);
                  delete $1;
                  delete $3;	
		}
	      | text ':'
		{
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*$1) << QString("");
                  $$ = new TreeItem(lc);
                  delete $1;
		}
	      | text IS_A OBJECT_TYPE OBJECT_SUBTYPE
		{
		  QVector<QVariant> lc_object;
		  lc_object << QString("object") <<  QString::fromStdString(*$1);
		  $$ = new TreeItem(lc_object);
		  QVector<QVariant> lc_name;
		  lc_name  << QString("name") <<  QString::fromStdString(*$1);
		  $$->appendChild(new TreeItem(lc_name,$$));
                  delete $1;
		  QVector<QVariant> lc_type;
		  lc_type << QString("type") <<  QString::fromStdString(*$3);
		  $$->appendChild(new TreeItem(lc_type,$$));
                  delete $3;
		  QVector<QVariant> lc_subtype;
		  lc_subtype << QString("subtype") <<  QString::fromStdString(*$4);
		  $$->appendChild(new TreeItem(lc_subtype,$$));
                  delete $4;
		  
		}
	      | text ':' label_content
		{
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*$1) << QString("");
                  $$ = new TreeItem(lc);
		  $$->appendChild($3);
		  $3->setParent($$);
                  delete $1;	
		}
	      | label_content ';' label_content
		{
		  $1->appendChild($3);
		  $3->setParent($1);
                  $$ = $1;	
		}


begin_query:	BEGIN_QUERY number text QUERY_TYPE
	    	{
		QVector<QVariant> lc;
		lc << QString::fromStdString(*$4) <<  QString::fromStdString(*$3);
		driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete $3;
		delete $4;
		/*driver.queryContext.begin = $3;*/
		}	
     		| BEGIN_QUERY number QUERY_TYPE text
		{
		QVector<QVariant> lc;
		lc << QString::fromStdString(*$3) <<  QString::fromStdString(*$4);
		driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete $3;
		delete $4;
		/*driver.queryContext.begin = $4;*/
		}
		| BEGIN_QUERY number QUERY_TYPE
		{
		QVector<QVariant> lc;
		lc << QString::fromStdString(*$3) <<  QString("");
                driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete $3;
		/*driver.queryContext.begin = $4;*/
		}
	
	
		


answer:		ANSWER
      		| ANSWER text
		{
		if (driver.queryContext.begin_query)
		{
			QVector<QVariant> lc_object;
		  	lc_object << QString("object") <<  QString::fromStdString(*$2);
                        TreeItem* answer = new TreeItem(lc_object); 
			driver.queryContext.root->lastChild()->appendChild(answer);
                        /*QModelIndex index_last_query = driver.queryContext.treeModelPtr->index(driver.queryContext.treeModelPtr->rowCount()-1,0);
			driver.queryContext.treeModelPtr->appendSubtree(answer,index_last_query);*/
			delete $2;
		}
		else
		{
			delete $2;
		}
		}
		| ANSWER label_content
		{
		if (driver.queryContext.begin_query)		
		{
			$2->setParent(driver.queryContext.root->lastChild());
			driver.queryContext.root->lastChild()->appendChild($2);
                        /*QModelIndex index_last_query = driver.queryContext.treeModelPtr->index(driver.queryContext.treeModelPtr->rowCount()-1,0);
			driver.queryContext.treeModelPtr->appendSubtree($2,index_last_query);*/
		}
		else
		{
			delete $2;
		}
		}
		| ANSWER LIST_OF number
		| ANSWER text LIST_OF number
		| ANSWER text LIST_OF number text
		;

answers:	answer
       		| answers  answer
		;

end_query:	END_QUERY number text QUERY_TYPE text 
	    	{
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete $2;
		delete $3;
		delete $5;
		}
		| END_QUERY number text QUERY_TYPE ',' text 
	    	{
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete $2;
		delete $3;
		delete $6;
		}	
     		| END_QUERY number QUERY_TYPE text
		{
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete $2;
		delete $4;
		}
		| END_QUERY number QUERY_TYPE ',' text
		{
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete $2;
		delete $5;
		}

query_syntax_error:	BEGIN_QUERY number END_QUERY number text
		  	| BEGIN_QUERY number text END_QUERY number text
			| BEGIN_QUERY END_QUERY text
		  	| BEGIN_QUERY text END_QUERY text
			; /* { qDebug() << "query_syntax_error";} */


start	: /* empty */
        | start answers 
	| start end_query
	| start query_syntax_error
	| start begin_query  end_query 
	{
	driver.queryContext.root->removeLastChild();
	/*driver.queryContext.treeModelPtr->removeRows(driver.queryContext.treeModelPtr->rowCount()-1,1);*/
	}
	| start begin_query  answers  end_query 
	| start END
	;

 /*** END LAZYNUTOUTPUT - Change the grammar rules above ***/

%% /*** Additional Code ***/

void lazyNutOutputParser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}

