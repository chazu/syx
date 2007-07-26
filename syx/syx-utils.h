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

#ifndef SYX_UTILS_H
#define SYX_UTILS_H

#include "syx-platform.h"
#include "syx-types.h"
#include "syx-lexer.h"

syx_bool syx_cold_parse (SyxLexer *lexer);
syx_bool syx_cold_file_in (syx_symbol filename);

void syx_semaphore_signal (SyxOop semaphore);
void syx_semaphore_wait (SyxOop semaphore);

/* Utilities to interact with Smalltalk */

SyxOop syx_send_unary_message (SyxOop parent_context, SyxOop receiver, syx_symbol selector);
SyxOop syx_send_binary_message (SyxOop parent_context, SyxOop receiver, syx_symbol selector, SyxOop argument);
SyxOop syx_send_message (SyxOop parent_context, SyxOop receiver, syx_symbol selector, syx_varsize num_args, ...);


/* Utilities for strings */

syx_uint32 syx_find_first_non_whitespace (syx_symbol string);

#endif /* SYX_UTILS_H */