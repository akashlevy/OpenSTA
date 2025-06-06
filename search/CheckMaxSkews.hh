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

#include "GraphClass.hh"
#include "Delay.hh"
#include "StaState.hh"
#include "SearchClass.hh"
#include "Path.hh"

namespace sta {

class MaxSkewCheckVisitor;

class CheckMaxSkews
{
public:
  explicit CheckMaxSkews(StaState *sta);
  ~CheckMaxSkews();
  void clear();
  // All violating max skew checks.
  MaxSkewCheckSeq &violations();
  // Max skew check with the least slack.
  MaxSkewCheck *minSlackCheck();

protected:
  void visitMaxSkewChecks(MaxSkewCheckVisitor *visitor);
  void visitMaxSkewChecks(Vertex *vertex,
			  MaxSkewCheckVisitor *visitor);

  MaxSkewCheckSeq checks_;
  StaState *sta_;
};

class MaxSkewCheck
{
public:
  MaxSkewCheck(Path *clk_path,
	       Path *ref_path,
	       TimingArc *check_arc,
	       Edge *check_edge);
  const Path *clkPath() const { return clk_path_; }
  Pin *clkPin(const StaState *sta) const;
  const Path *refPath() const { return ref_path_; }
  Pin *refPin(const StaState *sta) const;
  Delay skew() const;
  ArcDelay maxSkew(const StaState *sta) const;
  Slack slack(const StaState *sta) const;
  TimingArc *checkArc() const { return check_arc_; }

private:
  Path *clk_path_;
  Path *ref_path_;
  TimingArc *check_arc_;
  Edge *check_edge_;
};

class MaxSkewSlackLess
{
public:
  explicit MaxSkewSlackLess(const StaState *sta);
  bool operator()(const MaxSkewCheck *check1,
		  const MaxSkewCheck *check2) const;

protected:
  const StaState *sta_;
};

} // namespace
