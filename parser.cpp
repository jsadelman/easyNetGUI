/* A Bison parser, made by GNU Bison 2.7.  */

/* Skeleton implementation for Bison LALR(1) parsers in C++
   
      Copyright (C) 2002-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

// Take the name prefix into account.
#define yylex   lazyNutOutputParserlex

/* First part of user declarations.  */
/* Line 279 of lalr1.cc  */
#line 3 "..\\lazyNutGUI\\parser.y"
 /*** C/C++ Declarations ***/

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


/* Line 279 of lalr1.cc  */
#line 59 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"


#include "parser.h"

/* User implementation prologue.  */
/* Line 285 of lalr1.cc  */
#line 103 "..\\lazyNutGUI\\parser.y"


#include "driver.h"
#include "scanner.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex


/* Line 285 of lalr1.cc  */
#line 80 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"


# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* FIXME: INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


/* Suppress unused-variable warnings by "using" E.  */
#define YYUSE(e) ((void) (e))

/* Enable debugging if requested.  */
#if YYDEBUG

/* A pseudo ostream that takes yydebug_ into account.  */
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)	\
do {							\
  if (yydebug_)						\
    {							\
      *yycdebug_ << Title << ' ';			\
      yy_symbol_print_ ((Type), (Value), (Location));	\
      *yycdebug_ << std::endl;				\
    }							\
} while (false)

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug_)				\
    yy_reduce_print_ (Rule);		\
} while (false)

# define YY_STACK_PRINT()		\
do {					\
  if (yydebug_)				\
    yystack_print_ ();			\
} while (false)

#else /* !YYDEBUG */

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Type, Value, Location) YYUSE(Type)
# define YY_REDUCE_PRINT(Rule)        static_cast<void>(0)
# define YY_STACK_PRINT()             static_cast<void>(0)

#endif /* !YYDEBUG */

#define yyerrok		(yyerrstatus_ = 0)
#define yyclearin	(yychar = yyempty_)

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace lazyNutOutputParser {
/* Line 353 of lalr1.cc  */
#line 175 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  Parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              /* Fall through.  */
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  Parser::Parser (class Driver& driver_yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      driver (driver_yyarg)
  {
  }

  Parser::~Parser ()
  {
  }

#if YYDEBUG
  /*--------------------------------.
  | Print this symbol on YYOUTPUT.  |
  `--------------------------------*/

  inline void
  Parser::yy_symbol_value_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yyvaluep);
    std::ostream& yyo = debug_stream ();
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    switch (yytype)
      {
         default:
	  break;
      }
  }


  void
  Parser::yy_symbol_print_ (int yytype,
			   const semantic_type* yyvaluep, const location_type* yylocationp)
  {
    *yycdebug_ << (yytype < yyntokens_ ? "token" : "nterm")
	       << ' ' << yytname_[yytype] << " ("
	       << *yylocationp << ": ";
    yy_symbol_value_print_ (yytype, yyvaluep, yylocationp);
    *yycdebug_ << ')';
  }
#endif

  void
  Parser::yydestruct_ (const char* yymsg,
			   int yytype, semantic_type* yyvaluep, location_type* yylocationp)
  {
    YYUSE (yylocationp);
    YYUSE (yymsg);
    YYUSE (yyvaluep);

    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

    switch (yytype)
      {
        case 5: /* "word" */
/* Line 455 of lalr1.cc  */
#line 96 "..\\lazyNutGUI\\parser.y"
        { delete ((*yyvaluep).wordVal); };
/* Line 455 of lalr1.cc  */
#line 282 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
        break;
      case 25: /* number */
/* Line 455 of lalr1.cc  */
#line 97 "..\\lazyNutGUI\\parser.y"
        { delete ((*yyvaluep).stringVal); };
/* Line 455 of lalr1.cc  */
#line 289 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
        break;
      case 26: /* string */
/* Line 455 of lalr1.cc  */
#line 97 "..\\lazyNutGUI\\parser.y"
        { delete ((*yyvaluep).stringVal); };
/* Line 455 of lalr1.cc  */
#line 296 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
        break;
      case 28: /* text */
/* Line 455 of lalr1.cc  */
#line 97 "..\\lazyNutGUI\\parser.y"
        { delete ((*yyvaluep).stringVal); };
/* Line 455 of lalr1.cc  */
#line 303 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
        break;
      case 29: /* label_content */
/* Line 455 of lalr1.cc  */
#line 98 "..\\lazyNutGUI\\parser.y"
        { delete ((*yyvaluep).treeItemVal); };
/* Line 455 of lalr1.cc  */
#line 310 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
        break;

	default:
	  break;
      }
  }

  void
  Parser::yypop_ (unsigned int n)
  {
    yystate_stack_.pop (n);
    yysemantic_stack_.pop (n);
    yylocation_stack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  Parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  Parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  Parser::debug_level_type
  Parser::debug_level () const
  {
    return yydebug_;
  }

  void
  Parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif

  inline bool
  Parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  Parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  Parser::parse ()
  {
    /// Lookahead and lookahead in internal form.
    int yychar = yyempty_;
    int yytoken = 0;

    // State.
    int yyn;
    int yylen = 0;
    int yystate = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// Semantic value of the lookahead.
    static semantic_type yyval_default;
    semantic_type yylval = yyval_default;
    /// Location of the lookahead.
    location_type yylloc;
    /// The locations where the error started and ended.
    location_type yyerror_range[3];

    /// $$.
    semantic_type yyval;
    /// @$.
    location_type yyloc;

    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


/* User initialization code.  */
/* Line 545 of lalr1.cc  */
#line 49 "..\\lazyNutGUI\\parser.y"
{
    // initialize the initial location object
    yylloc.begin.filename = yylloc.end.filename = &driver.streamname;
}
/* Line 545 of lalr1.cc  */
#line 411 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"

    /* Initialize the stacks.  The initial state will be pushed in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystate_stack_ = state_stack_type (0);
    yysemantic_stack_ = semantic_stack_type (0);
    yylocation_stack_ = location_stack_type (0);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* New state.  */
  yynewstate:
    yystate_stack_.push (yystate);
    YYCDEBUG << "Entering state " << yystate << std::endl;

    /* Accept?  */
    if (yystate == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    /* Backup.  */
  yybackup:

    /* Try to take a decision without lookahead.  */
    yyn = yypact_[yystate];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    /* Read a lookahead token.  */
    if (yychar == yyempty_)
      {
        YYCDEBUG << "Reading a token: ";
        yychar = yylex (&yylval, &yylloc);
      }

    /* Convert token to internal form.  */
    if (yychar <= yyeof_)
      {
	yychar = yytoken = yyeof_;
	YYCDEBUG << "Now at end of input." << std::endl;
      }
    else
      {
	yytoken = yytranslate_ (yychar);
	YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
      }

    /* If the proper action on seeing token YYTOKEN is to reduce or to
       detect an error, take that action.  */
    yyn += yytoken;
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yytoken)
      goto yydefault;

    /* Reduce or error.  */
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
	if (yy_table_value_is_error_ (yyn))
	  goto yyerrlab;
	yyn = -yyn;
	goto yyreduce;
      }

    /* Shift the lookahead token.  */
    YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

    /* Discard the token being shifted.  */
    yychar = yyempty_;

    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yylloc);

    /* Count tokens shifted since error; after three, turn off error
       status.  */
    if (yyerrstatus_)
      --yyerrstatus_;

    yystate = yyn;
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystate];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    /* If YYLEN is nonzero, implement the default value of the action:
       `$$ = $1'.  Otherwise, use the top of the stack.

       Otherwise, the following line sets YYVAL to garbage.
       This behavior is undocumented and Bison
       users should not rely upon it.  */
    if (yylen)
      yyval = yysemantic_stack_[yylen - 1];
    else
      yyval = yysemantic_stack_[0];

    // Compute the default @$.
    {
      slice<location_type, location_stack_type> slice (yylocation_stack_, yylen);
      YYLLOC_DEFAULT (yyloc, slice, yylen);
    }

    // Perform the reduction.
    YY_REDUCE_PRINT (yyn);
    switch (yyn)
      {
          case 2:
/* Line 670 of lalr1.cc  */
#line 125 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = new std::string(std::to_string((yysemantic_stack_[(1) - (1)].integerVal)));		
	   }
    break;

  case 3:
/* Line 670 of lalr1.cc  */
#line 129 "..\\lazyNutGUI\\parser.y"
    {       
		(yyval.stringVal) = new std::string(std::to_string((yysemantic_stack_[(1) - (1)].doubleVal)));	
	   }
    break;

  case 4:
/* Line 670 of lalr1.cc  */
#line 138 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].wordVal);
	   }
    break;

  case 5:
/* Line 670 of lalr1.cc  */
#line 147 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);
	   }
    break;

  case 6:
/* Line 670 of lalr1.cc  */
#line 151 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].wordVal);
	   }
    break;

  case 7:
/* Line 670 of lalr1.cc  */
#line 157 "..\\lazyNutGUI\\parser.y"
    {
	  	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(3) - (1)].stringVal)) + "\\" + (*(yysemantic_stack_[(3) - (3)].stringVal)));
		delete (yysemantic_stack_[(3) - (1)].stringVal);
		delete (yysemantic_stack_[(3) - (3)].stringVal);
	  	}
    break;

  case 8:
/* Line 670 of lalr1.cc  */
#line 163 "..\\lazyNutGUI\\parser.y"
    {
	  	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(3) - (1)].stringVal)) + "/" + (*(yysemantic_stack_[(3) - (3)].stringVal)));
		delete (yysemantic_stack_[(3) - (1)].stringVal);
		delete (yysemantic_stack_[(3) - (3)].stringVal);
	  	}
    break;

  case 9:
/* Line 670 of lalr1.cc  */
#line 169 "..\\lazyNutGUI\\parser.y"
    {
	  	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(3) - (1)].stringVal)) + ":\\" + (*(yysemantic_stack_[(3) - (3)].stringVal)));
		delete (yysemantic_stack_[(3) - (1)].stringVal);
		delete (yysemantic_stack_[(3) - (3)].stringVal);
	  	}
    break;

  case 10:
/* Line 670 of lalr1.cc  */
#line 175 "..\\lazyNutGUI\\parser.y"
    {
	  	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(3) - (1)].stringVal)) + "\\" + (*(yysemantic_stack_[(3) - (3)].stringVal)));
		delete (yysemantic_stack_[(3) - (1)].stringVal);
		delete (yysemantic_stack_[(3) - (3)].stringVal);
	  	}
    break;

  case 11:
/* Line 670 of lalr1.cc  */
#line 181 "..\\lazyNutGUI\\parser.y"
    {
	  	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(3) - (1)].stringVal)) + "/" + (*(yysemantic_stack_[(3) - (3)].stringVal)));
		delete (yysemantic_stack_[(3) - (1)].stringVal);
		delete (yysemantic_stack_[(3) - (3)].stringVal);
	  	}
    break;

  case 12:
/* Line 670 of lalr1.cc  */
#line 188 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);
	   }
    break;

  case 13:
/* Line 670 of lalr1.cc  */
#line 192 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = (yysemantic_stack_[(1) - (1)].stringVal);
	   }
    break;

  case 14:
/* Line 670 of lalr1.cc  */
#line 196 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = new std::string((*(yysemantic_stack_[(2) - (1)].stringVal)) + " " + (*(yysemantic_stack_[(2) - (2)].stringVal)));
		delete (yysemantic_stack_[(2) - (1)].stringVal);
		delete (yysemantic_stack_[(2) - (2)].stringVal);
	   }
    break;

  case 15:
/* Line 670 of lalr1.cc  */
#line 202 "..\\lazyNutGUI\\parser.y"
    {
		(yyval.stringVal) = new std::string((*(yysemantic_stack_[(2) - (1)].stringVal)) + " " + (*(yysemantic_stack_[(2) - (2)].stringVal)));
		delete (yysemantic_stack_[(2) - (1)].stringVal);
		delete (yysemantic_stack_[(2) - (2)].stringVal);
	   }
    break;

  case 16:
/* Line 670 of lalr1.cc  */
#line 208 "..\\lazyNutGUI\\parser.y"
    {
	       	(yyval.stringVal) = new std::string((*(yysemantic_stack_[(2) - (1)].stringVal)) + ",");
		delete (yysemantic_stack_[(2) - (1)].stringVal);
           }
    break;

  case 17:
/* Line 670 of lalr1.cc  */
#line 213 "..\\lazyNutGUI\\parser.y"
    {
     	     (yyval.stringVal) = new std::string("(" + (*(yysemantic_stack_[(3) - (2)].stringVal)) + ")");
     	     delete (yysemantic_stack_[(3) - (2)].stringVal);
           }
    break;

  case 18:
/* Line 670 of lalr1.cc  */
#line 218 "..\\lazyNutGUI\\parser.y"
    {
     	     (yyval.stringVal) = new std::string((*(yysemantic_stack_[(4) - (1)].stringVal)) + " " + "(" + (*(yysemantic_stack_[(4) - (3)].stringVal)) + ")");
     	     delete (yysemantic_stack_[(4) - (1)].stringVal);
	     delete (yysemantic_stack_[(4) - (3)].stringVal);
           }
    break;

  case 19:
/* Line 670 of lalr1.cc  */
#line 224 "..\\lazyNutGUI\\parser.y"
    {
     	     (yyval.stringVal) = new std::string("[" + (*(yysemantic_stack_[(3) - (2)].stringVal)) + "]");
     	     delete (yysemantic_stack_[(3) - (2)].stringVal);
           }
    break;

  case 20:
/* Line 670 of lalr1.cc  */
#line 229 "..\\lazyNutGUI\\parser.y"
    {
     	     (yyval.stringVal) = new std::string((*(yysemantic_stack_[(4) - (1)].stringVal)) + " " + "[" + (*(yysemantic_stack_[(4) - (3)].stringVal)) + "]");
     	     delete (yysemantic_stack_[(4) - (1)].stringVal);
	     delete (yysemantic_stack_[(4) - (3)].stringVal);
           }
    break;

  case 21:
/* Line 670 of lalr1.cc  */
#line 236 "..\\lazyNutGUI\\parser.y"
    {
                  QVector<QVariant> lc;
                  lc << QString::fromStdString(*(yysemantic_stack_[(3) - (1)].stringVal)) << QString::fromStdString(*(yysemantic_stack_[(3) - (3)].stringVal));
                  (yyval.treeItemVal) = new TreeItem(lc);
                  delete (yysemantic_stack_[(3) - (1)].stringVal);
                  delete (yysemantic_stack_[(3) - (3)].stringVal);
		}
    break;

  case 22:
/* Line 670 of lalr1.cc  */
#line 244 "..\\lazyNutGUI\\parser.y"
    {
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*(yysemantic_stack_[(3) - (1)].stringVal)) << QString::fromStdString(*(yysemantic_stack_[(3) - (3)].stringVal));
                  (yyval.treeItemVal) = new TreeItem(lc);
                  delete (yysemantic_stack_[(3) - (1)].stringVal);
                  delete (yysemantic_stack_[(3) - (3)].stringVal);	
		}
    break;

  case 23:
/* Line 670 of lalr1.cc  */
#line 252 "..\\lazyNutGUI\\parser.y"
    {
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*(yysemantic_stack_[(2) - (1)].stringVal)) << QString("");
                  (yyval.treeItemVal) = new TreeItem(lc);
                  delete (yysemantic_stack_[(2) - (1)].stringVal);
		}
    break;

  case 24:
/* Line 670 of lalr1.cc  */
#line 259 "..\\lazyNutGUI\\parser.y"
    {
		  QVector<QVariant> lc_object;
		  lc_object << QString("object") <<  QString::fromStdString(*(yysemantic_stack_[(4) - (1)].stringVal));
		  (yyval.treeItemVal) = new TreeItem(lc_object);
		  QVector<QVariant> lc_name;
		  lc_name  << QString("name") <<  QString::fromStdString(*(yysemantic_stack_[(4) - (1)].stringVal));
		  (yyval.treeItemVal)->appendChild(new TreeItem(lc_name,(yyval.treeItemVal)));
                  delete (yysemantic_stack_[(4) - (1)].stringVal);
		  QVector<QVariant> lc_type;
		  lc_type << QString("type") <<  QString::fromStdString(*(yysemantic_stack_[(4) - (3)].wordVal));
		  (yyval.treeItemVal)->appendChild(new TreeItem(lc_type,(yyval.treeItemVal)));
                  delete (yysemantic_stack_[(4) - (3)].wordVal);
		  QVector<QVariant> lc_subtype;
		  lc_subtype << QString("subtype") <<  QString::fromStdString(*(yysemantic_stack_[(4) - (4)].wordVal));
		  (yyval.treeItemVal)->appendChild(new TreeItem(lc_subtype,(yyval.treeItemVal)));
                  delete (yysemantic_stack_[(4) - (4)].wordVal);
		  
		}
    break;

  case 25:
/* Line 670 of lalr1.cc  */
#line 278 "..\\lazyNutGUI\\parser.y"
    {
		  QVector<QVariant> lc;
                  lc << QString::fromStdString(*(yysemantic_stack_[(3) - (1)].stringVal)) << QString("");
                  (yyval.treeItemVal) = new TreeItem(lc);
		  (yyval.treeItemVal)->appendChild((yysemantic_stack_[(3) - (3)].treeItemVal));
		  (yysemantic_stack_[(3) - (3)].treeItemVal)->setParent((yyval.treeItemVal));
                  delete (yysemantic_stack_[(3) - (1)].stringVal);	
		}
    break;

  case 26:
/* Line 670 of lalr1.cc  */
#line 287 "..\\lazyNutGUI\\parser.y"
    {
		  (yysemantic_stack_[(3) - (1)].treeItemVal)->appendChild((yysemantic_stack_[(3) - (3)].treeItemVal));
		  (yysemantic_stack_[(3) - (3)].treeItemVal)->setParent((yysemantic_stack_[(3) - (1)].treeItemVal));
                  (yyval.treeItemVal) = (yysemantic_stack_[(3) - (1)].treeItemVal);	
		}
    break;

  case 27:
/* Line 670 of lalr1.cc  */
#line 295 "..\\lazyNutGUI\\parser.y"
    {
		QVector<QVariant> lc;
		lc << QString::fromStdString(*(yysemantic_stack_[(4) - (4)].wordVal)) <<  QString::fromStdString(*(yysemantic_stack_[(4) - (3)].stringVal));
		driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete (yysemantic_stack_[(4) - (3)].stringVal);
		delete (yysemantic_stack_[(4) - (4)].wordVal);
		/*driver.queryContext.begin = $3;*/
		}
    break;

  case 28:
/* Line 670 of lalr1.cc  */
#line 306 "..\\lazyNutGUI\\parser.y"
    {
		QVector<QVariant> lc;
		lc << QString::fromStdString(*(yysemantic_stack_[(4) - (3)].wordVal)) <<  QString::fromStdString(*(yysemantic_stack_[(4) - (4)].stringVal));
		driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete (yysemantic_stack_[(4) - (3)].wordVal);
		delete (yysemantic_stack_[(4) - (4)].stringVal);
		/*driver.queryContext.begin = $4;*/
		}
    break;

  case 29:
/* Line 670 of lalr1.cc  */
#line 317 "..\\lazyNutGUI\\parser.y"
    {
		QVector<QVariant> lc;
		lc << QString::fromStdString(*(yysemantic_stack_[(3) - (3)].wordVal)) <<  QString("");
                driver.queryContext.root->appendChild(new TreeItem(lc,driver.queryContext.root));
		/*driver.queryContext.treeModelPtr->appendSubtree(new TreeItem(lc));*/
		driver.queryContext.begin_query = true;
		delete (yysemantic_stack_[(3) - (3)].wordVal);
		/*driver.queryContext.begin = $4;*/
		}
    break;

  case 31:
/* Line 670 of lalr1.cc  */
#line 333 "..\\lazyNutGUI\\parser.y"
    {
		if (driver.queryContext.begin_query)
		{
			QVector<QVariant> lc_object;
		  	lc_object << QString("object") <<  QString::fromStdString(*(yysemantic_stack_[(2) - (2)].stringVal));
                        TreeItem* answer = new TreeItem(lc_object); 
			driver.queryContext.root->lastChild()->appendChild(answer);
                        /*QModelIndex index_last_query = driver.queryContext.treeModelPtr->index(driver.queryContext.treeModelPtr->rowCount()-1,0);
			driver.queryContext.treeModelPtr->appendSubtree(answer,index_last_query);*/
			delete (yysemantic_stack_[(2) - (2)].stringVal);
		}
		else
		{
			delete (yysemantic_stack_[(2) - (2)].stringVal);
		}
		}
    break;

  case 32:
/* Line 670 of lalr1.cc  */
#line 350 "..\\lazyNutGUI\\parser.y"
    {
		if (driver.queryContext.begin_query)		
		{
			(yysemantic_stack_[(2) - (2)].treeItemVal)->setParent(driver.queryContext.root->lastChild());
			driver.queryContext.root->lastChild()->appendChild((yysemantic_stack_[(2) - (2)].treeItemVal));
                        /*QModelIndex index_last_query = driver.queryContext.treeModelPtr->index(driver.queryContext.treeModelPtr->rowCount()-1,0);
			driver.queryContext.treeModelPtr->appendSubtree($2,index_last_query);*/
		}
		else
		{
			delete (yysemantic_stack_[(2) - (2)].treeItemVal);
		}
		}
    break;

  case 38:
/* Line 670 of lalr1.cc  */
#line 373 "..\\lazyNutGUI\\parser.y"
    {
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete (yysemantic_stack_[(5) - (2)].stringVal);
		delete (yysemantic_stack_[(5) - (3)].stringVal);
		delete (yysemantic_stack_[(5) - (5)].stringVal);
		}
    break;

  case 39:
/* Line 670 of lalr1.cc  */
#line 381 "..\\lazyNutGUI\\parser.y"
    {
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete (yysemantic_stack_[(6) - (2)].stringVal);
		delete (yysemantic_stack_[(6) - (3)].stringVal);
		delete (yysemantic_stack_[(6) - (6)].stringVal);
		}
    break;

  case 40:
/* Line 670 of lalr1.cc  */
#line 389 "..\\lazyNutGUI\\parser.y"
    {
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete (yysemantic_stack_[(4) - (2)].stringVal);
		delete (yysemantic_stack_[(4) - (4)].stringVal);
		}
    break;

  case 41:
/* Line 670 of lalr1.cc  */
#line 396 "..\\lazyNutGUI\\parser.y"
    {
		driver.queryContext.begin_query = false;
		/* match with driver.queryContext.begin */
		delete (yysemantic_stack_[(5) - (2)].stringVal);
		delete (yysemantic_stack_[(5) - (5)].stringVal);
		}
    break;

  case 50:
/* Line 670 of lalr1.cc  */
#line 415 "..\\lazyNutGUI\\parser.y"
    {
	driver.queryContext.root->removeLastChild();
	/*driver.queryContext.treeModelPtr->removeRows(driver.queryContext.treeModelPtr->rowCount()-1,1);*/
	}
    break;


/* Line 670 of lalr1.cc  */
#line 923 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
      default:
        break;
      }

    /* User semantic actions sometimes alter yychar, and that requires
       that yytoken be updated with the new translation.  We take the
       approach of translating immediately before every use of yytoken.
       One alternative is translating here after every semantic action,
       but that translation would be missed if the semantic action
       invokes YYABORT, YYACCEPT, or YYERROR immediately after altering
       yychar.  In the case of YYABORT or YYACCEPT, an incorrect
       destructor might then be invoked immediately.  In the case of
       YYERROR, subsequent parser actions might lead to an incorrect
       destructor call or verbose syntax error message before the
       lookahead is translated.  */
    YY_SYMBOL_PRINT ("-> $$ =", yyr1_[yyn], &yyval, &yyloc);

    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();

    yysemantic_stack_.push (yyval);
    yylocation_stack_.push (yyloc);

    /* Shift the result of the reduction.  */
    yyn = yyr1_[yyn];
    yystate = yypgoto_[yyn - yyntokens_] + yystate_stack_[0];
    if (0 <= yystate && yystate <= yylast_
	&& yycheck_[yystate] == yystate_stack_[0])
      yystate = yytable_[yystate];
    else
      yystate = yydefgoto_[yyn - yyntokens_];
    goto yynewstate;

  /*------------------------------------.
  | yyerrlab -- here on detecting error |
  `------------------------------------*/
  yyerrlab:
    /* Make sure we have latest lookahead translation.  See comments at
       user semantic actions for why this is necessary.  */
    yytoken = yytranslate_ (yychar);

    /* If not already recovering from an error, report this error.  */
    if (!yyerrstatus_)
      {
	++yynerrs_;
	if (yychar == yyempty_)
	  yytoken = yyempty_;
	error (yylloc, yysyntax_error_ (yystate, yytoken));
      }

    yyerror_range[1] = yylloc;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */
        if (yychar <= yyeof_)
          {
            /* Return failure if at end of input.  */
            if (yychar == yyeof_)
              YYABORT;
          }
        else
          {
            yydestruct_ ("Error: discarding", yytoken, &yylval, &yylloc);
            yychar = yyempty_;
          }
      }

    /* Else will try to reuse lookahead token after shifting the error
       token.  */
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;

    yyerror_range[1] = yylocation_stack_[yylen - 1];
    /* Do not reclaim the symbols of the rule which action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    yystate = yystate_stack_[0];
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;	/* Each real token shifted decrements this.  */

    for (;;)
      {
	yyn = yypact_[yystate];
	if (!yy_pact_value_is_default_ (yyn))
	{
	  yyn += yyterror_;
	  if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
	    {
	      yyn = yytable_[yyn];
	      if (0 < yyn)
		break;
	    }
	}

	/* Pop the current state because it cannot handle the error token.  */
	if (yystate_stack_.height () == 1)
	  YYABORT;

	yyerror_range[1] = yylocation_stack_[0];
	yydestruct_ ("Error: popping",
		     yystos_[yystate],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);
	yypop_ ();
	yystate = yystate_stack_[0];
	YY_STACK_PRINT ();
      }

    yyerror_range[2] = yylloc;
    // Using YYLLOC is tempting, but would change the location of
    // the lookahead.  YYLOC is available though.
    YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
    yysemantic_stack_.push (yylval);
    yylocation_stack_.push (yyloc);

    /* Shift the error token.  */
    YY_SYMBOL_PRINT ("Shifting", yystos_[yyn],
		     &yysemantic_stack_[0], &yylocation_stack_[0]);

    yystate = yyn;
    goto yynewstate;

    /* Accept.  */
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    /* Abort.  */
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (yychar != yyempty_)
      {
        /* Make sure we have latest lookahead translation.  See comments
           at user semantic actions for why this is necessary.  */
        yytoken = yytranslate_ (yychar);
        yydestruct_ ("Cleanup: discarding lookahead", yytoken, &yylval,
                     &yylloc);
      }

    /* Do not reclaim the symbols of the rule which action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystate_stack_.height ())
      {
        yydestruct_ ("Cleanup: popping",
                     yystos_[yystate_stack_[0]],
                     &yysemantic_stack_[0],
                     &yylocation_stack_[0]);
        yypop_ ();
      }

    return yyresult;
    }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (yychar != yyempty_)
          {
            /* Make sure we have latest lookahead translation.  See
               comments at user semantic actions for why this is
               necessary.  */
            yytoken = yytranslate_ (yychar);
            yydestruct_ (YY_NULL, yytoken, &yylval, &yylloc);
          }

        while (1 < yystate_stack_.height ())
          {
            yydestruct_ (YY_NULL,
                         yystos_[yystate_stack_[0]],
                         &yysemantic_stack_[0],
                         &yylocation_stack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  // Generate an error message.
  std::string
  Parser::yysyntax_error_ (int yystate, int yytoken)
  {
    std::string yyres;
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yytoken) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yychar.
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (yytoken != yyempty_)
      {
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            /* Stay within bounds of both yycheck and yytname.  */
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULL;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
  const signed char Parser::yypact_ninf_ = -21;
  const short int
  Parser::yypact_[] =
  {
       -21,   188,   -21,   242,   252,     4,    32,   -21,    -8,   -21,
     -21,   -21,   -21,   -21,   -21,   300,   300,   300,   265,    50,
      44,   173,     4,   -21,    13,    -6,   307,    32,   -21,   -21,
     232,   182,   118,   202,     4,   212,     1,     1,     1,     1,
       1,   300,   -21,   300,   300,    50,    44,   -21,     9,     4,
     300,   300,   274,   287,   -21,   -21,   -21,   232,   300,    59,
       4,   -21,   -21,   -21,   -21,   -21,   232,   222,   124,    12,
     300,   142,   160,    -6,   160,   -21,   300,   232,   294,   232,
     300,   -21,   -21,   -21,   232,   232,   300,   232,   232,   232
  };

  /* YYDEFACT[S] -- default reduction number in state S.  Performed when
     YYTABLE doesn't specify something else to do.  Zero means the
     default is an error.  */
  const unsigned char
  Parser::yydefact_[] =
  {
        46,     0,    52,     0,    30,     0,     0,    36,    47,    48,
      49,     2,     3,     4,     6,     0,     0,     0,     5,    12,
      13,     0,     0,     5,    31,    32,     0,     0,    50,    37,
      44,     0,     0,     6,     0,     0,     0,     0,     0,     0,
       0,     0,    16,     0,     0,    14,    15,    33,     0,     0,
      23,     0,     6,     0,    51,    17,    19,    28,     0,     6,
       0,     9,     7,     8,    10,    11,    45,     0,     0,     0,
      34,     5,    21,    25,     0,    26,     0,    40,     6,    42,
       0,    18,    20,    24,    35,    41,     0,    38,    43,    39
  };

  /* YYPGOTO[NTERM-NUM].  */
  const signed char
  Parser::yypgoto_[] =
  {
       -21,    31,    20,    82,    -4,   -20,   -21,    -7,    17,    -3,
     -21,   -21
  };

  /* YYDEFGOTO[NTERM-NUM].  */
  const signed char
  Parser::yydefgoto_[] =
  {
        -1,    23,    19,    20,    21,    25,     6,     7,     8,     9,
      10,     1
  };

  /* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule which
     number is the opposite.  If YYTABLE_NINF_, syntax error.  */
  const signed char Parser::yytable_ninf_ = -30;
  const signed char
  Parser::yytable_[] =
  {
        24,    29,     4,    28,    11,    12,    13,    11,    12,    14,
      51,    30,    31,    32,    35,    69,    11,    12,    13,    83,
      29,    14,    53,    27,    54,    48,    49,     0,    50,    57,
      73,    75,    42,    43,    18,    44,    26,    66,     0,    67,
      68,    45,     4,     5,    45,     0,    72,    74,    77,     0,
      45,    45,    45,    47,    79,    45,    61,    62,    63,    64,
      65,    39,    40,     0,    36,    58,    84,    37,    38,   -27,
      -6,     0,    85,    45,    87,     0,    88,    45,     0,     0,
      70,    71,    89,     0,     0,     0,    45,    45,    45,     0,
       0,    80,    45,     0,    45,     0,     0,    45,     0,    45,
       0,     0,     0,    46,    45,    45,    46,    45,    45,    45,
       0,     0,    46,    46,    46,     0,     0,    46,     0,     0,
       0,    11,    12,    13,     0,     0,    14,    11,    12,    13,
       0,     0,    14,     0,     0,    46,     0,    42,    43,    46,
      44,    56,    -5,    42,    43,     0,    44,    82,    46,    46,
      46,    -5,    -5,    -5,    46,     0,    46,     0,    -5,    46,
       0,    46,     0,    11,    12,    13,    46,    46,    14,    46,
      46,    46,    48,     0,     0,    50,    11,    12,    13,    42,
      43,    14,    44,     0,    41,    11,    12,    13,     2,     0,
      14,     0,    42,    43,     0,    44,     0,     3,     4,     5,
       0,    42,    43,    55,    44,    11,    12,    13,     0,     0,
      14,     0,   -29,    -6,     0,    11,    12,    13,     0,     0,
      59,     0,    16,    60,    17,    11,    12,    13,     0,     0,
      14,    42,    43,     0,    44,    11,    12,    13,     0,     0,
      14,    42,    43,    81,    44,    11,    12,    13,     0,     0,
      14,    42,    43,    15,    44,    11,    12,    13,     0,     0,
      14,     0,    16,     0,    17,    22,     0,     0,    11,    12,
      13,     0,    16,    33,    17,     0,    34,    11,    12,    13,
       0,     0,    14,     0,     0,    16,     0,    17,     0,     0,
      11,    12,    13,    76,    16,    78,    17,    11,    12,    13,
       0,     0,    14,    11,    12,    13,    42,    43,    14,    44,
      11,    12,    13,    86,    16,    52,    17,     0,     0,     0,
      16,     0,    17,     0,     0,     0,     0,    16,     0,    17
  };

  /* YYCHECK.  */
  const signed char
  Parser::yycheck_[] =
  {
         4,     8,    10,     6,     3,     4,     5,     3,     4,     8,
      16,    15,    16,    17,    18,     6,     3,     4,     5,     7,
      27,     8,    26,     6,    27,    12,    13,    -1,    15,    33,
      50,    51,    19,    20,     3,    22,     5,    41,    -1,    43,
      44,    21,    10,    11,    24,    -1,    50,    51,    52,    -1,
      30,    31,    32,    22,    58,    35,    36,    37,    38,    39,
      40,    17,    18,    -1,    14,    34,    70,    17,    18,    10,
      11,    -1,    76,    53,    78,    -1,    80,    57,    -1,    -1,
      49,    50,    86,    -1,    -1,    -1,    66,    67,    68,    -1,
      -1,    60,    72,    -1,    74,    -1,    -1,    77,    -1,    79,
      -1,    -1,    -1,    21,    84,    85,    24,    87,    88,    89,
      -1,    -1,    30,    31,    32,    -1,    -1,    35,    -1,    -1,
      -1,     3,     4,     5,    -1,    -1,     8,     3,     4,     5,
      -1,    -1,     8,    -1,    -1,    53,    -1,    19,    20,    57,
      22,    23,     0,    19,    20,    -1,    22,    23,    66,    67,
      68,     9,    10,    11,    72,    -1,    74,    -1,    16,    77,
      -1,    79,    -1,     3,     4,     5,    84,    85,     8,    87,
      88,    89,    12,    -1,    -1,    15,     3,     4,     5,    19,
      20,     8,    22,    -1,    11,     3,     4,     5,     0,    -1,
       8,    -1,    19,    20,    -1,    22,    -1,     9,    10,    11,
      -1,    19,    20,    21,    22,     3,     4,     5,    -1,    -1,
       8,    -1,    10,    11,    -1,     3,     4,     5,    -1,    -1,
       8,    -1,    20,    11,    22,     3,     4,     5,    -1,    -1,
       8,    19,    20,    -1,    22,     3,     4,     5,    -1,    -1,
       8,    19,    20,    21,    22,     3,     4,     5,    -1,    -1,
       8,    19,    20,    11,    22,     3,     4,     5,    -1,    -1,
       8,    -1,    20,    -1,    22,    13,    -1,    -1,     3,     4,
       5,    -1,    20,     8,    22,    -1,    11,     3,     4,     5,
      -1,    -1,     8,    -1,    -1,    20,    -1,    22,    -1,    -1,
       3,     4,     5,    19,    20,     8,    22,     3,     4,     5,
      -1,    -1,     8,     3,     4,     5,    19,    20,     8,    22,
       3,     4,     5,    19,    20,     8,    22,    -1,    -1,    -1,
      20,    -1,    22,    -1,    -1,    -1,    -1,    20,    -1,    22
  };

  /* STOS_[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
  const unsigned char
  Parser::yystos_[] =
  {
         0,    35,     0,     9,    10,    11,    30,    31,    32,    33,
      34,     3,     4,     5,     8,    11,    20,    22,    25,    26,
      27,    28,    13,    25,    28,    29,    25,    32,    33,    31,
      28,    28,    28,     8,    11,    28,    14,    17,    18,    17,
      18,    11,    19,    20,    22,    26,    27,    25,    12,    13,
      15,    16,     8,    28,    33,    21,    23,    28,    25,     8,
      11,    26,    26,    26,    26,    26,    28,    28,    28,     6,
      25,    25,    28,    29,    28,    29,    19,    28,     8,    28,
      25,    21,    23,     7,    28,    28,    19,    28,    28,    28
  };

#if YYDEBUG
  /* TOKEN_NUMBER_[YYLEX-NUM] -- Internal symbol number corresponding
     to YYLEX-NUM.  */
  const unsigned short int
  Parser::yytoken_number_[] =
  {
         0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    58,    59,    92,    47,    44,
      40,    41,    91,    93
  };
#endif

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
  const unsigned char
  Parser::yyr1_[] =
  {
         0,    24,    25,    25,    26,    26,    26,    27,    27,    27,
      27,    27,    28,    28,    28,    28,    28,    28,    28,    28,
      28,    29,    29,    29,    29,    29,    29,    30,    30,    30,
      31,    31,    31,    31,    31,    31,    32,    32,    33,    33,
      33,    33,    34,    34,    34,    34,    35,    35,    35,    35,
      35,    35,    35
  };

  /* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
  const unsigned char
  Parser::yyr2_[] =
  {
         0,     2,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     1,     1,     2,     2,     2,     3,     4,     3,
       4,     3,     3,     2,     4,     3,     3,     4,     4,     3,
       1,     2,     2,     3,     4,     5,     1,     2,     5,     6,
       4,     5,     5,     6,     3,     4,     0,     2,     2,     2,
       3,     4,     2
  };


  /* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
     First, the terminals, then, starting at \a yyntokens_, nonterminals.  */
  const char*
  const Parser::yytname_[] =
  {
    "\"end of file\"", "error", "$undefined", "\"integer\"", "\"double\"",
  "\"word\"", "\"object type\"", "\"object subtype\"", "\"query type\"",
  "BEGIN_QUERY", "ANSWER", "END_QUERY", "IS_A", "LIST_OF",
  "COLON_BACKSLASH", "':'", "';'", "'\\\\'", "'/'", "','", "'('", "')'",
  "'['", "']'", "$accept", "number", "string", "fullpath", "text",
  "label_content", "begin_query", "answer", "answers", "end_query",
  "query_syntax_error", "start", YY_NULL
  };

#if YYDEBUG
  /* YYRHS -- A `-1'-separated list of the rules' RHS.  */
  const Parser::rhs_number_type
  Parser::yyrhs_[] =
  {
        35,     0,    -1,     3,    -1,     4,    -1,     5,    -1,    25,
      -1,     8,    -1,    26,    17,    26,    -1,    26,    18,    26,
      -1,    26,    14,    26,    -1,    27,    17,    26,    -1,    27,
      18,    26,    -1,    26,    -1,    27,    -1,    28,    26,    -1,
      28,    27,    -1,    28,    19,    -1,    20,    28,    21,    -1,
      28,    20,    28,    21,    -1,    22,    28,    23,    -1,    28,
      22,    28,    23,    -1,    28,    15,    28,    -1,    28,    15,
      25,    -1,    28,    15,    -1,    28,    12,     6,     7,    -1,
      28,    15,    29,    -1,    29,    16,    29,    -1,     9,    25,
      28,     8,    -1,     9,    25,     8,    28,    -1,     9,    25,
       8,    -1,    10,    -1,    10,    28,    -1,    10,    29,    -1,
      10,    13,    25,    -1,    10,    28,    13,    25,    -1,    10,
      28,    13,    25,    28,    -1,    31,    -1,    32,    31,    -1,
      11,    25,    28,     8,    28,    -1,    11,    25,    28,     8,
      19,    28,    -1,    11,    25,     8,    28,    -1,    11,    25,
       8,    19,    28,    -1,     9,    25,    11,    25,    28,    -1,
       9,    25,    28,    11,    25,    28,    -1,     9,    11,    28,
      -1,     9,    28,    11,    28,    -1,    -1,    35,    32,    -1,
      35,    33,    -1,    35,    34,    -1,    35,    30,    33,    -1,
      35,    30,    32,    33,    -1,    35,     0,    -1
  };

  /* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
     YYRHS.  */
  const unsigned char
  Parser::yyprhs_[] =
  {
         0,     0,     3,     5,     7,     9,    11,    13,    17,    21,
      25,    29,    33,    35,    37,    40,    43,    46,    50,    55,
      59,    64,    68,    72,    75,    80,    84,    88,    93,    98,
     102,   104,   107,   110,   114,   119,   125,   127,   130,   136,
     143,   148,   154,   160,   167,   171,   176,   177,   180,   183,
     186,   190,   195
  };

  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
  const unsigned short int
  Parser::yyrline_[] =
  {
         0,   124,   124,   128,   137,   146,   150,   156,   162,   168,
     174,   180,   187,   191,   195,   201,   207,   212,   217,   223,
     228,   235,   243,   251,   258,   277,   286,   294,   305,   316,
     331,   332,   349,   363,   364,   365,   368,   369,   372,   380,
     388,   395,   403,   404,   405,   406,   410,   411,   412,   413,
     414,   419,   420
  };

  // Print the state stack on the debug stream.
  void
  Parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (state_stack_type::const_iterator i = yystate_stack_.begin ();
	 i != yystate_stack_.end (); ++i)
      *yycdebug_ << ' ' << *i;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  Parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    /* Print the symbols being reduced, and their result.  */
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
	       << " (line " << yylno << "):" << std::endl;
    /* The symbols being reduced.  */
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
		       yyrhs_[yyprhs_[yyrule] + yyi],
		       &(yysemantic_stack_[(yynrhs) - (yyi + 1)]),
		       &(yylocation_stack_[(yynrhs) - (yyi + 1)]));
  }
#endif // YYDEBUG

  /* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
  Parser::token_number_type
  Parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
           0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      20,    21,     2,     2,    19,     2,     2,    18,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    15,    16,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    22,    17,    23,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14
    };
    if ((unsigned int) t <= yyuser_token_number_max_)
      return translate_table[t];
    else
      return yyundef_token_;
  }

  const int Parser::yyeof_ = 0;
  const int Parser::yylast_ = 329;
  const int Parser::yynnts_ = 12;
  const int Parser::yyempty_ = -2;
  const int Parser::yyfinal_ = 2;
  const int Parser::yyterror_ = 1;
  const int Parser::yyerrcode_ = 256;
  const int Parser::yyntokens_ = 24;

  const unsigned int Parser::yyuser_token_number_max_ = 269;
  const Parser::token_number_type Parser::yyundef_token_ = 2;


} // lazyNutOutputParser
/* Line 1141 of lalr1.cc  */
#line 1557 "C:/Top/Simulators/Qt/easyNet/lazyNutGUI/parser.cpp"
/* Line 1142 of lalr1.cc  */
#line 425 "..\\lazyNutGUI\\parser.y"
 /*** Additional Code ***/

void lazyNutOutputParser::Parser::error(const Parser::location_type& l,
			    const std::string& m)
{
    driver.error(l, m);
}

