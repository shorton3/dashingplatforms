/******************************************************************************
*
* File name:   VersionId.cpp
* Subsystem:   Utility Tool
* Description: Implements the SCCS 'what' utility functionality required to
*              display the build version. Each file must implement a static
*              char* that contains "@(#)" followed by the module description
*              and version.
*
* Name                 Date       Release
* -------------------- ---------- ---------------------------------------------
* Stephen Horton       01/01/2014 Initial release
*
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// System include files, includes 3rd party libraries.
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

//-----------------------------------------------------------------------------
// Component includes, includes elements of our system.
//-----------------------------------------------------------------------------


/**
 * Implements the SCCS 'what' utility functionality required to
 * display the build version. Each file must implement a static
 * char* that contains "@(#)" followed by the module description
 * and version.
 * <p>
 * Test data for cases: to use, run "./VersionId VersionId.c"
 * Note: J is not a recognised ID keyword in standard SCCS, so the four
 *     GOT THIS lines should contain the characters '%', 'J', '%'.
 * <p>
 * @(#@(#)GOT IT? -- 1
 * @(@(#)GOT IT? -- 2
 * @@(#)GOT IT? -- 3
 * %A@(#)GOT IT? -- 4
 * %@(#)GOT IT? -- 5
 * @(#%J%GOT THIS? -- 1
 * @(%J%GOT THIS? -- 2
 * @%J%GOT THIS? -- 3
 * %%J%GOT THIS? -- 4
 * @(#)Should have seen GOT IT 5 times
 * @(#)Should have seen GOT THIS 4 times
 * <p>
 * $Author: Stephen Horton$
 * $Revision: 1$
 */

//-----------------------------------------------------------------------------
// Static Declarations.
//-----------------------------------------------------------------------------

static char usestr[] = "[-hsV] file [...]";
static char *filename;
static int headings = 0;
static int strict = 0;

#define StrConvert(x) #x
#define XstrConvert(x) StrConvert(x)
static volatile char main_sccs_id[] __attribute__ ((unused))="@(#)VersionId"
   "\n  Build Label: " XstrConvert(BUILD_LABEL)
   "\n  Compile Time: " __DATE__" "__TIME__;

//-----------------------------------------------------------------------------
// Function Type: STATIC
// Description: 
// Design:     
//-----------------------------------------------------------------------------
static void dostring(FILE *fp)
{
   int c;
   while ((c = getc(fp)) != EOF)
   {
      if (c == '\0' || strchr("\"\\>", c) || !(isprint(c) || isspace(c)))
         break;
      putchar(c);
   }//end while
   putchar('\n');
}//end dostring


//-----------------------------------------------------------------------------
// Function Type: STATIC
// Description: 
// Design:     
//-----------------------------------------------------------------------------
static void whatstr(FILE *fp)
{
   char c;
   if ((c = getc(fp)) != '(' || (c = getc(fp)) != '#' || (c = getc(fp)) != ')')
   {
      ungetc(c, fp);
      return;
   }//end if

   if (headings)
      printf("%s: ", filename);
   else
      putchar('\t');

   dostring(fp);
}//end whatstr


//-----------------------------------------------------------------------------
// Function Type: STATIC
// Description:  
// Design:     
//-----------------------------------------------------------------------------
static void sccsstr(FILE *fp)
{
   int c;
   int c2;
   if (!isupper(c = getc(fp)))
   {
      ungetc(c, fp);
      return;
   }//end if
   if ((c2 = getc(fp)) != '%')
   {
      ungetc(c2, fp);
      return;
   }//end if

   if (headings)
      printf("%s: ", filename);
   else
      putchar('\t');
   putchar('%');
   putchar(c);
   putchar('%');
   dostring(fp);
}//end sccsstr


//-----------------------------------------------------------------------------
// Function Type: STATIC
// Description:
// Design:
//-----------------------------------------------------------------------------
static void what(FILE *fp, char *file)
{
   int c;
   filename = file;
   if (!headings)
      printf("%s:\n", filename);
   while ((c = getc(fp)) != EOF)
   {
      if (c == '@')
         whatstr(fp);
      else if (c == '%' && strict == 0)
         sccsstr(fp);
   }//end while
}//end what


//-----------------------------------------------------------------------------
// Function Type: Main Function
// Description:
// Design:
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
   int opt;
   FILE* fd;
   opterr = 0;
   while ((opt = getopt(argc, argv, "hsV")) != EOF)
   {
      switch (opt)
      {
         case 'h':
            headings = 1;
            break;
         case 's':
            strict = 1;
            break;
         case 'V':
            printf("VersionId Revision: 1.0");
            break;
         default:
            printf("%s %s", argv[0], usestr);
            break;
      }//end switch
   }//end while

   if (optind >= argc)
      printf("%s: Invalid options: %s", argv[0], usestr);

   fd = fopen(argv[1], "r");
   if (fd == NULL)
   {
      printf("File not found. Errno: %s.\n%s", strerror(errno), usestr);
      exit (EXIT_FAILURE);
   }//end if
   // Parse the file for the identifier
   what (fd, argv[1]);
   return (EXIT_SUCCESS);
}//end main

#ifdef __cplusplus
}
#endif

