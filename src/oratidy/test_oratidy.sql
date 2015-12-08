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

