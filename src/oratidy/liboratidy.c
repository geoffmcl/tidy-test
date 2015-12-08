/* *********************************************************
   Oracle SQL Binding
   
   from: https://sourceforge.net/p/tidy/patches/86/
   Christian Created: 2010-09-15
   Dependence: 
      Oracle PL/SQL - ???
      HTML Tidy - https://github.com/htacg/tidy-html5
   ********************************************************* */

#include <stdio.h> 
#include <string.h> 
#include <errno.h> 
#include <tidy.h>
#include <tidybuffio.h>
#include <oci.h> 
#include <ociextp.h>

struct ocictx
{
  OCIEnv     *envhp;                           /* For OCI Environment Handle */
  OCISvcCtx  *svchp;                               /* For OCI Service Handle */
  OCIError   *errhp;                                /* For OCI Error Handle  */
  OCIStmt    *stmtp;                             /* For OCI Statement Handle */
  OCIStmt    *stm1p;                             /* For OCI Statement Handle */
  OCIBind    *bnd1p;                                  /* For OCI Bind Handle */
  OCIBind    *bnd2p;                                  /* For OCI Bind Handle */
  OCIBind    *bnd3p;                                  /* For OCI Bind Handle */
  OCIDefine  *dfn1p;                                /* For OCI Define Handle */
  OCIDefine  *dfn2p;                                /* For OCI Define Handle */
  OCIDefine  *dfn3p;                                /* For OCI Define Handle */
};
typedef struct ocictx ocictx;

void parseTidy( 
   OCIExtProcContext *ctx 
  ,OCILobLocator *clobinput 
  ,int *rc 
  ,OCILobLocator **cloboutxml 
  ,OCILobLocator **clobouterr 
  ,char *options 
) 
{ 
  // tidy vairables
  TidyDoc tdoc = tidyCreate();                     // Initialize "document" 
  TidyBuffer output = {0}; 
  TidyBuffer errbuf = {0}; 

  // oracle oci variables
  int err;
  ub4 lenp=0;
  ocictx   oci_ctx;
  ocictx  *oci_ctxp = &oci_ctx;


  /* set tidy options */ 
  int i=1;
  char *pch1;
  char sep1[] = ",=";
  pch1 = strtok (options,sep1);
  while (pch1 != NULL)
  {
    char *option;
    char *value;   
   
    if ( i % 2 != 0) 
    {
      option = pch1;
    }
    else
    { 
      value = pch1;
      if (!tidyOptSetValue( tdoc, tidyOptGetIdForName(option), value))
        *rc = -9800 - (i/2);
      else
        *rc = 0;
    }

    i++;
    pch1 = strtok (NULL, sep1);
  }

  /* get oci context from passed context */
  if ( *rc >= 0)
    if (OCIExtProcGetEnv(ctx,                       /* With context */
                         &oci_ctxp->envhp,
                         &oci_ctxp->svchp,
                         &oci_ctxp->errhp) != OCI_SUCCESS)
      *rc = -9901;
    else 
      *rc = 0; 

  /* if context is ok, read lob length */
  if ( *rc == 0)  
    if (OCILobGetLength(oci_ctxp->svchp, oci_ctxp->errhp, clobinput, &lenp) != OCI_SUCCESS)
      *rc = -9902;
    else 
      *rc = 0;

  /* if ok and length >0 the read lob */
  if (*rc == 0 && lenp>0)
  { 
    /* Read lob to local_buf */ 
    int amount = lenp;
    int offset = 1;
    int local_buflen = lenp +1; 
    ub1 local_buf[local_buflen];
    if (OCILobRead(oci_ctxp->svchp,
                   oci_ctxp->errhp,
                   clobinput,
                   &amount,
                   offset,
                   (dvoid *) local_buf,
                   local_buflen,
                   (dvoid *) 0,
                   (sb4 (*)(dvoid *, CONST dvoid *, ub4, ub1 )) 0,
                   (ub2) 0,
                   (ub1) SQLCS_IMPLICIT)!= OCI_SUCCESS)
    {
      *rc = -9903;
    } else {
      *rc = 0;
    }

    /* if ok proceed with tidy tasks */  
    if ( *rc >= 0 ) 
      *rc = tidySetErrorBuffer( tdoc, &errbuf );          // Capture diagnostics 
    if ( *rc >= 0 ) 
      *rc = tidyParseString( tdoc, local_buf );           // Parse the input 
    if ( *rc >= 0 ) 
      *rc = tidyCleanAndRepair( tdoc );                   // Tidy it up! 
    if ( *rc >= 0 ) 
      *rc = tidyRunDiagnostics( tdoc );                   // Kvetch 
    if ( *rc > 1 )                                        // If error, force output. 
      *rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 ); 
    if ( *rc >= 0 ) 
      *rc = tidySaveBuffer( tdoc, &output );              // Pretty Print 

    //return output.bp = cloboutxml mit OCILobWrite 
    if ( OCILobWrite(oci_ctxp->svchp,
                      oci_ctxp->errhp,
                      *cloboutxml,
                      &output.size,
                      1,
                      (dvoid *) output.bp,
                      (ub4) output.size,
                      OCI_ONE_PIECE,
                      (dvoid *)0,
                      (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 * )) 0,
                      (ub2)0, (ub1)SQLCS_IMPLICIT)!= OCI_SUCCESS)
    {
      *rc = -9904;
    }

    //return errbuf.bp = outerr mit OCILobWrite 
    if ( OCILobWrite(oci_ctxp->svchp,
                     oci_ctxp->errhp,
                     *clobouterr,
                     &errbuf.size,
                     1,
                     (dvoid *) errbuf.bp,
                     (ub4) errbuf.size,
                     OCI_ONE_PIECE,
                     (dvoid *)0,
                     (sb4 (*)(dvoid *, dvoid *, ub4 *, ub1 * )) 0,
                     (ub2)0, (ub1)SQLCS_IMPLICIT)!= OCI_SUCCESS)
    {
      *rc = -9905;
    }
  }

  /* clean up*/  
  tidyBufFree( &output ); 
  tidyBufFree( &errbuf ); 
  tidyRelease( tdoc ); 
}

/* USAGE

set serveroutput on

declare 
  o_xml clob  := ' ';
  o_err clob  := ' ';
  rc pls_integer := -1;

  procedure test( 
     i_xml IN CLOB   
    ,o_rc  OUT PLS_INTEGER 
    ,o_xml IN OUT CLOB 
    ,o_err IN OUT CLOB 
    ,i_options varchar2
  )   
  AS LANGUAGE C 
  NAME "parseTidy" 
  LIBRARY test3_lib 
  WITH CONTEXT 
  PARAMETERS ( 
     CONTEXT 
    ,i_xml
    ,o_rc BY REFERENCE 
    ,o_xml BY REFERENCE 
    ,o_err BY REFERENCE 
    ,i_options
  ); 
begin 
  -- good one
  test('<title>Foo</title><p>Foo!',rc,o_xml,o_err,'output-xhtml=yes');
  dbms_output.put_line(rc);
  dbms_output.put_line(o_xml);
  dbms_output.put_line(o_err);
  commit;

  -- bad one
  test('<title>Foo</title><p>Foo!',rc,o_xml,o_err,'output-xhtml=yes,foo=bar');
  dbms_output.put_line(rc);
  commit;
end; 
/

*/
