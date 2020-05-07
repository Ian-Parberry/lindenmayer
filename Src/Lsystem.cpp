/// \file Lsystem.cpp
/// \brief Code for LProduction and LSystem.

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

#include "Types.h"
#include <sstream>

#include "Lsystem.h"

///////////////////////////////////////////////////////////////////////////////
// LProduction: Rule data structure for Lindenmayer Systems

#pragma region LProduction

/// \param lhs Left hand side of production.
/// \param rhs Right hand side of production.
/// \param fProb Production probability (defaults to 1).

LProduction::LProduction(char lhs, const std::wstring rhs, float fProb):
  m_chLHS(lhs), m_strRHS(rhs), m_fProb(fProb){
} //constructor

#pragma endregion LProduction

///////////////////////////////////////////////////////////////////////////////
// LSystem: A Lindenmayer System

// Settings functions (functions that change LSystem's state)

#pragma region Settings functions

/// Add a new production. The new production is inserted into m_mapRules,
/// that is, the left-hand side is mapped to a vector of right-hand sides
/// to which the new right-hand side is appended. The new rule is also appended
/// to the rule string for display.
/// \param rule A production.

void LSystem::AddRule(const LProduction& rule){
  if(rule.m_fProb < 1)m_bStochastic = true;

  auto p = m_mapRules.find(rule.m_chLHS);

  if(p == m_mapRules.end()){
    std::vector<LProduction> v;
    v.push_back(rule);
    m_mapRules.insert(std::make_pair(rule.m_chLHS, v));
  } //if

  else p->second.push_back(rule);

  //add rule to rule string for display
  
  m_strRuleString += rule.m_chLHS;
  m_strRuleString += L" \u2192 " + (std::wstring)rule.m_strRHS;

  //a bit of fuss here to get the probability with only 2 digits precision

  if(m_bStochastic){
    std::wstringstream wstream; //weird but necessary
    wstream.precision(2); //2 digits after the decimal point
    wstream << std::fixed << rule.m_fProb; //apply it
    m_strRuleString += L" (" + wstream.str() + L")"; //append to rule string
  } //if

  m_strRuleString += L"\n"; //end rule
} //AddRule

/// Set the root, that is, store it in `m_strRoot` and prepend it to the rule
/// string for display. 
/// \param omega The new root.

void LSystem::SetRoot(const std::wstring& omega){
  m_strRoot = omega; //set the root
  m_strRuleString = L"Root is " + omega + L"\n" + m_strRuleString; //prepend
} //SetRoot

/// Clear the rules, the rule string, the root string, the generation buffers,
/// and the settings.

void LSystem::Clear(){
  m_mapRules.clear();
  m_strRuleString.clear();
  m_strRoot.clear();
  m_strBuffer[0].clear(); 
  m_strBuffer[1].clear(); 
  m_bStochastic = false;
} //Clear

#pragma endregion Settings functions

///////////////////////////////////////////////////////////////////////////////
// Generate

#pragma region Generate

/// Generate a string from the root by applying the L-system productions in
/// parallel, and repeating for a fixed number of generations. Double-buffering
/// is used, that is, if generation \f$i\f$ is stored in m_strBuffer[\f$j\f$],
/// where \f$j \in \{0,1\}\f$, then generation \f$i+1\f$ is stored in
/// m_strBuffer[\f$j + 1 \pmod 2\f$]. 
/// \param n The number of generations.

void LSystem::Generate(int n){
  m_nGenerations = n;

  std::wstring* pSrc = &m_strBuffer[0]; //source buffer
  std::wstring* pDest = &m_strBuffer[1]; //destination buffer

  *pSrc = m_strRoot; //copy root string to source buffer
 
  for(int i=0; i<n; i++){ //for each generation 
    pDest->clear();

    for(size_t i=0; i<pSrc->size(); i++){ //for each char in source
      bool bRuleApplied = false; //whether a rule has been applied yet

      auto p = m_mapRules.find((*pSrc)[i]);

      if(p != m_mapRules.end()){
        float fProb = 0; //cumulative probability
        const float fRand = m_cRandom.randf(); //random value in [0, 1]

        for(auto rule: p->second){ //for each production that applies
          fProb += rule.m_fProb; //accumulate probability

          if(fRand <= fProb){ //use the current rule
            *pDest += rule.m_strRHS; //apply rule
            bRuleApplied = true; //record that a rule was applied
            break; //no need to try more rules
          } //if
        } //for
      } //if

      if(!bRuleApplied) //no rule was applied to current symbol
        *pDest += (*pSrc)[i]; //just copy over the current symbol
    } //for

    std::swap(pSrc, pDest); //swap generation buffers 
  } //for

  m_pResult = pDest; //copy the latest string to the result string
} //Generate

#pragma endregion Generate

///////////////////////////////////////////////////////////////////////////////
// Reader functions

#pragma region Reader functions

/// Reader function for the result string.
/// \return A const reference to the result string.

const std::wstring& LSystem::GetString() const{
  return *m_pResult;
} //GetString

/// Reader function for the rule string.
/// \return A const reference to the rule string.

const std::wstring& LSystem::GetRuleString() const{
  return m_strRuleString;
} //GetRuleString

/// Reader function for the current number of generations.
/// \return The current number of generations.

const UINT LSystem::GetGenerations() const{
  return m_nGenerations;
} //GetGenerations

/// Reader function for the stochasticity flag.
/// \return true if the current rules are stochastic.

const bool LSystem::IsStochastic() const{
  return m_bStochastic;
} //IsStochastic

#pragma endregion Reader functions
