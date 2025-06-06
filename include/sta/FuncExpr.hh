// OpenSTA, Static Timing Analyzer
// Copyright (c) 2025, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
// 
// The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.
// 
// Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 
// This notice may not be removed or altered from any source distribution.

#pragma once

#include <string>

#include "Set.hh"
#include "NetworkClass.hh"
#include "LibertyClass.hh"

namespace sta {

class FuncExpr
{
public:
  enum Operator {op_port,
		 op_not,
		 op_or,
		 op_and,
		 op_xor,
		 op_one,
		 op_zero};

  // Constructors.
  FuncExpr(Operator op,
	   FuncExpr *left,
	   FuncExpr *right,
	   LibertyPort *port);
  static FuncExpr *makePort(LibertyPort *port);
  static FuncExpr *makeNot(FuncExpr *expr);
  static FuncExpr *makeAnd(FuncExpr *left,
			   FuncExpr *right);
  static FuncExpr *makeOr(FuncExpr *left,
			  FuncExpr *right);
  static FuncExpr *makeXor(FuncExpr *left,
			   FuncExpr *right);
  static FuncExpr *makeZero();
  static FuncExpr *makeOne();
  static bool equiv(const FuncExpr *expr1,
		    const FuncExpr *expr2);
  static bool less(const FuncExpr *expr1,
		   const FuncExpr *expr2);

  // Deep copy.
  FuncExpr *copy();
  // Delete expression and all of its subexpressions.
  void deleteSubexprs();
  // op == op_port
  LibertyPort *port() const;
  Operator op() const { return op_; }
  // When operator is NOT left is the only operand.
  FuncExpr *left() const { return left_; }
  // nullptr when op == op_not
  FuncExpr *right() const { return right_; }
  TimingSense portTimingSense(const LibertyPort *port) const;
  // Return true if expression has port as an input.
  bool hasPort(const LibertyPort *port) const;
  std::string to_string() const;
  // Sub expression for a bus function (bit_offset is 0 to bus->size()-1).
  FuncExpr *bitSubExpr(int bit_offset);
  // Check to make sure the function and port size are compatible.
  // Return true if there is a mismatch.
  bool checkSize(size_t size);
  bool checkSize(LibertyPort *port);

private:
  std::string to_string(bool with_parens) const;
  std::string to_string(bool with_parens,
                        char op) const;

  Operator op_;
  FuncExpr *left_;
  FuncExpr *right_;
  LibertyPort *port_;
};

// Negate an expression.
FuncExpr *
funcExprNot(FuncExpr *expr);

class FuncExprPortIterator : public Iterator<LibertyPort*>
{
public:
  explicit FuncExprPortIterator(const FuncExpr *expr);
  virtual bool hasNext() { return iter_.hasNext(); }
  virtual LibertyPort *next() { return iter_.next(); }

private:
  void findPorts(const FuncExpr *expr);

  LibertyPortSet ports_;
  LibertyPortSet::ConstIterator iter_;
};

} // namespace
