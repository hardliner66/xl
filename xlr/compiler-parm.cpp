// ****************************************************************************
//  compiler-parm.cpp                                              XLR project
// ****************************************************************************
// 
//   File Description:
// 
//    Actions collecting parameters on the left of a rewrite
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "compiler-parm.h"
#include "errors.h"

XL_BEGIN


bool ParameterList::EnterName(Name *what,
                              const llvm::Type *type,
                              bool globalCheck)
// ----------------------------------------------------------------------------
//   Enter a name in the parameter list
// ----------------------------------------------------------------------------
{
    // We only allow names here, not symbols (bug #154)
    if (what->value.length() == 0 || !isalpha(what->value[0]))
        Ooops("The pattern variable $1 is not a name", what);

    // Check if the name already exists in parameter list, e.g. in 'A+A'
    text name = what->value;
    Parameters::iterator it;
    for (it = parameters.begin(); it != parameters.end(); it++)
    {
        if ((*it).name->value == name)
        {
            if (type == compiler->treePtrTy || type == (*it).type)
                return true;

            Ooops("Conflicting types for $1", what);
            return false;
        }
    }

    // Check if the name already exists in context, e.g. 'false'
    if (globalCheck)
        if (context->Bound(what))
            return true;
        
    // We need to record a new parameter
    parameters.push_back(Parameter(what, type));
    return true;
}


bool ParameterList::DoInteger(Integer *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return true;
}


bool ParameterList::DoReal(Real *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return true;
}


bool ParameterList::DoText(Text *what)
// ----------------------------------------------------------------------------
//   Nothing to do for leaves
// ----------------------------------------------------------------------------
{
    return true;
}


bool ParameterList::DoName(Name *what)
// ----------------------------------------------------------------------------
//    Identify the named parameters being defined in the shape
// ----------------------------------------------------------------------------
{
    if (!defined)
    {
        // The first name we see must match exactly, e.g. 'sin' in 'sin X'
        defined = what;
        return true;
    }
    else
    {
        // We need to record a new parameter, type is Tree * by default
        return EnterName(what, compiler->treePtrTy, true);
    }
}


bool ParameterList::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Parameters may be in a block, we just look inside
// ----------------------------------------------------------------------------
{
    return what->child->Do(this);
}


bool ParameterList::DoInfix(Infix *what)
// ----------------------------------------------------------------------------
//   Check if we match an infix operator
// ----------------------------------------------------------------------------
{
    // Check if we match a type, e.g. 2 vs. 'K : integer'
    if (what->name == ":")
    {
        // Check the variable name, e.g. K in example above
        if (Name *varName = what->left->AsName())
        {
            // Enter a name in the parameter list with adequate machine type
            llvm_type mtype = compiler->MachineType(context, what->right);
            return EnterName(varName, mtype, false);
        }
        else
        {
            // We ar specifying the type of the expression, e.g. (X+Y):integer
            if (returned || defined)
            {
                Ooops("Cannot specify type of $1", what->left);
                return false;
            }

            // Remember the specified returned value
            returned = compiler->MachineType(context, what->right);

            // Keep going with the left-hand side
            return what->left->Do(this);
        }
    }

    // If this is the first one, this is what we define
    if (!defined)
        defined = what;

    // Otherwise, test left and right
    if (!what->left->Do(this))
        return false;
    if (!what->right->Do(this))
        return false;
    return true;
}


bool ParameterList::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   For prefix expressions, simply test left then right
// ----------------------------------------------------------------------------
{
    // In 'if X then Y', 'then' is defined first, but we want 'if'
    Infix *defined_infix = defined->AsInfix();
    if (defined_infix)
        defined = NULL;

    if (!what->left->Do(this))
        return false;
    if (!what->right->Do(this))
        return false;

    if (!defined && defined_infix)
        defined = defined_infix;

    return true;
}


bool ParameterList::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//    For postfix expressions, simply test right, then left
// ----------------------------------------------------------------------------
{
    // Note that ordering is reverse compared to prefix, so that
    // the 'defined' names is set correctly
    if (!what->right->Do(this))
        return false;
    if (!what->left->Do(this))
        return false;
    return true;
}


void ParameterList::Signature(llvm_types &signature)
// ----------------------------------------------------------------------------
//   Extract the types from the parameter list
// ----------------------------------------------------------------------------
{
    for (Parameters::iterator p=parameters.begin(); p!=parameters.end(); p++)
        signature.push_back((*p).type);
}

XL_END