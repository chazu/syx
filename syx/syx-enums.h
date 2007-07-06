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

#ifndef SYX_ENUMS_H
#define SYX_ENUMS_H

//! The type of SyxOop contained in SyxOop::c::type
typedef enum
  {
    //! This means SyxOop indexes an object into the syx memory
    SYX_TYPE_POINTER,
    SYX_TYPE_SMALL_INTEGER,
    SYX_TYPE_CHARACTER,
  } SyxType;

//! Indexes of known instance variables
/*!
  Each known class has default instance variables used by the VM that are mandatory.
  These indexes are used to facilitate the access these variables and are contained in SyxObject::data
*/
typedef enum
  {
    SYX_VARS_OBJECT_ALL,

    SYX_VARS_CLASS_NAME = SYX_VARS_OBJECT_ALL,
    SYX_VARS_CLASS_SUPERCLASS,
    SYX_VARS_CLASS_INSTANCE_VARIABLES,
    SYX_VARS_CLASS_INSTANCE_SIZE,
    SYX_VARS_CLASS_METHODS,
    SYX_VARS_CLASS_SUBCLASSES,
    SYX_VARS_CLASS_ALL,

    SYX_VARS_METACLASS_INSTANCE_CLASS = SYX_VARS_CLASS_ALL,
    SYX_VARS_METACLASS_ALL,

    SYX_VARS_CLASS_CLASS_VARIABLES = SYX_VARS_CLASS_ALL,
    SYX_VARS_CLASS_CLASS_ALL,

    SYX_VARS_SYMBOL_ALL = SYX_VARS_OBJECT_ALL,

    SYX_VARS_STRING_ALL = SYX_VARS_OBJECT_ALL,

    SYX_VARS_ASSOCIATION_KEY = SYX_VARS_OBJECT_ALL,
    SYX_VARS_ASSOCIATION_VALUE,
    SYX_VARS_ASSOCIATION_ALL,

    SYX_VARS_LINK_NEXT = SYX_VARS_ASSOCIATION_ALL,
    SYX_VARS_LINK_ALL,

    SYX_VARS_VARIABLE_BINDING_DICTIONARY = SYX_VARS_ASSOCIATION_ALL,
    SYX_VARS_VARIABLE_BINDING_ALL,

    SYX_VARS_METHOD_BINDING_CLASS = SYX_VARS_VARIABLE_BINDING_ALL,
    SYX_VARS_METHOD_BINDING_ALL,

    SYX_VARS_DICTIONARY_ALL = SYX_VARS_OBJECT_ALL,

    SYX_VARS_METHOD_SELECTOR = SYX_VARS_OBJECT_ALL,
    SYX_VARS_METHOD_BYTECODES,
    SYX_VARS_METHOD_LITERALS,
    SYX_VARS_METHOD_ARGUMENTS_COUNT,
    SYX_VARS_METHOD_TEMPORARIES_COUNT,
    SYX_VARS_METHOD_STACK_SIZE,
    SYX_VARS_METHOD_PRIMITIVE,
    SYX_VARS_METHOD_ALL,

    SYX_VARS_BLOCK_ARGUMENTS_TOP=SYX_VARS_METHOD_ALL,
    SYX_VARS_BLOCK_ALL,

    SYX_VARS_BLOCK_CLOSURE_BLOCK = SYX_VARS_OBJECT_ALL,
    SYX_VARS_BLOCK_CLOSURE_DEFINED_CONTEXT,
    SYX_VARS_BLOCK_CLOSURE_ALL,

    SYX_VARS_PROCESS_CONTEXT = SYX_VARS_OBJECT_ALL,
    SYX_VARS_PROCESS_SUSPENDED,
    SYX_VARS_PROCESS_RETURNED_OBJECT,
    SYX_VARS_PROCESS_NEXT,
    SYX_VARS_PROCESS_SCHEDULED,
    SYX_VARS_PROCESS_ALL,

    SYX_VARS_SEMAPHORE_LIST = SYX_VARS_OBJECT_ALL,
    SYX_VARS_SEMAPHORE_SIGNALS,
    SYX_VARS_SEMAPHORE_ALL,

    SYX_VARS_METHOD_CONTEXT_PARENT = SYX_VARS_OBJECT_ALL,
    SYX_VARS_METHOD_CONTEXT_METHOD,
    SYX_VARS_METHOD_CONTEXT_STACK,
    SYX_VARS_METHOD_CONTEXT_SP,
    SYX_VARS_METHOD_CONTEXT_IP,
    SYX_VARS_METHOD_CONTEXT_RECEIVER,
    SYX_VARS_METHOD_CONTEXT_ARGUMENTS,
    SYX_VARS_METHOD_CONTEXT_TEMPORARIES,
    SYX_VARS_METHOD_CONTEXT_RETURN_CONTEXT,
    SYX_VARS_METHOD_CONTEXT_ALL,

    SYX_VARS_BLOCK_CONTEXT_OUTER_CONTEXT = SYX_VARS_METHOD_CONTEXT_ALL,
    SYX_VARS_BLOCK_CONTEXT_HANDLED_EXCEPTION,
    SYX_VARS_BLOCK_CONTEXT_HANDLER_BLOCK,
    SYX_VARS_BLOCK_CONTEXT_ALL,

    SYX_VARS_PROCESSOR_SCHEDULER_BYTESLICE = SYX_VARS_OBJECT_ALL,
    SYX_VARS_PROCESSOR_SCHEDULER_FIRST_PROCESS,
    SYX_VARS_PROCESSOR_SCHEDULER_ACTIVE_PROCESS,
    SYX_VARS_PROCESSOR_SCHEDULER_ALL,

  } SyxVariables;

//! The type of the token in SyxToken::type
typedef enum
  {
    SYX_TOKEN_END,
    SYX_TOKEN_INT_CONST,
    SYX_TOKEN_LARGE_INT_CONST,
    SYX_TOKEN_CHAR_CONST,
    SYX_TOKEN_FLOAT_CONST,
    SYX_TOKEN_CLOSING,
    SYX_TOKEN_ARRAY_BEGIN,

    SYX_TOKEN_STRING_ENTRY,
    SYX_TOKEN_NAME_CONST,
    SYX_TOKEN_NAME_COLON,
    SYX_TOKEN_SYM_CONST,
    SYX_TOKEN_STR_CONST,
    SYX_TOKEN_BINARY,
  } SyxTokenType;

//! List of commands of a bytecode in SyxBytecode::code
typedef enum
  {
    SYX_BYTECODE_PUSH_INSTANCE,
    SYX_BYTECODE_PUSH_ARGUMENT,
    SYX_BYTECODE_PUSH_TEMPORARY,
    SYX_BYTECODE_PUSH_LITERAL,
    SYX_BYTECODE_PUSH_CONSTANT,
    SYX_BYTECODE_PUSH_BINDING_VARIABLE,
    SYX_BYTECODE_PUSH_ARRAY,
    SYX_BYTECODE_PUSH_BLOCK_CLOSURE,
    
    SYX_BYTECODE_ASSIGN_INSTANCE,
    SYX_BYTECODE_ASSIGN_TEMPORARY,
    SYX_BYTECODE_ASSIGN_BINDING_VARIABLE,
    
    SYX_BYTECODE_MARK_ARGUMENTS,
    SYX_BYTECODE_SEND_MESSAGE,
    SYX_BYTECODE_SEND_SUPER,
    SYX_BYTECODE_SEND_UNARY,
    SYX_BYTECODE_SEND_BINARY,

    SYX_BYTECODE_DO_SPECIAL,
  
    SYX_BYTECODE_EXTENDED = 0x1F
  } SyxBytecodeCommand;

//! Special commands performed by the command SYX_BYTECODE_DO_SPECIAL
typedef enum
  {
    SYX_BYTECODE_POP_TOP,
    SYX_BYTECODE_SELF_RETURN,
    SYX_BYTECODE_STACK_RETURN,
    SYX_BYTECODE_DUPLICATE,
    SYX_BYTECODE_BRANCH,
    SYX_BYTECODE_BRANCH_IF_TRUE,
    SYX_BYTECODE_BRANCH_IF_FALSE,
  } SyxBytecodeSpecial;

//! Constants pushed by SYX_BYTECODE_PUSH_CONSTANT
typedef enum
  {
    SYX_BYTECODE_CONST_NIL,
    SYX_BYTECODE_CONST_TRUE,
    SYX_BYTECODE_CONST_FALSE,

    //! This constant is resolved at runtime by the interpreter
    SYX_BYTECODE_CONST_CONTEXT,
  } SyxBytecodeConstant;


//! Type of signals emitted in the Smalltalk environment
/*!
  Each type is relative to a class defined in syx-error.c
*/
typedef enum
  {
    SYX_ERROR_INTERP,
    SYX_ERROR_NOT_FOUND,
  } SyxSignalInternal;

#endif /* SYX_ENUMS_H */