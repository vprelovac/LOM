/******************************************************************************
 Snippet: Soundex parser.
 Author:  Richard Woolcock (aka KaVir).
 Date:    20th December 2000.
 ******************************************************************************
 This code is copyright (C) 2000 by Richard Woolcock.  It may be used and
 distributed freely, as long as you don't remove this copyright notice.
 ******************************************************************************/

/******************************************************************************
 Remove the following line to use this code in your mud.
 ******************************************************************************/

//#define STANDALONE_PROGRAM

/******************************************************************************
 Required library files.
 ******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char *GetSoundexKey ( const char *szTxt );
int   SoundexMatch  ( char *szFirst, char *szSecond );

/******************************************************************************
 Local literals
 ******************************************************************************/

#define KEY_SIZE 5 /* Size of the soundex key */

/******************************************************************************
 Local operation prototypes.
 ******************************************************************************/

static char LetterConversion ( char chLetter );

/******************************************************************************
 Standalone main function.
 ******************************************************************************/

#ifdef STANDALONE_PROGRAM
int main( int iArgCount, char *a_szArgument[] )
{
   int iMatch; /* Percentage match between the two arguments */

   /* Make sure there are the correct number of arguments (2 + program name) */
   if ( iArgCount == 3 )
   {
      /* Retain the pointers to the soundex keys */
      char *szArg[2] = { GetSoundexKey( a_szArgument[1] ), GetSoundexKey( a_szArgument[2] ) };

      /* Calculate the percentage match between the two arguments */
      iMatch = SoundexMatch( GetSoundexKey( a_szArgument[1] ), GetSoundexKey( a_szArgument[2] ) );

      /* Display the result */
      printf( "Soundex match between keys '%s' and '%s' is %d%%\n", szArg[0], szArg[1], iMatch );
   }
   else /* iArgCount != 3 */
   {
      /* Inform the user of the correct command syntax */
      printf( "Syntax: %s <word> <word>\n", a_szArgument[0] );
   }

   /* Indicate successful termination of program */
   return ( 0 );
}
#endif

/******************************************************************************
 Global operations.
 ******************************************************************************/

/* Function: GetSoundexKey
 *
 * This function determines a soundex key from the string argument and returns 
 * the address of the key (which is stored in a static variable).  Because the 
 * most common use of the soundex key is to compare it with _another_ soundex 
 * key, this function uses two internal storage buffers, which are alternated 
 * between every time the function is called.
 *
 * The function takes one parameter, as follows:
 *
 * szTxt: A pointer to the string from which the soundex key is calculated.
 *
 * The return value is a pointer to the soundex key string.
 */
char *GetSoundexKey( const char *szTxt )
{
   int iOldIndex = 0; /* Loop index for the old (szTxt) string */
   int iNewIndex = 0; /* Loop index for the new (s_a_chSoundex) string */
   static char s_a_chSoundex[2][KEY_SIZE+1]; /* Stores the new string */
   static unsigned iSoundex; /* Determines which s_a_chSoundex is used */

   iSoundex++; /* Switch to the other s_a_chSoundex array */

   s_a_chSoundex[iSoundex%2][0] = '\0'; /* Clear any previous data */

   /* Copy the first character without conversion */
   if ( ( s_a_chSoundex[iSoundex%2][iNewIndex++] = tolower(szTxt[iOldIndex++]) ) )
   {
      do /* Loop through szTxt */
      {
         char chLetter; /* Stores the soundex value of a letter */

         /* Double/triple/etc letters are treated as single letters */
         while ( tolower(szTxt[iOldIndex]) == tolower(szTxt[iOldIndex+1]) )
         {
            iOldIndex++;
            continue;
         }

         /* Convert the letter into its soundex value and store it */
         chLetter = LetterConversion((char)tolower(szTxt[iOldIndex]));

         /* Ignore NUL and 0 characters and only store KEY_SIZE characters */
         if ( chLetter != '\0' && chLetter != '0' && iNewIndex < KEY_SIZE )
         {
            /* Store the soundex value */
            s_a_chSoundex[iSoundex%2][iNewIndex++] = chLetter;
         }
      }
      while ( szTxt[iOldIndex++] != '\0' );

      /* If less than KEY_SIZE characters were copied, buffer with zeros */
      while ( iNewIndex < KEY_SIZE )
      {
         s_a_chSoundex[iSoundex%2][iNewIndex++] = '0';
      }

      /* Add the NUL terminator to the end of the soundex string */
      s_a_chSoundex[iSoundex%2][iNewIndex] = '\0';
   }

   /* Return the address of the soundex string */
   return ( s_a_chSoundex[iSoundex%2] );
}

/* Function: SoundexMatch
 *
 * This function compares two soundex keys and returns a percentage match.
 *
 * The function takes two parameters, as follows:
 *
 * szFirst:  A pointer to the first soundex key.
 * szSecond: A pointer to the second soundex key.
 *
 * The return value is an integer which contains the percentage match.
 */
int SoundexMatch( char *szFirst, char *szSecond )
{
   int iMatch = 0; /* Number of matching characters found */
   int iMax   = 0; /* Total number of characters compared */

   /* Make sure that both strings are of the correct size */
   if ( strlen( szFirst ) == KEY_SIZE && strlen( szSecond ) == KEY_SIZE )
   {
      int i; /* Loop counter */

      /* Loop through both strings */
      for ( i = 0; i < KEY_SIZE; i++ )
      {
         /* If either of the values are not NUL */
         if ( szFirst[i] != '0' || szSecond[i] != '0' )
         {
            iMax++; /* Increment the maximum */
         }

         /* If BOTH values are not NUL */
         if ( szFirst[i] != '0' && szSecond[i] != '0' )
         {
            /* Check for a match */
            if ( szFirst[i] == szSecond[i] )
            {
               iMatch++; /* A match was found */
            }
         }
      }
   }

   /* Return the percentage match */
   return ( iMatch * 100 / iMax );
}

/******************************************************************************
 Local operations.
 ******************************************************************************/

/* Function: LetterConversion
 *
 * This function converts a single letter into it's appropriate soundex value.
 *
 * The function takes one parameter, as follows:
 *
 * chLetter: The letter to be converted.
 *
 * The return value is a single character which contains the converted value.
 */
static char LetterConversion( char chLetter )
{
   const char * kszSoundexData = "01230120022455012623010202";
   char chResult; /* Store the soundex value, or NUL */

   if ( islower(chLetter) )
   {
      /* Determine the soundex value associated with the letter */
      chResult = kszSoundexData[ (chLetter - 'a') ];
   }
   else /* it's not a lowercase letter */
   {
      /* NUL means there is no associated soundex value */
      chResult = '\0';
   }

   /* Return the soundex value, or NUL if there isn't one */
   return ( chResult );
}

