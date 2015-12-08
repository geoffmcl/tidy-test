set serveroutput on

create or replace package tidy is
  procedure parse(
     i_xml IN CLOB   
    ,o_rc  OUT PLS_INTEGER 
    ,o_xml IN OUT CLOB 
    ,o_err IN OUT CLOB 
    ,i_options varchar2 := ''
  );

  procedure test;
end;
/

create or replace package body tidy is
  g_o_xml  clob  := ' ';
  g_o_err  clob  := ' ';
  g_o_rc     pls_integer := -1;

  procedure parse_c( 
     i_xml IN CLOB   
    ,o_rc  OUT PLS_INTEGER 
    ,o_xml IN OUT CLOB 
    ,o_err IN OUT CLOB 
    ,i_options varchar2
  )   
  AS LANGUAGE C 
  NAME "parseTidy" 
  LIBRARY TIDY_LIB
  WITH CONTEXT 
  PARAMETERS ( 
     CONTEXT 
    ,i_xml
    ,o_rc BY REFERENCE 
    ,o_xml BY REFERENCE 
    ,o_err BY REFERENCE 
    ,i_options
  ); 

  procedure parse(
     i_xml IN CLOB   
    ,o_rc  OUT PLS_INTEGER 
    ,o_xml IN OUT CLOB 
    ,o_err IN OUT CLOB 
    ,i_options varchar2 := ''
  ) 
  is
  begin
    -- parse
    parse_c(i_xml,g_o_rc,g_o_xml,g_o_err,i_options);
    o_rc  := g_o_rc;
    o_xml := g_o_xml;
    o_err := g_o_err;    

    -- reset globals
    g_o_xml  := ' ';
    g_o_err  := ' ';
    g_o_rc   := -1;
  end parse;

  procedure test
  is
    o_rc  pls_integer;
    o_xml clob;
    o_err clob;
  begin
    parse('<title>Foo</title><p>Foo!',o_rc,o_xml,o_err,'output-xhtml=yes');
    dbms_output.put_line('Return Code=' || o_rc);
    dbms_output.put_line('xml=' || o_xml);
    dbms_output.put_line('warnings/errors=' || o_err);
  end test;
end; 
/

