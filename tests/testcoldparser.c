#include <assert.h>
#include "../syx/syx.h"

int
main (int argc, char *argv[])
{
  SyxLexer *lexer;
  SyxObject *temp;
  GTimer *timer;

  syx_init ("..");
  syx_build_basic ();
  timer = g_timer_new ();

  g_timer_start (timer);

  /*  lexer = syx_lexer_new ("undefined subclass: #Object!");
  assert (syx_cold_parse (lexer, &error) == FALSE);

  lexer = syx_lexer_new ("nil message: #Object!");
  assert (syx_cold_parse (lexer, &error) == FALSE);

  lexer = syx_lexer_new ("nil subclass: Object!");
  assert (syx_cold_parse (lexer, &error) == FALSE);

  lexer = syx_lexer_new ("nil subclass: #Object");
  assert (syx_cold_parse (lexer, &error) == FALSE);*/
  
  temp = syx_globals_at ("Object");
  lexer = syx_lexer_new ("nil subclass: #Object instanceVariableNames: ''!");
  assert (syx_cold_parse (lexer) == TRUE);
  assert (syx_globals_at ("Object") == temp);
  syx_lexer_free (lexer, FALSE);

  lexer = syx_lexer_new ("!Object methodsFor: 'test'! testMethod ^nil! !");
  assert (syx_cold_parse (lexer) == TRUE);
  syx_lexer_free (lexer, FALSE);

  g_timer_stop (timer);
  g_print("Time elapsed: %f\n", g_timer_elapsed (timer, NULL));
  g_timer_destroy (timer);

  return 0;
}