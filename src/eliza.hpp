
#ifndef ELIZA_HPP
#define ELIZA_HPP

#include <string.h>
#include "chatmain.hpp"
#include "allkeys.hpp"

#ifdef DMALLOC
# include "dmalloc.h"
#endif

#define USE_EX_SEARCH	
		

class eliza
{
public:
	enum { maxdbases=100,maxnames=200,defaultdbase=0};
	enum { repsize=150}; 

	struct nametype 
	{
		char* name;
		int dbase;

		char *set( char *n, int d ) 
		{ 
			if( name != NULL )
				free( name );

			name = strdup(n);
			dbase = d;
			return name; 
		}

		nametype() 
		{
			name=NULL;
			dbase=0; 
		}

		~nametype()
		{
			if(name) 
			{
				free(name); 
				name = NULL;
			}
		}
  };
  static char *trim( char str[] );
protected:
	int numdbases,numnames;
	allkeys thekeys[maxdbases];
	nametype thenames[maxnames];

	int doop(char op,int a,int b);
	int lowcase(char ch);
	int strpos(char *s,char *sub);
	int match(char s[],char m[],int& in,int&);
	
	bool addname(char*,int);  
	bool addbunch(char*, int); 
	void sortnames();
	int getname(char*);     
	int getanyname(char*);  
	
public:
	void reducespaces(char *);
	void eliza::addrest(char* replied, char* talker,
							  char* rep, char* target,char* rest);
public:
	bool loaddata( char *, char recurflag = 0 );
	char* processdbase(char* talker,char* message,char* target,int dbase);
	char* process(char* talker,char* message,char* target)    
	{
#ifdef USE_EX_SEARCH	
		return processdbase(talker,message,target,getanyname(talker));
#else
		return processdbase(talker,message,target,getname(talker));
#endif
	}

	eliza() 
	{
		numdbases = 0; 
		numnames = 0;
		addname("default",0); 
	}
	~eliza();
};

#endif 



