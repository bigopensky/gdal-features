/* Last modified Time-stamp: 2018-08-02
   Copyright (c) 2018, Alexander Weidauer All rights reserved.

   Test program to find out what is happening with exif metadata
   transfer for a geotif by CreateCopy
   
   This program is free software; you can redistribute it and/or modify
   it under the same terms as GDAL itself, either GDAL version 2.0.2
   or, at your option, any later version of GDAL you may have available.

   -------------------------------------------------------------

    gcc -std=c99 -lgdal store_md.c -o store-md
*/

// ---------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------
#include <gdal_version.h>
#include <gdal.h>
#include <cpl_string.h>

// ---------------------------------------------------------------
/**
 * Central Exit point for errors and error numbers
 */
void error_exit(int code, const char *message, ...) {
    va_list arglist;
    va_start(arglist,message);
    vfprintf(stderr, message, arglist);
    va_end(arglist);
    exit(code);
}

// ---------------------------------------------------------------
/**
 * Copy a string with mem allocation
 */
char *strdup(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p != NULL) {
        memcpy(p, s, size);
    }
    return p;
}

// ---------------------------------------------------------------
/**
 * Remove a string pattern (non regexp)
 */
void str_strip(char *s,const char *pat)
{

  while( s=strstr(s,pat) )
     memmove(s,s+strlen(pat),1+strlen(s+strlen(pat)));

}

// ---------------------------------------------------------------
/**
 * Check the command line yes no param
 */
int check_yesno(char *arg)
{
    if (arg == NULL) return(0);
    if ( strcmp(arg,"N") == 0) return(0);
    if ( strcmp(arg,"n") == 0) return(0);
    if ( strcmp(arg,"no") == 0) return(0);
    if ( strcmp(arg,"Y") == 0) return(1);
    if ( strcmp(arg,"y") == 0) return(1);
    if ( strcmp(arg,"yes") == 0) return(1);
    error_exit(30,"Use N,n,no,Y,y,yes for yes/no parameter\n");
}

// ---------------------------------------------------------------
/**
 * List all Metadata domains of a dataset
 */
void list_domains(char* title, GDALDatasetH ds)
{
  printf("list metadata domains of %s\n",title);
  char **lst_domain = GDALGetMetadataDomainList(ds);
  if (lst_domain != NULL)
  {
      int dlen = CSLCount(lst_domain);
      // printf(".OK %d found domains\n",dlen);

      for (int dix=0; dix < dlen ; dix++ )
      {
          const char* key =  CSLGetField(lst_domain, dix);
          char** lst_tags = NULL;
          if ( key == NULL )
          {
              printf("Domain %d has no key \n",dix);

          }
          else
          {
              printf("DOMAIN.%02d: %8s \n", dix+1, key );
              lst_tags = GDALGetMetadata(ds, key);
          }
          if (lst_tags != NULL)
          {
              int tlen = CSLCount(lst_tags);
              //printf(".OK %d found tags\n",dlen);
              for (int tix = 0; tix < tlen; tix++)
              {
                  const char *pair =  CSLGetField(lst_tags, tix);
                  printf("  TAG.%02d: %8s \n", tix+1, pair );
              }
              printf("\n");
          }
          else
          {
              printf("No tags!\n\n") ;
          }

      }
  }
  else { printf("No domains!\n\n");  }

}
// -------------------------------------------------------------------
// main
// -------------------------------------------------------------------
int main(int argc, char **argv)
{

  printf("GDAL VERSION %s %d\n\n", GDAL_RELEASE_NAME, GDAL_RELEASE_DATE);

  // Register all drivers
  GDALAllRegister();

  // Driver name
  const char *format = "GTiff";

  // Format section EXIF
  const char *dmn_exif = "EXIF";

  GDALDriverH h_drv = GDALGetDriverByName( format );
  if( h_drv == NULL ) {
    error_exit(10,"Driver %s not found!" ,format);
  }

  // Check gtiff write
  char **test_meta;
  test_meta = GDALGetMetadata( h_drv, NULL );
  if( ! CSLFetchBoolean( test_meta, GDAL_DCAP_CREATE, FALSE ) ) {
    error_exit(10,"Format %s is not witeable" ,format);
  }

  // 4 commandline args needed
  if (argc<5) {
      error_exit(20, "Missing Parameter\nUsage: %s infile outfile N|Y N|Y\n", argv[0]);
  }

  // Input file
  char *ifile   = argv[1];

  // Output file
  char *ofile   = argv[2];

   // N - do nothing   Y - remove EXIF_
  char *strip_prefix = argv[3];

   // N - do nothinng not  Y - use the EXIF section tos store
  char *use_tag = argv[4];

  int strip_it = check_yesno(strip_prefix);
  int tag_it   = check_yesno(use_tag);

  printf("# -------------------------------------------\n");
  printf("#  INVESTIGATE SOURCE \n");
  printf("# -------------------------------------------\n");
  printf("..open %s file %s\n", format, ifile);
  GDALDatasetH h_dsrc = GDALOpen( ifile, GA_ReadOnly);

  if( h_dsrc == NULL ) {
     error_exit(10, "Unable to open dataset %s!\n", ifile);
  }
  printf(".OK open\n\n");

  list_domains(ifile, h_dsrc);


  printf("# -------------------------------------------\n");
  printf("#  CREATE COPY \n");
  printf("# -------------------------------------------\n");
  printf("..copy %s file %s to %s\n", format, ifile, ofile);
  GDALDatasetH h_ddst = GDALCreateCopy( h_drv, ofile, h_dsrc, 0,
                                        NULL, NULL, NULL);
  if( h_ddst == NULL ) {error_exit(10, "Unable to copy dataset %s!\n", ofile); }
  printf(".OK open\n\n");

  printf(".. copy exif data from %s to %s \n", ifile, ofile);
  char** lst_exif = GDALGetMetadata(h_dsrc, dmn_exif);
  printf("..create copy %s from %s\n", ofile, ifile);
  if (tag_it)
  {
      printf("..use %s the metadata domain\n",dmn_exif);
  }
  else
  {
      printf("..use the default domain NULL\n");
  }

  if (strip_it)
  {
      printf("..remove the prefix EXIF_ from the tags\n");
  }

  if (lst_exif != NULL)
  {
      int tlen = CSLCount(lst_exif);

      for (int ix=0; ix < tlen; ix++ )
      {
          char *tkn  = strdup(CSLGetField(lst_exif, ix));
          char *key  = strtok(tkn, "=");
          char *value  = strtok(NULL, "");
          // DONT WORK
          // value = CPLParseNameValue( CSLGetField(lst_tags, ix), key );
          if ( (value != NULL) && (key!=NULL) )
          {
              if (strip_it) str_strip(key, "EXIF_");
              printf ("  TAG.%02d: %s=%s\n",ix, key, value);
              if (tag_it)
              {
                  GDALSetMetadataItem(h_ddst, key, value, dmn_exif);
              }
              else
              {
                  GDALSetMetadataItem(h_ddst, key, value, NULL);
              }
          }
          free(tkn);
      }
  }

  printf("# -------------------------------------------\n");
  printf("#  RESULTS \n");
  printf("# -------------------------------------------\n");

  list_domains(ifile, h_ddst);

  printf("# -------------------------------------------\n");
  printf("#  CLEAN UP \n");
  printf("# -------------------------------------------\n");

  GDALFlushCache(h_ddst);
  printf("..close source\n");
  GDALClose(h_dsrc);
  printf("..close destination\n");
  GDALClose(h_ddst);
  printf("# -------------------------------------------\n");
  printf("#  EOF \n");
  printf("# -------------------------------------------\n");


  //exit(0);
}
