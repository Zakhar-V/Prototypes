/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
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

#ifndef YY_YY_GLSLANG_TAB_CPP_H_INCLUDED
# define YY_YY_GLSLANG_TAB_CPP_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ATTRIBUTE = 258,
     VARYING = 259,
     CONST = 260,
     BOOL = 261,
     FLOAT = 262,
     DOUBLE = 263,
     INT = 264,
     UINT = 265,
     BREAK = 266,
     CONTINUE = 267,
     DO = 268,
     ELSE = 269,
     FOR = 270,
     IF = 271,
     DISCARD = 272,
     RETURN = 273,
     SWITCH = 274,
     CASE = 275,
     DEFAULT = 276,
     SUBROUTINE = 277,
     BVEC2 = 278,
     BVEC3 = 279,
     BVEC4 = 280,
     IVEC2 = 281,
     IVEC3 = 282,
     IVEC4 = 283,
     UVEC2 = 284,
     UVEC3 = 285,
     UVEC4 = 286,
     VEC2 = 287,
     VEC3 = 288,
     VEC4 = 289,
     MAT2 = 290,
     MAT3 = 291,
     MAT4 = 292,
     CENTROID = 293,
     IN = 294,
     OUT = 295,
     INOUT = 296,
     UNIFORM = 297,
     PATCH = 298,
     SAMPLE = 299,
     BUFFER = 300,
     SHARED = 301,
     COHERENT = 302,
     VOLATILE = 303,
     RESTRICT = 304,
     READONLY = 305,
     WRITEONLY = 306,
     DVEC2 = 307,
     DVEC3 = 308,
     DVEC4 = 309,
     DMAT2 = 310,
     DMAT3 = 311,
     DMAT4 = 312,
     NOPERSPECTIVE = 313,
     FLAT = 314,
     SMOOTH = 315,
     LAYOUT = 316,
     MAT2X2 = 317,
     MAT2X3 = 318,
     MAT2X4 = 319,
     MAT3X2 = 320,
     MAT3X3 = 321,
     MAT3X4 = 322,
     MAT4X2 = 323,
     MAT4X3 = 324,
     MAT4X4 = 325,
     DMAT2X2 = 326,
     DMAT2X3 = 327,
     DMAT2X4 = 328,
     DMAT3X2 = 329,
     DMAT3X3 = 330,
     DMAT3X4 = 331,
     DMAT4X2 = 332,
     DMAT4X3 = 333,
     DMAT4X4 = 334,
     ATOMIC_UINT = 335,
     SAMPLER1D = 336,
     SAMPLER2D = 337,
     SAMPLER3D = 338,
     SAMPLERCUBE = 339,
     SAMPLER1DSHADOW = 340,
     SAMPLER2DSHADOW = 341,
     SAMPLERCUBESHADOW = 342,
     SAMPLER1DARRAY = 343,
     SAMPLER2DARRAY = 344,
     SAMPLER1DARRAYSHADOW = 345,
     SAMPLER2DARRAYSHADOW = 346,
     ISAMPLER1D = 347,
     ISAMPLER2D = 348,
     ISAMPLER3D = 349,
     ISAMPLERCUBE = 350,
     ISAMPLER1DARRAY = 351,
     ISAMPLER2DARRAY = 352,
     USAMPLER1D = 353,
     USAMPLER2D = 354,
     USAMPLER3D = 355,
     USAMPLERCUBE = 356,
     USAMPLER1DARRAY = 357,
     USAMPLER2DARRAY = 358,
     SAMPLER2DRECT = 359,
     SAMPLER2DRECTSHADOW = 360,
     ISAMPLER2DRECT = 361,
     USAMPLER2DRECT = 362,
     SAMPLERBUFFER = 363,
     ISAMPLERBUFFER = 364,
     USAMPLERBUFFER = 365,
     SAMPLERCUBEARRAY = 366,
     SAMPLERCUBEARRAYSHADOW = 367,
     ISAMPLERCUBEARRAY = 368,
     USAMPLERCUBEARRAY = 369,
     SAMPLER2DMS = 370,
     ISAMPLER2DMS = 371,
     USAMPLER2DMS = 372,
     SAMPLER2DMSARRAY = 373,
     ISAMPLER2DMSARRAY = 374,
     USAMPLER2DMSARRAY = 375,
     SAMPLEREXTERNALOES = 376,
     IMAGE1D = 377,
     IIMAGE1D = 378,
     UIMAGE1D = 379,
     IMAGE2D = 380,
     IIMAGE2D = 381,
     UIMAGE2D = 382,
     IMAGE3D = 383,
     IIMAGE3D = 384,
     UIMAGE3D = 385,
     IMAGE2DRECT = 386,
     IIMAGE2DRECT = 387,
     UIMAGE2DRECT = 388,
     IMAGECUBE = 389,
     IIMAGECUBE = 390,
     UIMAGECUBE = 391,
     IMAGEBUFFER = 392,
     IIMAGEBUFFER = 393,
     UIMAGEBUFFER = 394,
     IMAGE1DARRAY = 395,
     IIMAGE1DARRAY = 396,
     UIMAGE1DARRAY = 397,
     IMAGE2DARRAY = 398,
     IIMAGE2DARRAY = 399,
     UIMAGE2DARRAY = 400,
     IMAGECUBEARRAY = 401,
     IIMAGECUBEARRAY = 402,
     UIMAGECUBEARRAY = 403,
     IMAGE2DMS = 404,
     IIMAGE2DMS = 405,
     UIMAGE2DMS = 406,
     IMAGE2DMSARRAY = 407,
     IIMAGE2DMSARRAY = 408,
     UIMAGE2DMSARRAY = 409,
     STRUCT = 410,
     VOID = 411,
     WHILE = 412,
     IDENTIFIER = 413,
     TYPE_NAME = 414,
     FLOATCONSTANT = 415,
     DOUBLECONSTANT = 416,
     INTCONSTANT = 417,
     UINTCONSTANT = 418,
     BOOLCONSTANT = 419,
     FIELD_SELECTION = 420,
     LEFT_OP = 421,
     RIGHT_OP = 422,
     INC_OP = 423,
     DEC_OP = 424,
     LE_OP = 425,
     GE_OP = 426,
     EQ_OP = 427,
     NE_OP = 428,
     AND_OP = 429,
     OR_OP = 430,
     XOR_OP = 431,
     MUL_ASSIGN = 432,
     DIV_ASSIGN = 433,
     ADD_ASSIGN = 434,
     MOD_ASSIGN = 435,
     LEFT_ASSIGN = 436,
     RIGHT_ASSIGN = 437,
     AND_ASSIGN = 438,
     XOR_ASSIGN = 439,
     OR_ASSIGN = 440,
     SUB_ASSIGN = 441,
     LEFT_PAREN = 442,
     RIGHT_PAREN = 443,
     LEFT_BRACKET = 444,
     RIGHT_BRACKET = 445,
     LEFT_BRACE = 446,
     RIGHT_BRACE = 447,
     DOT = 448,
     COMMA = 449,
     COLON = 450,
     EQUAL = 451,
     SEMICOLON = 452,
     BANG = 453,
     DASH = 454,
     TILDE = 455,
     PLUS = 456,
     STAR = 457,
     SLASH = 458,
     PERCENT = 459,
     LEFT_ANGLE = 460,
     RIGHT_ANGLE = 461,
     VERTICAL_BAR = 462,
     CARET = 463,
     AMPERSAND = 464,
     QUESTION = 465,
     INVARIANT = 466,
     PRECISE = 467,
     HIGH_PRECISION = 468,
     MEDIUM_PRECISION = 469,
     LOW_PRECISION = 470,
     PRECISION = 471,
     PACKED = 472,
     RESOURCE = 473,
     SUPERP = 474
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2058 of yacc.c  */
#line 66 "glslang.y"

    struct {
        glslang::TSourceLoc loc;
        union {
            glslang::TString *string;
            int i;
            unsigned int u;
            bool b;
            double d;
        };
        glslang::TSymbol* symbol;
    } lex;
    struct {
        glslang::TSourceLoc loc;
        glslang::TOperator op;
        union {
            TIntermNode* intermNode;
            glslang::TIntermNodePair nodePair;
            glslang::TIntermTyped* intermTypedNode;
        };
        union {
            glslang::TPublicType type;
            glslang::TFunction* function;
            glslang::TParameter param;
            glslang::TTypeLoc typeLine;
            glslang::TTypeList* typeList;
            glslang::TArraySizes* arraySizes;
            glslang::TIdentifierList* identifierList;
        };
    } interm;


/* Line 2058 of yacc.c  */
#line 309 "glslang_tab.cpp.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (glslang::TParseContext* pParseContext);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_GLSLANG_TAB_CPP_H_INCLUDED  */
