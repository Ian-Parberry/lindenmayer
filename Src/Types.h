/// \file Types.h
/// \brief Useful types and structures.

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

///////////////////////////////////////////////////////////////////////////////
// Turtle graphics descriptor

#pragma region Turtle graphics descriptor

/// \brief Turtle graphics descriptor.
///
/// A descriptor for turtle graphics that describes the start state of the turtle.

class TurtleDesc{
  public:
    Gdiplus::PointF m_ptStart; ///< Start point.

    float m_fAngleDelta = 0; ///< Line angle delta.
    float m_fLength = 8; ///< Line length.
    float m_fLenMultiplier = 1; ///< Line length multiplier.
    float m_fPointSize = 1; ///< Line point size.
    
    /// \brief Default constructor.

    TurtleDesc(){}; 

    /// \brief Constructor.
    ///
    /// \param start Start position.
    /// \param angledelta Angle delta.
    /// \param len Line length.

    TurtleDesc(Gdiplus::PointF start, float angledelta, float len):
      m_ptStart(start), m_fAngleDelta(angledelta), m_fLength(len){
    }; //constructor
}; //TurtleDesc

#pragma endregion Turtle graphics descriptor

///////////////////////////////////////////////////////////////////////////////
// Stack frame

#pragma region Stack frame

/// \brief Stack frame.
///
/// A stack frame to be used in turtle graphics.

class StackFrame{
  public:
    Gdiplus::PointF m_ptPos; ///< Position.
    float m_fAngle = 0; ///< Rotation angle.
    float m_fLength = 0; ///< Length.  
    
    /// \brief Default constructor.

    StackFrame(){}; 

    /// \brief Constructor.
    ///
    /// \param pos Position.
    /// \param angle Angle.
    /// \param len Line length.

    StackFrame(Gdiplus::PointF pos, float angle, float len):
      m_ptPos(pos), m_fAngle(angle), m_fLength(len){
    }; //constructor
}; //StackFrame

#pragma endregion Stack frame
