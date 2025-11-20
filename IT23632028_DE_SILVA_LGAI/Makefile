# Makefile for TestLangPP (Windows / MinGW)

all: parser

# Build parser
parser: parser.tab.c lex.yy.c
	gcc -o parser parser.tab.c lex.yy.c

# Generate Bison parser files
parser.tab.c parser.tab.h: parser.y
	win_bison -d parser.y

# Generate Flex lexer file
lex.yy.c: lexer.l
	win_flex lexer.l

# Clean generated files
clean:
	del parser.exe parser.tab.c parser.tab.h lex.yy.c
