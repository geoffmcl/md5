/*
  Copyright (C) 2002 Aladdin Enterprises.  All rights reserved.

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  L. Peter Deutsch
  ghost@aladdin.com

 */
/* $Id: md5main.c,v 1.1 2002/04/13 19:20:28 lpd Exp $ */
/*
  Independent implementation of MD5 (RFC 1321).

  This code implements the MD5 Algorithm defined in RFC 1321, whose
  text is available at
	http://www.ietf.org/rfc/rfc1321.txt
  The code is derived from the text of the RFC, including the test suite
  (section A.5) but excluding the rest of Appendix A.  It does not include
  any code or documentation that is identified in the RFC as being
  copyrighted.

  The original and principal author of md5.c is L. Peter Deutsch
  <ghost@aladdin.com>.  Other authors are noted in the change history
  that follows (in reverse chronological order):

  2002-04-13 lpd Splits off main program into a separate file, md5main.c.
  2008-11-26 grm Added simple file handling to interface, and compiled using
                 Microsoft Visual Studio 2005 (MSVC8), in an XP machine,
                 and later with VS 2019 (MSVC16), in x64 (64-bits).
 */

#ifdef _MSC_VER
#pragma warning(disable:4996)
#ifdef UNICODE
#undef UNICODE
#endif
#ifdef _UNICODE
#undef _UNICODE
#endif
#endif

#include <sys/types.h>  // must be this order types, then stat
#include <sys/stat.h>
#include "md5.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define  MX_BUFFER   256
#define  EndBuf(a)   ( a + strlen(a) )

/*
 * This file builds an executable that performs various functions related
 * to the MD5 library.  Typical compilation in unix:
 *	gcc -o md5main -lm md5main.c md5.c
 * in WIN32, use Md5.dsw in MSVC, or md5.sln in MSVC8, or later
 */
static const char *const usage = "\
md5: Compile on %s, at %s - Usage: [Options] file\n\
  --help     -h (or -?)  # this brief help, and exit.\n\
  --test     -t          # run the self-test (A.5 of RFC 1321), and exit.\n\
  --t-values -l          # print the T values for the library, and exit\n\
  --version  -v          # print the version of the package, and exit.\n\
  --digest=  -d=<digest> # compare file digest with this value.\n\
  --html                 # ouput information in HTML table form.\n\
  filename               # output the MD5 sum for the file.\n\
";

static const char *const version = "2020-06-10 compiled on %s, at %s\n";
//static const char* const version = "2002-04-13 compiled on %s, at %s\n";

/* Run the self-test. */
static const char *const test[7*2] = {
	"", "d41d8cd98f00b204e9800998ecf8427e",
	"a", "0cc175b9c0f1b6a831c399e269772661",
	"abc", "900150983cd24fb0d6963f7d28e17f72",
	"message digest", "f96b697d7cb7938d525a2f31aaf161d0",
	"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
				"d174ab98d277d9f5a5611c2c9f419d9f",
	"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"
    };

static int
do_test(void)
{
   int i;
   int status = 0;

   for (i = 0; i < 7*2; i += 2) {
      md5_state_t state;
      md5_byte_t digest[16];
      char hex_output[16*2 + 1];
      int di;
      size_t   len = strlen(test[i]);
      size_t   off = 0;

      // test interface using ONE byte at a time
      // =======================================
      md5_init(&state);
      while(len--)
      {
         md5_append(&state, (const md5_byte_t *)(test[i] + off), 1);
         off++;
      }
      md5_finish(&state, digest);
      // =======================================

      // generate HEX output
      for (di = 0; di < 16; ++di)
         sprintf(hex_output + di * 2, "%02x", digest[di]);

      // show waht we got
      printf("MD5 (\"%s\") = ", test[i]);
      puts(hex_output);

      // compare it to canned values
      if (strcmp(hex_output, test[i + 1])) {
         printf("**** ERROR, should be: %s\n", test[i + 1]);
         status++;
	   }
   }

   if (status == 0)
      puts("md5 self-test completed successfully.");
   else
      puts("md5 self-test FAILED, as noted above!");

   return status;
}

static int
do_test_original(void)
{
   int i;
   int status = 0;

   for (i = 0; i < 7*2; i += 2) {
      md5_state_t state;
      md5_byte_t digest[16];
      char hex_output[16*2 + 1];
      int di;

      md5_init(&state);
      md5_append(&state, (const md5_byte_t *)test[i], (int)strlen(test[i]));
      md5_finish(&state, digest);
      for (di = 0; di < 16; ++di)
         sprintf(hex_output + di * 2, "%02x", digest[di]);

      printf("MD5 (\"%s\") = ", test[i]);
      puts(hex_output);

      if (strcmp(hex_output, test[i + 1])) {
         printf("**** ERROR, should be: %s\n", test[i + 1]);
         status = 1;
	   }
   }

   if (status == 0)
      puts("md5 self-test completed successfully.");

   return status;
}

/* Print the T values. */
static int
do_t_values(void)
{
    int i;
    for (i = 1; i <= 64; ++i) {
	unsigned long v = (unsigned long)(4294967296.0 * fabs(sin((double)i)));

	/*
	 * The following nonsense is only to avoid compiler warnings about
	 * "integer constant is unsigned in ANSI C, signed with -traditional".
	 */
	if (v >> 31) {
	    printf("#define T%d /* 0x%08lx */ (T_MASK ^ 0x%08lx)\n", i,
		   v, (unsigned long)(unsigned int)(~v));
	} else {
	    printf("#define T%d    0x%08lx\n", i, v);
	}
    }
    return 0;
}

// variable for the FILE interface
static char buffer[MX_BUFFER+16];
static md5_state_t state;
static md5_byte_t digest[16];
static char hex_output[16*2 + 1];
static char * test_digest = NULL;
static char time_buffer[256];
static char number_buffer[256];
static int utc_option = 1;
static int html_output = 0;

char * get_time_string( __int64 time )
{
   char * pb = time_buffer;
   struct tm * ptm;
   ptm = utc_option ? gmtime (&time) : localtime (&time);
   sprintf (pb, "%04ld-%02d-%02d %02d:%02d:%02d %s",
		   ptm->tm_year + 1900L, ptm->tm_mon + 1, ptm->tm_mday,
		   ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
         ( utc_option ? "(utc)" : "(local)" ) );
   return pb;
}

char * get_number_string( __int64 i )
{
   char * p = number_buffer + 254;
   int   cnt = 0;
   *p = 0;
   do {
      if( cnt == 3 ) {
         *--p = ',';
         cnt = 0;
      }
      *--p = (char)('0' + (i % 10));
      cnt++;
   } while ((i /= 10) != 0);

   return p;
}

static char html_head[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
"\"http://www.w3.org/TR/html4/loose.dtd\">\n"
"<html>\n"
"<head>\n"
"<meta http-equiv=\'Content-Type\" content=\"text/html; charset=us-ascii\">\n"
"<title>MD5 for %s</title>\n"
"</head>\n"
"<body bgcolor=\"#c0ffff\">\n";

static char table_bgn[] = "<table align=\"center\"\n"
"       border=\"1\"\n"
"       cellpadding=\"2\"\n"
"       cellspacing=\"2\"\n"
"       summary=\"MD5 for %s file\">\n";

static char table_head[] = "<tr>\n"
"<th>File</th>\n"
"<th>MD5</th>\n"
"<th>Size</th>\n"
"<th>Date</th>\n"
"</tr>\n";


int   do_file( char * file, char * exe )
{
   int   rval = 0;   // assume SUCCESS
   char * cp = buffer;
   size_t read;
   FILE * pf;
   struct __stat64 buf;

   if( _stat64(file, &buf) ) {
      printf("ERROR: Can NOT stat file [%s] ... check name, location ...\n",
         file );
      return 1;
   }
   pf = fopen(file, "rb"); // open in binary mode
   if( !pf ) {
      printf("ERROR: Can NOT open file [%s] ... check name, location ...\n",
         file );
      return 1;
   }

   read = fread( cp, 1, MX_BUFFER, pf );
   if( read ) {
      int   di;
      md5_init(&state);
      do {
         md5_append(&state, (const md5_byte_t *)cp, (int)read);
      } while( (read = fread( cp, 1, MX_BUFFER, pf )) > 0 );

      md5_finish(&state, digest);

      for (di = 0; di < 16; ++di)
         sprintf(hex_output + di * 2, "%02x", digest[di]);

      if( test_digest )
      {
         if ( strcmp(hex_output, test_digest) ) {
            fprintf(stderr,"***** NOTE: Not same as: %s *****\n", test_digest);
            rval = 3;
         } else {
            fprintf(stderr,"Input digest is equal.\n");
         }
	   }

      if(html_output) {
         time_t t;
         time(&t);
         printf(html_head, file);
         printf("<h1 align=\"center\">MD5 for %s</h1>\n", file);
         printf(table_bgn, file);
         printf(table_head);
         printf("<tr>\n");
         printf("<td><a href=\"%s\">%s</a></td>\n", file, file);
         printf("<td><tt>%s</tt></td>\n", hex_output);
         printf("<td>%s</td>\n", get_number_string(buf.st_size));
         printf("<td>%s</td>\n", get_time_string(buf.st_mtime));
         printf("</tr>\n");
         printf("</table>\n");
         printf("<!-- generated %s by %s EXE, compiled %s, on %s -->\n",
            ctime(&t), exe, __DATE__, __TIME__ );
         printf("</body>\n");
         printf("</html>\n");
      } else {
         sprintf(cp, "MD5 (\"%s\") = ", file);
         //puts(hex_output);
         strcat(cp, hex_output);
         sprintf( EndBuf(cp), " dated %s, of %s bytes.\n",
            get_time_string(buf.st_mtime),
            get_number_string(buf.st_size) );
         printf(cp);
      }
   } else {
      printf("ERROR: Read error on file [%s], or it is NULL ...\n",
         file );
      rval = 2;
   }
   return rval;
}

void put_string_wdt( const char * stg )
{
   printf( stg, __DATE__, __TIME__ );
}

void put_usage( void ) { put_string_wdt( usage ); }
void put_version( void ) { put_string_wdt( version ); }

/* Main program */
int main(int argc, char *argv[])
{
   int   iret = 2;   // set error condition
   int   i;
   char * in_file = NULL;
   size_t   len;
   if (argc > 1) {
      for(i = 1; i < argc; i++)
      {
         char * arg = argv[i];
         if ( !arg || !*arg )
            continue;
         len = strlen(arg);
         if( *arg == '-' )
         {
            while(*arg == '-') arg++;
            // got an OPTION
            if (!strcmp(arg, "test") || !strcmp(arg, "t"))
               return do_test();
            if (!strcmp(arg, "t-values") || !strcmp(arg, "l"))
               return do_t_values();
            if (!strcmp(arg, "version") || !strcmp(arg, "v")) {
               put_version();
               return 0;   // output version is a SUCCESS
            }
            if (!strncmp(arg, "d=", 2)) {
               arg += 2;
               goto Set_Digest;
            }
            if (!strncmp(arg, "digest=", 7)) {
               arg += 7;
Set_Digest:
               if(test_digest) {
                  printf("ERROR: Multiple digests! Already have [%s]\n", test_digest);
                  goto Unknown_ARG;
               }
               if(strlen(arg) != 32) {
                  printf("ERROR: Digest must be 32 bytes long! Have %d?\n", (int)strlen(arg));
                  goto Unknown_ARG;
               }
               test_digest = arg;   // set pointer to TEST digest string
               continue;
            }
            if(!strcmp(arg, "h") || !strcmp(arg,"help") || !strcmp(arg,"?")) {
               put_usage();
               return 0;   // showing HELP is a success!
            }
            if(!strcmp(arg, "html")) {
               html_output = 1;
               continue;
            }
Unknown_ARG:
            printf("ERROR: Unknown argument [%s]\n", argv[i]);
            return 2;
         } else {
            if(in_file) {
               printf("ERROR: Multiple files! Already have [%s]\n", in_file);
               goto Unknown_ARG;
            }
            in_file = arg; // set the INPUT file name pointer
         }
      }

      // done arguments - did we get a file?
      if(in_file) {
         iret = do_file( in_file, argv[0] ); // process the file
         test_digest = NULL;
         return iret;   // return ZERO for FULL success
      } else {
         printf("ERROR: No file name found in command!\n");
      }
   }
   put_usage();
   return iret;
}

// eof - md5main.c
