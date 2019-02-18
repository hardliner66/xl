// ****************************************************************************
//  comparison.xs                                   XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Specification for comparisons between types
//
//     Comparisons are binary operators returning boolean values
//
//
//
//
//
//
// ****************************************************************************
//  (C) 2019 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************
//
// Comparisions usually represent mathematical equality or ordering.
// However, computer scientists sometimes chose to diverge from mathematics.
//
// Notably, IEEE-754, the most common standard for floating point, has
// very special cases for NaN values (Not a Number).
// If X is a NaN, no comparison returns true. So X = X is false, meaning
// that equality is not reflexive. But then X <> X is also false...
//
// IEEE-754 has the same problem with order relationships and NaN values
//
// This has two consequences for the design of XL libraries:
// 1) The properties of comparisons are described on a case-by-case basis
// 2) XL splits the `real` and `ieee` types (see REAL module)


use BOOLEAN

type equatable with
// ----------------------------------------------------------------------------
//   Type that has equality operators
// ----------------------------------------------------------------------------
//   For many basic types, this can be implemented using a bitwise comparison
//   If not, it is usually only necessary to implement `Left=Right` as long
//   as there is a guarantee that `X<>Y` is the opposite, although for
//   optimization reasons, it may be useful to provide both.

    with
        Left    : equatable
        Right   : equatable
    do
        Left =  Right           as boolean
        Left <> Right           as boolean is not Left = Right

    // Indicate if behaves like a true mathematical equality (default is true)
    SYMMETRIC                   as boolean is true // X = Y  => Y = X
    REFLEXIVE                   as boolean is true // X = X is true
    TRANSITIVE                  as boolean is true // X = Y and Y = Z => X = Z
    EXCLUSIVE                   as boolean is true // not ((X=Y) and (X<>Y))
    TOTAL                       as boolean is true // (X = Y) xor (X <> Y)
    BITWISE                     as boolean is true // Bitwise comparison works


type identifiable with
// ----------------------------------------------------------------------------
//   A type that has identity operators (same entity for the computer)
// ----------------------------------------------------------------------------
//   For references, the "equality" checks that the objects have the same value
//   whereas the "identity" checks that the objects have the same address
//   If your equality is reflexive, identity implies equality

    with
        Left    : identifiable
        Right   : identifiable
    do
        Left == Right           as boolean
        Left != Right           as boolean is Left == Right


type ordered with
// ----------------------------------------------------------------------------
//   Type with an ordering between elements
// ----------------------------------------------------------------------------
//   In general, it is sufficient to implement a <= operator for the type
//   to become ordered. Other operators can be implemented as optimizations.

    with
        Left    : ordered
        Right   : ordered
    do
        Left <= Right           as boolean

        Left >= Right           as boolean is     Right <= Left
        Left <  Right           as boolean is not Right <= Left
        Left >  Right           as boolean is not Left  <= Right

    // Indicate what kind of order we are dealing with
    ANTISYMMETRIC           as boolean // X<=Y and Y<=X => X=Y
    TRANSITIVE              as boolean // X<=Y and Y<=Z => X<=Z
    CONNEX                  as boolean // X<=Y or Y<=X is true
