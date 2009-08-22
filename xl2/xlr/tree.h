#ifndef TREE_H
#define TREE_H
// ****************************************************************************
//  tree.h                          (C) 1992-2003 Christophe de Dinechin (ddd)
//                                                            XL2 project
// ****************************************************************************
//
//   File Description:
//
//     Basic representation of parse tree.
//
//     See the big comment at the top of parser.h for details about
//     the basics of XL tree representation
//
//
//
//
//
//
// ****************************************************************************
// This program is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html for details
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "base.h"
#include <map>
#include <vector>


XL_BEGIN

// ============================================================================
//
//    The types being defined or used to define XL trees
//
// ============================================================================

struct Context;                                 // Execution context
struct Tree;                                    // Base tree
struct Integer;                                 // Integer: 0, 3, 8
struct Real;                                    // Real: 3.2, 1.6e4
struct Text;                                    // Text: "ABC"
struct Name;                                    // Name / symbol: ABC, ++-
struct Prefix;                                  // Prefix: sin X
struct Postfix;                                 // Postfix: 3!
struct Infix;                                   // Infix: A+B, newline
struct Block;                                   // Block: (A), {A}
struct Action;                                  // Action on trees
struct Symbols;                                 // Symbol table
typedef ulong tree_position;                    // Position in context
typedef std::vector<Tree *> tree_list;          // A list of trees
typedef Tree *(*eval_fn) (Tree *);              // Compiled evaluation code



// ============================================================================
// 
//    The Tree class
// 
// ============================================================================

enum kind
// ----------------------------------------------------------------------------
//   The kinds of tree that compose an XL parse tree
// ----------------------------------------------------------------------------
{
    INTEGER, REAL, TEXT, NAME,                  // Leaf nodes
    BLOCK, PREFIX, POSTFIX, INFIX               // Non-leaf nodes
};


struct Tree
// ----------------------------------------------------------------------------
//   The base class for all XL trees
// ----------------------------------------------------------------------------
{
    enum { NOWHERE = ~0UL };
    enum { KINDBITS = 3, KINDMASK=7 };

    // Constructor and destructor
    Tree (kind k, tree_position pos = NOWHERE):
        tag((pos<<KINDBITS) | k), code(NULL), symbols(NULL), type(NULL) {}
    ~Tree() {}

    // Perform recursive actions on a tree
    Tree *              Do(Action *action);
    Tree *              Do(Action &action)    { return Do(&action); }

    // Attributes
    kind                Kind()                { return kind(tag & KINDMASK); }
    tree_position       Position()            { return tag>>KINDBITS; }
    bool                IsLeaf()              { return Kind() <= NAME; }
    bool                IsConstant()          { return Kind() <= TEXT; }

    // Safe cast to an appropriate subclass
    Integer *           AsInteger();
    Real *              AsReal();
    Text *              AsText();
    Name *              AsName();
    Block *             AsBlock();
    Infix *             AsInfix();
    Prefix *            AsPrefix();
    Postfix *           AsPostfix();

    // Conversion to text
                        operator text();

    // Operator new to record the tree in the garbage collector
    void *              operator new(size_t sz);

public:
    ulong       tag;                            // Position + kind
    eval_fn     code;                           // Compiled code
    Symbols *   symbols;                        // Local symbols
    Tree *      type;                           // Type information
};


struct Action
// ----------------------------------------------------------------------------
//   An operation we do recursively on trees
// ----------------------------------------------------------------------------
{
    Action () {}
    virtual ~Action() {}
    virtual Tree *Do (Tree *what) = 0;

    // Specialization for the canonical nodes, default is to run them
    virtual Tree *DoInteger(Integer *what);
    virtual Tree *DoReal(Real *what);
    virtual Tree *DoText(Text *what);
    virtual Tree *DoName(Name *what);
    virtual Tree *DoPrefix(Prefix *what);
    virtual Tree *DoPostfix(Postfix *what);
    virtual Tree *DoInfix(Infix *what);
    virtual Tree *DoBlock(Block *what);
};



// ============================================================================
//
//   Leaf nodes (integer, real, name, text)
//
// ============================================================================

struct Integer : Tree
// ----------------------------------------------------------------------------
//   Integer constants
// ----------------------------------------------------------------------------
{
    Integer(longlong i = 0, tree_position pos = NOWHERE):
        Tree(INTEGER, pos), value(i) {}
    longlong            value;
    operator longlong()         { return value; }
};


struct Real : Tree
// ----------------------------------------------------------------------------
//   Real numbers
// ----------------------------------------------------------------------------
{
    Real(double d = 0.0, tree_position pos = NOWHERE):
        Tree(REAL, pos), value(d) {}
    double              value;
    operator double()           { return value; }
};


struct Text : Tree
// ----------------------------------------------------------------------------
//   Text, e.g. "Hello World"
// ----------------------------------------------------------------------------
{
    Text(text t, text open="\"", text close="\"", tree_position pos=NOWHERE):
        Tree(TEXT, pos), value(t), opening(open), closing(close) {}
    text                value;
    text                opening, closing;
    static text         textQuote, charQuote;
    operator text()             { return value; }
};


struct Name : Tree
// ----------------------------------------------------------------------------
//   A node representing a name or symbol
// ----------------------------------------------------------------------------
{
    Name(text n, tree_position pos = NOWHERE):
        Tree(NAME, pos), value(n) {}
    text                value;
    operator bool();
};



// ============================================================================
//
//   Structured types: Block, Prefix, Infix
//
// ============================================================================

struct Block : Tree
// ----------------------------------------------------------------------------
//   A block, such as (X), {X}, [X] or indented block
// ----------------------------------------------------------------------------
{
    Block(Tree *c, text open, text close, tree_position pos = NOWHERE):
        Tree(BLOCK, pos), child(c), opening(open), closing(close) {}
    Tree *              child;
    text                opening, closing;
    static text         indent, unindent;
};


struct Prefix : Tree
// ----------------------------------------------------------------------------
//   A prefix operator, e.g. sin X, +3
// ----------------------------------------------------------------------------
{
    Prefix(Tree *l, Tree *r, tree_position pos = NOWHERE):
        Tree(PREFIX, pos), left(l), right(r) {}
    Tree *              left;
    Tree *              right;
};


struct Postfix : Tree
// ----------------------------------------------------------------------------
//   A postfix operator, e.g. 3!
// ----------------------------------------------------------------------------
{
    Postfix(Tree *l, Tree *r, tree_position pos = NOWHERE):
        Tree(POSTFIX, pos), left(l), right(r) {}
    Tree *              left;
    Tree *              right;
};


struct Infix : Tree
// ----------------------------------------------------------------------------
//   Infix operators, e.g. A+B, A and B, A,B,C,D,E
// ----------------------------------------------------------------------------
{
    Infix(text n, Tree *l, Tree *r, tree_position pos = NOWHERE):
        Tree(INFIX, pos), left(l), right(r), name(n) {}
    Tree *              left;
    Tree *              right;
    text                name;
};


// ============================================================================
//
//    Safe casts
//
// ============================================================================

inline Integer *Tree::AsInteger()
// ----------------------------------------------------------------------------
//    Return a pointer to an Integer or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == INTEGER)
        return (Integer *) this;
    return NULL;
}


inline Real *Tree::AsReal()
// ----------------------------------------------------------------------------
//    Return a pointer to an Real or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == REAL)
        return (Real *) this;
    return NULL;
}


inline Text *Tree::AsText()
// ----------------------------------------------------------------------------
//    Return a pointer to an Text or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == TEXT)
        return (Text *) this;
    return NULL;
}


inline Name *Tree::AsName()
// ----------------------------------------------------------------------------
//    Return a pointer to an Name or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == NAME)
        return (Name *) this;
    return NULL;
}


inline Block *Tree::AsBlock()
// ----------------------------------------------------------------------------
//    Return a pointer to an Block or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == BLOCK)
        return (Block *) this;
    return NULL;
}


inline Infix *Tree::AsInfix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Infix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == INFIX)
        return (Infix *) this;
    return NULL;
}


inline Prefix *Tree::AsPrefix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Prefix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == PREFIX)
        return (Prefix *) this;
    return NULL;
}


inline Postfix *Tree::AsPostfix()
// ----------------------------------------------------------------------------
//    Return a pointer to an Postfix or NULL
// ----------------------------------------------------------------------------
{
    if (this && Kind() == POSTFIX)
        return (Postfix *) this;
    return NULL;
}

// ============================================================================
// 
//    Tree shape equality comparison
// 
// ============================================================================

struct TreeMatch : Action
// ----------------------------------------------------------------------------
//   Check if two trees match in structure
// ----------------------------------------------------------------------------
{
    TreeMatch (Tree *t): test(t) {}
    Tree *DoInteger(Integer *what)
    {
        if (Integer *it = test->AsInteger())
            if (it->value == what->value)
                return what;
        return NULL;
    }
    Tree *DoReal(Real *what)
    {
        if (Real *rt = test->AsReal())
            if (rt->value == what->value)
                return what;
        return NULL;
    }
    Tree *DoText(Text *what)
    {
        if (Text *tt = test->AsText())
            if (tt->value == what->value)
                return what;
        return NULL;
    }
    Tree *DoName(Name *what)
    {
        if (Name *nt = test->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }

    Tree *DoBlock(Block *what)
    {
        // Test if we exactly match the block, i.e. the reference is a block
        if (Block *bt = test->AsBlock())
        {
            if (bt->opening == what->opening &&
                bt->closing == what->closing)
            {
                test = bt->child;
                Tree *br = what->child->Do(this);
                test = bt;
                if (br)
                    return br;
            }
        }
        return NULL;
    }
    Tree *DoInfix(Infix *what)
    {
        if (Infix *it = test->AsInfix())
        {
            // Check if we match the tree, e.g. A+B vs 2+3
            if (it->name == what->name)
            {
                test = it->left;
                Tree *lr = what->left->Do(this);
                test = it;
                if (!lr)
                    return NULL;
                test = it->right;
                Tree *rr = what->right->Do(this);
                test = it;
                if (!rr)
                    return NULL;
                return what;
            }
        }
        return NULL;
    }
    Tree *DoPrefix(Prefix *what)
    {
        if (Prefix *pt = test->AsPrefix())
        {
            // Check if we match the tree, e.g. f(A) vs. f(2)
            test = pt->left;
            Tree *lr = what->left->Do(this);
            test = pt;
            if (!lr)
                return NULL;
            test = pt->right;
            Tree *rr = what->right->Do(this);
            test = pt;
            if (!rr)
                return NULL;
            return what;
        }
        return NULL;
    }
    Tree *DoPostfix(Postfix *what)
    {
        if (Postfix *pt = test->AsPostfix())
        {
            // Check if we match the tree, e.g. A! vs 2!
            test = pt->right;
            Tree *rr = what->right->Do(this);
            test = pt;
            if (!rr)
                return NULL;
            test = pt->left;
            Tree *lr = what->left->Do(this);
            test = pt;
            if (!lr)
                return NULL;
            return what;
        }
        return NULL;
    }
    Tree *Do(Tree *what)
    {
        return NULL;
    }

    Tree *      test;
};

XL_END

#endif // TREE_H
