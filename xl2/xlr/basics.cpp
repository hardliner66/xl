// ****************************************************************************
//  basics.cpp                      (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     Basic operations (arithmetic, ...)
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "basics.h"
#include "context.h"
#include "renderer.h"
#include <iostream>


XL_BEGIN

// ============================================================================
// 
//    Top-level operation
// 
// ============================================================================

ReservedName *true_name = NULL;
ReservedName *false_name = NULL;
ReservedName *nil_name = NULL;

#include "opcodes_declare.h"
#include "basics.tbl"


void EnterBasics(Context *c)
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in basics.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_define.h"
#include "basics.tbl"
}



// ============================================================================
// 
//    Type matching
// 
// ============================================================================

Tree *BooleanType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a boolean value (true/false)
// ----------------------------------------------------------------------------
{
    if (value == true_name || value == false_name)
        return value;
    return NULL;
}


Tree *IntegerType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an integer
// ----------------------------------------------------------------------------
{
    if (Integer *it = dynamic_cast<Integer *>(value->Run(stack)))
        return it;
    return NULL;
}


Tree *RealType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a real
// ----------------------------------------------------------------------------
{
    if (Real *rt = dynamic_cast<Real *>(value->Run(stack)))
        return rt;
    return NULL;
}


Tree *TextType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a text
// ----------------------------------------------------------------------------
{
    if (Text *tt = dynamic_cast<Text *>(value->Run(stack)))
    {
        Quote q;
        if (tt->Opening() != q.Opening() || tt->Closing() != q.Closing())
            return tt;
    }
    return NULL;
}


Tree *CharacterType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an integer
// ----------------------------------------------------------------------------
{
    if (Text *tt = dynamic_cast<Text *>(value->Run(stack)))
    {
        Quote q;
        if (tt->Opening() == q.Opening() && tt->Closing() == q.Closing())
            return tt;
    }
    return NULL;
}


Tree *AnyType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Don't really check the argument
// ----------------------------------------------------------------------------
{
    return value;
}


Tree *InfixType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as an infix
// ----------------------------------------------------------------------------
{
    if (Infix *it = dynamic_cast<Infix *>(value->Run(stack)))
        return it;
    return NULL;
}


Tree *PrefixType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a prefix
// ----------------------------------------------------------------------------
{
    if (Prefix *it = dynamic_cast<Prefix *>(value->Run(stack)))
        return it;
    return NULL;
}


Tree *PostfixType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a postfix
// ----------------------------------------------------------------------------
{
    if (Postfix *it = dynamic_cast<Postfix *>(value->Run(stack)))
        return it;
    return NULL;
}


Tree *BlockType::TypeCheck(Stack *stack, Tree *value)
// ----------------------------------------------------------------------------
//   Check if argument can be evaluated as a block
// ----------------------------------------------------------------------------
{
    if (Block *it = dynamic_cast<Block *>(value->Run(stack)))
        return it;
    return NULL;
}

XL_END