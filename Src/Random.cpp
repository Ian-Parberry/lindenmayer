/// \file Random.cpp
/// \brief Code for the pseudorandom number generator CRandom.

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

#pragma comment(lib,"Winmm.lib")

#include <Windows.h>

#include "Random.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor and initialization

#pragma region Constructor and initialization

/// Seed the PRNG with an unpredictable value drawn from a timer.
/// Call srand() later to override this.

CRandom::CRandom(){ 
  srand();
} //constructor

/// If the seed is negative (which it is by default if no parameter is 
/// supplied), then use timeGetTime instead (which is, one hopes, unpredictable).
/// The state variables for xorshift128 are initialized using the C Standard
/// Library function rand() seeded using the pro-offered seed value.
/// \param seed The seed, defaults to -1.

void CRandom::srand(int seed){ 
  ::srand((seed >= 0)? seed: timeGetTime() & 0x7FFF);

  m_uState[0] = UINT(::rand());

  for(int i=1; i<=3; i++)
    m_uState[i] = m_uState[i - 1]*UINT(::rand());
} //srand

#pragma endregion Constructor and initialization

///////////////////////////////////////////////////////////////////////////////
// Functions that generate pseudo-random numbers

#pragma region Generate pseudo-random numbers

/// Generate a pseudorandom unsigned integer using xorshift128. This is the one
/// that does the actual work here: The other pseudorandom generation functions
/// rely on it to do the heavy lifting. Algorithm snarfed from the interwebs
/// someplace, I don't quite remember where.
/// \return A pseudorandom unsigned integer.

UINT CRandom::randn(){ 
	UINT s = m_uState[3];

	s ^= s << 11;
	s ^= s >> 8;

	m_uState[3] = m_uState[2]; 
  m_uState[2] = m_uState[1]; 
  m_uState[1] = m_uState[0];

	s ^= m_uState[0];
	s ^= m_uState[0] >> 19;	

	m_uState[0] = s;

	return s;
} //randn

/// Generate a pseudorandom unsigned integer within a range.
/// \param i Bottom of range.
/// \param j Top of range.
/// \return A random positive integer r such that i \f$\leq\f$ r \f$\leq\f$ j.

UINT CRandom::randn(UINT i, UINT j){  
  return randn()%(j - i + 1) + i;
} //randn

/// Generate a pseudorandom floating positive point number 
/// in \f$[0,1]\f$by generatings a pseudorandom unsigned
/// integer and dividing it by \f$2^{32} - 1\f$.
/// \return A pseudorandom floating point number from \f$[0,1]\f$.

float CRandom::randf(){
  return (float)randn()/(float)0xFFFFFFFF;
} //randf

#pragma endregion Generate pseudo-random numbers
