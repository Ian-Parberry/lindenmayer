/// \file Random.h
/// \brief Interface for the pseudorandom number generator CRandom.

// MIT License
//
// Copyright (c) 2020 Ian Parberry
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#pragma once

#include "Includes.h"

/// \brief Pseudorandom Number Generator (PRNG for short).
///
/// A simple pseudorandom number generator based on xorshift128. It
/// can be seeded with the time or, if reproducability is desired (eg. when
/// debugging), with a fixed seed.

class CRandom{
  private: 
    UINT m_uState[4]; ///< Current state.

  public:
    CRandom(); ///< Constructor.

    void srand(int seed=-1); ///< Seed the random number generator.

    UINT randn(); ///< Get random unsigned integer.
    UINT randn(UINT i, UINT j); ///< Get random integer in \f$[i,j]\f$.
    float randf(); ///< Get random floating point number.
}; //CRandom
