/// \file Lsystem.h
/// \brief Interface for LProduction and LSystem.

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

#include "Random.h"
#include "Includes.h"

////////////////////////////////////////////////////////////////////////////////
// class LProduction

#pragma region LProduction

/// \brief Stochastic context-free production. 
///
/// A production (also known as a rule) consists of a left-hand side, a
/// right-hand side, and the probability that the production will be applied in
/// a stochastic L-system. If your L-system is not stochastic, the probability
/// is set to 1.

class LProduction{
  public:
    char m_chLHS = '\0'; ///< Left-hand side of production.
    std::wstring m_wstrRHS; ///< Right-hand side of production.
    float m_fProb; ///< Probability of production applying.

    LProduction(char lhs, const std::wstring rhs, float fProb=1); ///< Constructor.
}; //LProduction

#pragma endregion LProduction

////////////////////////////////////////////////////////////////////////////////
// class LSystem

#pragma region LSystem

/// \brief A stochastic bracketed context-free L-system.
///
/// This basic context-free stochastic bracketed L-system can be used to
/// re-create some of the line drawings in ABOP. The productions are stored
/// in a `std::map<char, std::vector<LProduction>>` which maps the
/// left-hand side of a production to an `std::vector` of the productions that 
/// have that left-hand side. A text string m_wstrRuleString is used to store
/// a printable rule string in text form which is used to display the rules
/// on the window. Double-buffering in `m_wstrBuffer[2]` is used to generate the
/// result string `m_pResult`.

class LSystem{
  private: 
    CRandom m_cRandom; ///< PRNG.

    std::wstring m_wstrRoot; ///< Root string.

    std::map<wchar_t, std::vector<LProduction>> m_mapRules; ///< Productions.
    std::wstring m_wstrRuleString; ///< Rule string.

    std::wstring m_wstrBuffer[2]; ///< Generation buffers.
    std::wstring* m_pResult = m_wstrBuffer; ///< Pointer to generated string.

    bool m_bStochastic = false; ///< Includes a stochastic rule.
    UINT m_nGenerations = 0; ///< Number of generations.

  public:
    void SetRoot(const std::wstring& omega); ///< Set the root string.
    void AddRule(const LProduction& rule); ///< AddRule rule.

    void Clear(); ///< Clear the rules, buffers, and settings.
    void Generate(const UINT n); ///< Generate L-system from stored root and rules.

    const std::wstring& GetString() const; ///< Get generated string.
    const std::wstring& GetRuleString() const; ///< Get rule string.
    const UINT GetGenerations() const; ///< Get number of generations.

    const bool IsStochastic() const; ///< Is a stochastic L-system.
}; //LSystem

#pragma endregion LSystem
