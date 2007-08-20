/* 
   Copyright (c) 2007 Luca Bruno

   This file is part of Smalltalk YX.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell   
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER    
   DEALINGS IN THE SOFTWARE.
*/

#include "syx-memory.h"
#include "syx-error.h"
#include "syx-types.h"
#include "syx-plugins.h"
#include "syx-object.h"
#include "syx-enums.h"
#include "syx-scheduler.h"
#include "syx-parser.h"
#include "syx-lexer.h"
#include "syx-interp.h"
#include "syx-utils.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

inline SyxOop 
_syx_block_context_new_from_closure (SyxExecState *es, SyxOop arguments)
{
  return syx_block_context_new (es->context,
				SYX_BLOCK_CLOSURE_BLOCK (es->message_receiver),
				arguments,
				SYX_BLOCK_CLOSURE_DEFINED_CONTEXT(es->message_receiver));
}

/* This method is inlined in syx_interp_call_primitive */
SYX_FUNC_PRIMITIVE (Processor_yield)
{
  SYX_PRIM_YIELD (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Behavior_new)
{
  SYX_PRIM_RETURN (syx_object_new (es->message_receiver, TRUE));
}

SYX_FUNC_PRIMITIVE (Behavior_newColon)
{
  SYX_PRIM_ARGS(1);
  syx_varsize size = SYX_SMALL_INTEGER (es->message_arguments[0]);
  SYX_PRIM_RETURN(syx_object_new_size (es->message_receiver, TRUE, size));
}

SYX_FUNC_PRIMITIVE (ByteArray_newColon)
{
  SYX_PRIM_ARGS(1);
  syx_varsize size = SYX_SMALL_INTEGER (es->message_arguments[0]);
  SYX_PRIM_RETURN(syx_object_new_data (es->message_receiver, FALSE,
				       size,
				       syx_calloc (size, sizeof (syx_int8))));
}

SYX_FUNC_PRIMITIVE (Object_class)
{
  SYX_PRIM_RETURN(syx_object_get_class (es->message_receiver));
}

SYX_FUNC_PRIMITIVE (Object_at)
{
  SYX_PRIM_ARGS(1);
  syx_varsize index;
  index = SYX_SMALL_INTEGER(es->message_arguments[0]) - 1;
  if (index < 0 || index >= SYX_OBJECT_SIZE(es->message_receiver))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN(SYX_OBJECT_DATA(es->message_receiver)[index]);
}

SYX_FUNC_PRIMITIVE (Object_at_put)
{
  SYX_PRIM_ARGS(2);
  syx_varsize index;
  SyxOop object;

  index = SYX_SMALL_INTEGER(es->message_arguments[0]) - 1;
  if (index < 0 || index >= SYX_OBJECT_SIZE(es->message_receiver))
    {
      SYX_PRIM_FAIL;
    }
  object = es->message_arguments[1];
  SYX_OBJECT_DATA(es->message_receiver)[index] = object;

  SYX_PRIM_RETURN (object);
}

SYX_FUNC_PRIMITIVE (Object_resize)
{
  SYX_PRIM_ARGS(1);
  syx_varsize size;

  size = SYX_SMALL_INTEGER(es->message_arguments[0]);
  syx_object_resize (es->message_receiver, size);

  SYX_PRIM_RETURN (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Object_size)
{
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_OBJECT_SIZE (es->message_receiver)));
}

SYX_FUNC_PRIMITIVE (Object_identityEqual)
{
  SYX_PRIM_ARGS(1);
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OOP_EQ (es->message_receiver, es->message_arguments[0])));
}

SYX_FUNC_PRIMITIVE (Object_identityHash)
{
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_MEMORY_INDEX_OF (es->message_receiver)));
}

SYX_FUNC_PRIMITIVE (Object_hash)
{
  SYX_PRIM_RETURN (syx_small_integer_new (syx_object_hash (es->message_receiver)));
}

SYX_FUNC_PRIMITIVE (Object_equal)
{
  SYX_PRIM_ARGS(1);
  SYX_PRIM_RETURN (syx_boolean_new (syx_object_hash (es->message_receiver) ==
				    syx_object_hash (es->message_arguments[0])));
}

SYX_FUNC_PRIMITIVE (Object_copy)
{
  SYX_PRIM_RETURN (syx_object_copy (es->message_receiver));
}




SYX_FUNC_PRIMITIVE (ArrayedCollection_replaceFromToWith)
{
  SYX_PRIM_ARGS(3);
  syx_varsize start = SYX_SMALL_INTEGER (es->message_arguments[0]) - 1;
  SyxOop coll = es->message_arguments[2];
  syx_varsize end = SYX_SMALL_INTEGER(es->message_arguments[1]);
  syx_varsize i;

  // distinguish between arrays and bytearrays
  if (SYX_OBJECT_HAS_REFS (es->message_receiver))
    {
      if (SYX_OBJECT_HAS_REFS (coll))
	memcpy (SYX_OBJECT_DATA (es->message_receiver) + start, SYX_OBJECT_DATA (coll), end * sizeof (SyxOop));
      else
	{
	  for (i=start; i < end; i++)
	    SYX_OBJECT_DATA(es->message_receiver)[i] = syx_character_new (SYX_OBJECT_BYTE_ARRAY(coll)[i]);
	}
    }
  else
    {
      if (!SYX_OBJECT_HAS_REFS (coll))
	memcpy (SYX_OBJECT_BYTE_ARRAY (es->message_receiver) + start,
		SYX_OBJECT_BYTE_ARRAY (coll), end * sizeof (syx_int8));
      else
	{
	  SYX_PRIM_FAIL;
	}
    }

  SYX_PRIM_RETURN (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (ByteArray_at)
{
  SYX_PRIM_ARGS(1);
  syx_varsize index;

  index = SYX_SMALL_INTEGER(es->message_arguments[0]) - 1;
  if (index < 0 || index >= SYX_OBJECT_SIZE(es->message_receiver))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN(syx_character_new (SYX_OBJECT_BYTE_ARRAY(es->message_receiver)[index]));
}

SYX_FUNC_PRIMITIVE (ByteArray_at_put)
{
  SYX_PRIM_ARGS(2);
  syx_varsize index;
  SyxOop oop = es->message_arguments[1];

  index = SYX_SMALL_INTEGER(es->message_arguments[0]) - 1;
  if (index < 0 || index >= SYX_OBJECT_SIZE(es->message_receiver))
    {
      SYX_PRIM_FAIL;
    }
  SYX_OBJECT_BYTE_ARRAY(es->message_receiver)[index] = SYX_CHARACTER (oop);
  SYX_PRIM_RETURN (oop);
}



SYX_FUNC_PRIMITIVE (BlockClosure_asContext)
{
  SYX_PRIM_ARGS(1);
  SyxOop args;
  SyxOop ctx;
  syx_memory_gc_begin ();
  args = syx_array_new_ref (SYX_OBJECT_SIZE(es->message_arguments[0]), SYX_OBJECT_DATA(es->message_arguments[0]));
  ctx = _syx_block_context_new_from_closure (es, args);
  syx_memory_gc_end ();
  SYX_PRIM_RETURN (ctx);
}

SYX_FUNC_PRIMITIVE (BlockClosure_value)
{
  SyxOop ctx = _syx_block_context_new_from_closure (es, syx_nil);
  return syx_interp_enter_context (ctx);
}

SYX_FUNC_PRIMITIVE (BlockClosure_valueWith)
{
  SYX_PRIM_ARGS(1);
  SyxOop args;
  SyxOop ctx;

  syx_memory_gc_begin ();
  args = syx_array_new_size (1);
  SYX_OBJECT_DATA(args)[0] = es->message_arguments[0];
  ctx = _syx_block_context_new_from_closure (es, args);
  syx_memory_gc_end ();
  return syx_interp_enter_context (ctx);
}
  
SYX_FUNC_PRIMITIVE (BlockClosure_valueWithArguments)
{
  SYX_PRIM_ARGS(1);
  SyxOop args, ctx;
  syx_memory_gc_begin ();
  args = syx_array_new_ref (SYX_OBJECT_SIZE(es->message_arguments[0]), SYX_OBJECT_DATA(es->message_arguments[0]));
  ctx = _syx_block_context_new_from_closure (es, args);
  syx_memory_gc_end ();
  return syx_interp_enter_context (ctx);
}

SYX_FUNC_PRIMITIVE (BlockClosure_on_do)
{
  SYX_PRIM_ARGS(2);
  SyxOop ctx = _syx_block_context_new_from_closure (es, syx_nil);

  SYX_BLOCK_CONTEXT_HANDLED_EXCEPTION (ctx) = es->message_arguments[0];
  SYX_BLOCK_CONTEXT_HANDLER_BLOCK (ctx) = es->message_arguments[1];

  return syx_interp_enter_context (ctx);
}

SYX_FUNC_PRIMITIVE (BlockClosure_newProcess)
{
  SyxOop ctx;
  SyxOop proc;
  
  syx_memory_gc_begin ();
  ctx = _syx_block_context_new_from_closure (es, syx_nil);
  SYX_METHOD_CONTEXT_PARENT (ctx) = syx_nil;
  SYX_METHOD_CONTEXT_RETURN_CONTEXT (ctx) = syx_nil;
  proc = syx_process_new (ctx);
  syx_memory_gc_end ();

  SYX_PRIM_RETURN (proc);
}

SYX_FUNC_PRIMITIVE (Symbol_asString)
{
  SYX_PRIM_RETURN (syx_string_new (SYX_OBJECT_SYMBOL (es->message_receiver)));
}

/* These printing function are used ONLY for tests */
SYX_FUNC_PRIMITIVE (SmallInteger_print)
{
  printf ("%d\n", SYX_SMALL_INTEGER(es->message_receiver));
  SYX_PRIM_RETURN (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (LargeInteger_print)
{
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (es->message_receiver))
    printf ("%llu\n", SYX_OBJECT_LARGE_INTEGER (es->message_receiver));
  else
    printf ("-%llu\n", SYX_OBJECT_LARGE_INTEGER (es->message_receiver));
  SYX_PRIM_RETURN (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Float_print)
{
  printf ("%f\n", SYX_OBJECT_FLOAT(es->message_receiver));
  SYX_PRIM_RETURN (es->message_receiver);
}



/* Processor */

SYX_FUNC_PRIMITIVE (Processor_enter)
{
  SYX_PRIM_ARGS(1);
  return syx_interp_enter_context (es->message_arguments[0]);
}

SYX_FUNC_PRIMITIVE (Processor_swapWith)
{
  SYX_PRIM_ARGS(1);
  return syx_interp_swap_context (es->message_arguments[0]);
}

SYX_FUNC_PRIMITIVE (Processor_leaveTo_andAnswer)
{
  SYX_PRIM_ARGS(2);
  SYX_METHOD_CONTEXT_RETURN_CONTEXT(es->context) = es->message_arguments[0];
  return syx_interp_leave_context_and_answer (es->message_arguments[1], TRUE);
}

SYX_FUNC_PRIMITIVE (Character_new)
{
  SYX_PRIM_ARGS(1);
  SYX_PRIM_RETURN (syx_character_new (SYX_SMALL_INTEGER (es->message_arguments[0])));
}

SYX_FUNC_PRIMITIVE (Character_value)
{
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_CHARACTER (es->message_receiver)));
}


SYX_FUNC_PRIMITIVE (Semaphore_signal)
{
  syx_semaphore_signal (es->message_receiver);
  SYX_PRIM_YIELD (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Semaphore_wait)
{
  syx_semaphore_wait (es->message_receiver);
  SYX_PRIM_YIELD (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Semaphore_waitFor)
{
  SYX_PRIM_ARGS(2);
  syx_int32 fd = SYX_SMALL_INTEGER(es->message_arguments[0]);
  syx_bool t = es->message_arguments[1];
  syx_semaphore_wait (es->message_receiver);
  if (t == syx_true)
    syx_scheduler_poll_write_register (fd,
				       es->message_receiver);
  else
    syx_scheduler_poll_read_register (fd,
				      es->message_receiver);
  SYX_PRIM_YIELD (es->message_receiver);
}

/* File streams */

SYX_FUNC_PRIMITIVE (FileStream_fileOp)
{
  SYX_PRIM_ARGS(2);
  syx_int32 fd = SYX_SMALL_INTEGER (es->message_arguments[1]);
  syx_int32 ret = 0;

  switch (SYX_SMALL_INTEGER (es->message_arguments[0]))
    {
    case 0: // open
      {
	syx_symbol mode = SYX_OBJECT_SYMBOL (es->message_arguments[2]);
	syx_int32 flags = 0;

	if (*mode == 'r')
	  {
	    if (mode[1] == '+')
	      flags |= O_RDWR;
	    else
	      flags |= O_RDONLY;
	  }
	else if (*mode == 'w')
	  flags |= O_WRONLY;
	else
	  syx_error ("mh? %s\n", mode);
	
	ret = open (SYX_OBJECT_STRING (es->message_arguments[1]), flags);
      }
      break;

    case 1: // close
      ret = close (fd);
      break;
      
    case 2: // nextPut:
      SYX_PRIM_ARGS(3);
      {
	syx_char c = SYX_CHARACTER (es->message_arguments[2]);
	ret = write (fd, &c, 1);
      }
      break;

    case 3: // nextPutAll:
      SYX_PRIM_ARGS(3);

      if (!SYX_IS_NIL (es->message_arguments[2]))
	ret = write (fd, SYX_OBJECT_STRING (es->message_arguments[2]),
		     SYX_OBJECT_SIZE (es->message_arguments[2]) - 1);
      else
	ret = 0;
      break;

    case 4: // flush
//      ret = fsync (fd);
      break;

    case 5: // next
      {
	syx_char c;
	read (fd, &c, 1);
	SYX_PRIM_RETURN (syx_character_new (c));
      }
      break;

    case 6: // nextAll:
      SYX_PRIM_ARGS(3);

      {
	syx_int32 count = SYX_SMALL_INTEGER (es->message_arguments[2]);
	syx_char s[count];
	SyxOop string;

	count = read (fd, s, count);

	if (!count)
	  {
	    // maybe EOF
	    SYX_PRIM_RETURN (syx_nil);
	  }

	s[count] = '\0';

	string = syx_string_new (s);
	SYX_PRIM_RETURN (string);
      }
      break;

    default: // unknown
      syx_error ("Unknown file operation: %d\n", SYX_SMALL_INTEGER (es->message_arguments[0]));

    }

  SYX_PRIM_RETURN (syx_small_integer_new (ret));
}

SYX_FUNC_PRIMITIVE (String_compile)
{
  SyxLexer *lexer;
  SyxParser *parser;
  SyxOop meth;

  lexer = syx_lexer_new (SYX_OBJECT_STRING (es->message_receiver));
  if (!lexer)
    {
      SYX_PRIM_RETURN (syx_nil);
    }

  meth = syx_method_new ();
  parser = syx_parser_new (lexer, meth, syx_undefined_object_class);
  if (!syx_parser_parse (parser))
    {
      syx_parser_free (parser, TRUE);
      SYX_PRIM_FAIL;
    }

  syx_lexer_free (lexer, FALSE);
  syx_parser_free (parser, FALSE);

  SYX_PRIM_RETURN (meth);
}

SYX_FUNC_PRIMITIVE (String_hash)
{
  SYX_PRIM_RETURN (syx_small_integer_new (syx_string_hash (SYX_OBJECT_SYMBOL (es->message_receiver))));
}

/* Small integers */

SYX_FUNC_PRIMITIVE (SmallInteger_plus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_int32 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }

  a = SYX_SMALL_INTEGER (first);
  b = SYX_SMALL_INTEGER (second);
  if (SYX_SMALL_INTEGER_OVERFLOW (a, b))
    {
      SYX_PRIM_FAIL;
    }

  result = SYX_SMALL_INTEGER (first) + SYX_SMALL_INTEGER (second);
  if (!SYX_SMALL_INTEGER_CAN_EMBED (result))
    {
      SYX_PRIM_FAIL;
    }

  SYX_PRIM_RETURN (syx_small_integer_new (result));
}

SYX_FUNC_PRIMITIVE (SmallInteger_minus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_int32 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }

  a = SYX_SMALL_INTEGER (first);
  b = SYX_SMALL_INTEGER (second);
  if (SYX_SMALL_INTEGER_OVERFLOW (a, -b))
    {
      SYX_PRIM_FAIL;
    }

  result = SYX_SMALL_INTEGER (first) - SYX_SMALL_INTEGER (second);
  if (!SYX_SMALL_INTEGER_CAN_EMBED (result))
    {
      SYX_PRIM_FAIL;
    }

  SYX_PRIM_RETURN (syx_small_integer_new (result));
}

SYX_FUNC_PRIMITIVE (SmallInteger_lt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_SMALL_INTEGER (first) <
				    SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_gt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_SMALL_INTEGER (first) >
				    SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_le)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_SMALL_INTEGER (first) <=
				    SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_ge)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_SMALL_INTEGER (first) >=
				    SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_eq)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (first == second));
}

SYX_FUNC_PRIMITIVE (SmallInteger_ne)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (first != second));
}

SYX_FUNC_PRIMITIVE (SmallInteger_div)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_SMALL_INTEGER (first) /
					  SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_mod)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_SMALL_INTEGER (first) %
					  SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_bitAnd)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_SMALL_INTEGER (first) &
					  SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_bitOr)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_SMALL_INTEGER (first) |
					  SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_bitXor)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_IS_SMALL_INTEGER (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_small_integer_new (SYX_SMALL_INTEGER (first) ^
					  SYX_SMALL_INTEGER (second)));
}

SYX_FUNC_PRIMITIVE (SmallInteger_asFloat)
{
  syx_double n = (syx_double)SYX_SMALL_INTEGER (es->message_receiver);
  SYX_PRIM_RETURN (syx_float_new (n));
}

SYX_FUNC_PRIMITIVE (SmallInteger_asLargeInteger)
{
  syx_int32 num = SYX_SMALL_INTEGER (es->message_receiver);
  if (num >= 0)
    {
      SYX_PRIM_RETURN (syx_large_positive_integer_new ((syx_uint64) num));
    }
  else
    {
      SYX_PRIM_RETURN (syx_large_negative_integer_new ((syx_uint64) -num));
    }
}

/* Large positive integers */

SYX_FUNC_PRIMITIVE (LargePositiveInteger_plus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_uint64 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_large_positive_integer_new (SYX_OBJECT_LARGE_INTEGER (first) + SYX_OBJECT_LARGE_INTEGER (second)));
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      a = SYX_OBJECT_LARGE_INTEGER (first);
      b = SYX_OBJECT_LARGE_INTEGER (second);
      if (a > b)
	{
	  result = a - b;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_positive_integer_new (result));
	    }
	}
      else
	{
	  result = b - a;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (-result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) -result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_negative_integer_new (result));
	    }
	}
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_minus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_uint64 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_large_positive_integer_new (SYX_OBJECT_LARGE_INTEGER (first) + SYX_OBJECT_LARGE_INTEGER (second)));
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      a = SYX_OBJECT_LARGE_INTEGER (first);
      b = SYX_OBJECT_LARGE_INTEGER (second);
      if (a > b)
	{
	  result = a - b;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_positive_integer_new (result));
	    }
	}
      else
	{
	  result = b - a;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (-result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) -result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_negative_integer_new (result));
	    }
	}
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_lt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) <
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_gt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) >
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_le)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) <=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_ge)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) >=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_eq)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) ==
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargePositiveInteger_ne)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) !=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

/* Large negative integers */

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_plus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_uint64 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_large_negative_integer_new (SYX_OBJECT_LARGE_INTEGER (first) + SYX_OBJECT_LARGE_INTEGER (second)));
    }
  else if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      a = SYX_OBJECT_LARGE_INTEGER (first);
      b = SYX_OBJECT_LARGE_INTEGER (second);
      if (a > b)
	{
	  result = a - b;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (-result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) -result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_negative_integer_new (result));
	    }
	}
      else
	{
	  result = b - a;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_positive_integer_new (result));
	    }
	}
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_minus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  syx_uint64 a, b, result;

  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_large_positive_integer_new (SYX_OBJECT_LARGE_INTEGER (first) + SYX_OBJECT_LARGE_INTEGER (second)));
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      a = SYX_OBJECT_LARGE_INTEGER (first);
      b = SYX_OBJECT_LARGE_INTEGER (second);
      if (a > b)
	{
	  result = a - b;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (-result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) -result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_negative_integer_new (result));
	    }
	}
      else
	{
	  result = b - a;
	  if (result < ((syx_uint64)1 << 31) && SYX_SMALL_INTEGER_CAN_EMBED (result))
	    {
	      SYX_PRIM_RETURN (syx_small_integer_new ((syx_int32) result));
	    }
	  else
	    {
	      SYX_PRIM_RETURN (syx_large_positive_integer_new (result));
	    }
	}
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_lt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) >
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_gt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) <
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_le)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) >=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_ge)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) <=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_eq)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_false);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) ==
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

SYX_FUNC_PRIMITIVE (LargeNegativeInteger_ne)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (SYX_OBJECT_IS_LARGE_POSITIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_true);
    }
  else if (SYX_OBJECT_IS_LARGE_NEGATIVE_INTEGER (second))
    {
      SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_LARGE_INTEGER (first) !=
					SYX_OBJECT_LARGE_INTEGER (second)));
    }

  SYX_PRIM_FAIL;
}

/* Floats */

SYX_FUNC_PRIMITIVE (Float_plus)
{
  SYX_PRIM_ARGS(1);
  
  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_float_new (SYX_OBJECT_FLOAT (first) +
				  SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_minus)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_float_new (SYX_OBJECT_FLOAT (first) -
				  SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_lt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) <
				    SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_gt)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) >
				    SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_le)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) <=
				    SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_ge)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) >=
				    SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_eq)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) ==
				    SYX_OBJECT_FLOAT (second)));
}

SYX_FUNC_PRIMITIVE (Float_ne)
{
  SYX_PRIM_ARGS(1);

  SyxOop first, second;
  first = es->message_receiver;
  second = es->message_arguments[0];
  if (!SYX_OBJECT_IS_FLOAT (second))
    {
      SYX_PRIM_FAIL;
    }
  SYX_PRIM_RETURN (syx_boolean_new (SYX_OBJECT_FLOAT (first) !=
				    SYX_OBJECT_FLOAT (second)));
}


/* Object memory and Smalltalk */


SYX_FUNC_PRIMITIVE (ObjectMemory_snapshot)
{
  SYX_PRIM_ARGS(1);

  SyxOop filename = es->message_arguments[0];

  // save the current execution state
  syx_interp_stack_push (es->message_receiver);
  syx_exec_state_save ();

  if (SYX_IS_NIL (filename))
    syx_memory_save_image (NULL);
  else
    syx_memory_save_image (SYX_OBJECT_STRING (filename));
  
  return FALSE;
}

SYX_FUNC_PRIMITIVE (ObjectMemory_garbageCollect)
{
  syx_memory_gc ();
  SYX_PRIM_RETURN (es->message_receiver);
}

SYX_FUNC_PRIMITIVE (Smalltalk_quit)
{
  SYX_PRIM_ARGS(1);
  syx_int32 status = SYX_SMALL_INTEGER (es->message_arguments[0]);
  syx_quit ();
  exit (status);
}

SYX_FUNC_PRIMITIVE (Smalltalk_loadPlugin)
{
  SYX_PRIM_ARGS(1);
  syx_symbol name = SYX_OBJECT_SYMBOL(es->message_arguments[0]);
  SYX_PRIM_RETURN(syx_boolean_new (syx_plugin_load (name)));
}

SYX_FUNC_PRIMITIVE (Smalltalk_unloadPlugin)
{
  SYX_PRIM_ARGS(1);
  syx_symbol name = SYX_OBJECT_SYMBOL(es->message_arguments[0]);
  SYX_PRIM_RETURN(syx_boolean_new (syx_plugin_unload (name)));
}


SYX_FUNC_PRIMITIVE (Dictionary_pos)
{
  SYX_PRIM_ARGS(1);
  SyxOop table = SYX_DICTIONARY_HASH_TABLE (es->message_receiver);
  syx_varsize size = SYX_OBJECT_SIZE (table);
  syx_int32 hash = SYX_SMALL_INTEGER (es->message_arguments[0]);
  syx_int32 pos = 2 * (hash % ((size - 1) / 2));

  SYX_PRIM_RETURN (syx_small_integer_new (pos));
}

static SyxPrimitiveEntry primitive_entries[] = {
  { "Processor_yield", Processor_yield },

  /* Common for objects */
  { "Object_class", Object_class },
  { "Behavior_new", Behavior_new },
  { "Behavior_newColon", Behavior_newColon },
  { "Object_at", Object_at },
  { "Object_at_put", Object_at_put },
  { "Object_size", Object_size },
  { "Object_identityEqual", Object_identityEqual },
  { "Object_identityHash", Object_identityHash },
  { "Object_hash", Object_hash },
  { "Object_equal", Object_equal },
  { "Object_resize", Object_resize },
  { "Object_copy", Object_copy },

  /* Arrayed collections */
  { "ArrayedCollection_replaceFromToWith", ArrayedCollection_replaceFromToWith },

  /* Byte arrays */
  { "ByteArray_newColon", ByteArray_newColon },
  { "ByteArray_at", ByteArray_at },
  { "ByteArray_at_put", ByteArray_at_put },

  { "BlockClosure_asContext", BlockClosure_asContext },
  { "BlockClosure_value", BlockClosure_value },
  { "BlockClosure_valueWith", BlockClosure_valueWith },
  { "BlockClosure_valueWithArguments", BlockClosure_valueWithArguments },
  { "BlockClosure_on_do", BlockClosure_on_do },
  { "BlockClosure_newProcess", BlockClosure_newProcess },

  { "Symbol_asString", Symbol_asString },
  { "SmallInteger_print", SmallInteger_print },
  { "LargeInteger_print", LargeInteger_print },
  { "Float_print", Float_print },

  /* Interpreter */
  { "Processor_enter", Processor_enter },
  { "Processor_swapWith", Processor_swapWith },
  { "Processor_leaveTo_andAnswer", Processor_leaveTo_andAnswer },

  { "Character_new", Character_new },
  { "Character_value", Character_value },
  { "Semaphore_signal", Semaphore_signal },
  { "Semaphore_wait", Semaphore_wait },
  { "Semaphore_waitFor", Semaphore_waitFor },
  { "String_compile", String_compile },

  /* Strings */
  { "String_hash", String_hash },

  /* Dictionaries */
  { "Dictionary_pos", Dictionary_pos },

  /* File streams */
  { "FileStream_fileOp", FileStream_fileOp },

  /* Small integers */
  { "SmallInteger_plus", SmallInteger_plus },
  { "SmallInteger_minus", SmallInteger_minus },
  { "SmallInteger_lt", SmallInteger_lt },
  { "SmallInteger_gt", SmallInteger_gt },
  { "SmallInteger_le", SmallInteger_le },
  { "SmallInteger_ge", SmallInteger_ge },
  { "SmallInteger_eq", SmallInteger_eq },
  { "SmallInteger_ne", SmallInteger_ne },
  { "SmallInteger_div", SmallInteger_div },
  { "SmallInteger_mod", SmallInteger_mod },
  { "SmallInteger_bitAnd", SmallInteger_bitAnd },
  { "SmallInteger_bitOr", SmallInteger_bitOr },
  { "SmallInteger_bitXor", SmallInteger_bitXor },
  { "SmallInteger_asFloat", SmallInteger_asFloat },
  { "SmallInteger_asLargeInteger", SmallInteger_asLargeInteger },

  /* Large positive integers */
  { "LargePositiveInteger_plus", LargePositiveInteger_plus },
  { "LargePositiveInteger_minus", LargePositiveInteger_minus },
  { "LargePositiveInteger_lt", LargePositiveInteger_lt },
  { "LargePositiveInteger_gt", LargePositiveInteger_gt },
  { "LargePositiveInteger_le", LargePositiveInteger_le },
  { "LargePositiveInteger_ge", LargePositiveInteger_ge },
  { "LargePositiveInteger_eq", LargePositiveInteger_eq },
  { "LargePositiveInteger_ne", LargePositiveInteger_ne },

  /* Large negative integers */
  { "LargeNegativeInteger_plus", LargeNegativeInteger_plus },
  { "LargeNegativeInteger_minus", LargeNegativeInteger_minus },
  { "LargeNegativeInteger_lt", LargeNegativeInteger_lt },
  { "LargeNegativeInteger_gt", LargeNegativeInteger_gt },
  { "LargeNegativeInteger_le", LargeNegativeInteger_le },
  { "LargeNegativeInteger_ge", LargeNegativeInteger_ge },
  { "LargeNegativeInteger_eq", LargeNegativeInteger_eq },
  { "LargeNegativeInteger_ne", LargeNegativeInteger_ne },

  /* Floats */
  { "Float_plus", Float_plus },
  { "Float_minus", Float_minus },
  { "Float_lt", Float_lt },
  { "Float_gt", Float_gt },
  { "Float_le", Float_le },
  { "Float_ge", Float_ge },
  { "Float_eq", Float_eq },
  { "Float_ne", Float_ne },

  /* Object memory */
  { "ObjectMemory_snapshot", ObjectMemory_snapshot },
  { "ObjectMemory_garbageCollect", ObjectMemory_garbageCollect },

  /* Smalltalk environment */
  { "Smalltalk_quit", Smalltalk_quit },
  { "Smalltalk_loadPlugin", Smalltalk_loadPlugin },
  { "Smalltalk_unloadPlugin", Smalltalk_unloadPlugin },

  { NULL }
};

inline SyxPrimitiveEntry *
syx_primitive_get_entry (syx_int32 index)
{
  if (index < SYX_PRIMITIVES_MAX)
    return &primitive_entries[index];

  return NULL;
}

syx_int32
syx_primitive_get_index (syx_symbol name)
{
  syx_int32 i;

  for (i=0; i < SYX_PRIMITIVES_MAX; i++)
    {
      if (!strcmp (primitive_entries[i].name, name))
	return i;
    }

  return -1;
}