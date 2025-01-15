// OpenSTA, Static Timing Analyzer
// Copyright (c) 2024, Parallax Software, Inc.
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

#include "UnitDelayNoInvBufCalc.hh"

#include "Units.hh"
#include "Network.hh"
#include "Liberty.hh"

namespace sta {

ArcDelayCalc *
makeUnitDelayNoInvBufCalc(StaState *sta)
{
  return new UnitDelayNoInvBufCalc(sta);
}

UnitDelayNoInvBufCalc::UnitDelayNoInvBufCalc(StaState *sta) :
  ArcDelayCalc(sta)
{
}

ArcDelayCalc *
UnitDelayNoInvBufCalc::copy()
{
  return new UnitDelayNoInvBufCalc(this);
}

Parasitic *
UnitDelayNoInvBufCalc::findParasitic(const Pin *,
			             const RiseFall *,
			             const DcalcAnalysisPt *)
{
  return nullptr;
}

Parasitic *
UnitDelayNoInvBufCalc::reduceParasitic(const Parasitic *,
                                       const Pin *,
                                       const RiseFall *,
                                       const DcalcAnalysisPt *)
{
  return nullptr;
}

void
UnitDelayNoInvBufCalc::reduceParasitic(const Parasitic *,
                                       const Net *,
                                       const Corner *,
                                       const MinMaxAll *)
{
}

void
UnitDelayNoInvBufCalc::setDcalcArgParasiticSlew(ArcDcalcArg &,
                                                const DcalcAnalysisPt *)
{
}

void
UnitDelayNoInvBufCalc::setDcalcArgParasiticSlew(ArcDcalcArgSeq &,
                                                const DcalcAnalysisPt *)
{
}

ArcDcalcResult
UnitDelayNoInvBufCalc::inputPortDelay(const Pin *drvr_pin,
			              float,
			              const RiseFall *,
			              const Parasitic *,
                                      const LoadPinIndexMap &load_pin_index_map,
                                      const DcalcAnalysisPt *)
{
  return unitDelayResult(drvr_pin, load_pin_index_map);
}

ArcDcalcResult
UnitDelayNoInvBufCalc::gateDelay(const Pin *drvr_pin,
                                 const TimingArc *,
			         const Slew &,
			         float,
			         const Parasitic *,
                                 const LoadPinIndexMap &load_pin_index_map,
                                 const DcalcAnalysisPt *)
{
  return unitDelayResult(drvr_pin, load_pin_index_map);
}

ArcDcalcResultSeq
UnitDelayNoInvBufCalc::gateDelays(ArcDcalcArgSeq &dcalc_args,
                                  const LoadPinIndexMap &load_pin_index_map,
                                  const DcalcAnalysisPt *)
{
  size_t drvr_count = dcalc_args.size();
  ArcDcalcResultSeq dcalc_results(drvr_count);
  for (size_t drvr_idx = 0; drvr_idx < drvr_count; drvr_idx++) {
    ArcDcalcResult &dcalc_result = dcalc_results[drvr_idx];
    ArcDcalcArg dcalc_arg = dcalc_args[drvr_idx];
    dcalc_result = unitDelayResult(dcalc_arg.drvrPin(), load_pin_index_map);
  }
  return dcalc_results;
}

ArcDcalcResult
UnitDelayNoInvBufCalc::unitDelayResult(const Pin *drvr_pin,
	                               const LoadPinIndexMap &load_pin_index_map)
{
  ArcDelay arc_delay = 0.0;
  Instance *inst = network_->instance(drvr_pin);
  if (inst) {
    LibertyCell *libcell = network_->libertyCell(inst);
    if (libcell && !libcell->isInverter() && !libcell->isBuffer())
      arc_delay = units_->timeUnit()->scale();
  }
  size_t load_count = load_pin_index_map.size();
  ArcDcalcResult dcalc_result(load_count);
  dcalc_result.setGateDelay(arc_delay);
  dcalc_result.setDrvrSlew(0.0);
  for (size_t load_idx = 0; load_idx < load_count; load_idx++) {
    dcalc_result.setWireDelay(load_idx, 0.0);
    dcalc_result.setLoadSlew(load_idx, 0.0);
  }
  return dcalc_result;
}

string
UnitDelayNoInvBufCalc::reportGateDelay(const Pin *drvr_pin,
                                       const TimingArc *,
			               const Slew &,
			               float,
			               const Parasitic *,
                                       const LoadPinIndexMap &,
			               const DcalcAnalysisPt *,
			               int)
{
  string result = "Delay = 0.0\n";
  Instance *inst = network_->instance(drvr_pin);
  if (inst) {
    LibertyCell *libcell = network_->libertyCell(inst);
    if (libcell && !libcell->isInverter() && !libcell->isBuffer())
      result = "Delay = 1.0\n";
  }
  result += "Slew = 0.0\n";
  return result;
}

ArcDelay 
UnitDelayNoInvBufCalc::checkDelay(const Pin *drvr_pin,
                                  const TimingArc *,
			          const Slew &,
			          const Slew &,
			          float,
			          const DcalcAnalysisPt *)
{
  ArcDelay arc_delay = 0.0;
  Instance *inst = network_->instance(drvr_pin);
  if (inst) {
    LibertyCell *libcell = network_->libertyCell(inst);
    if (libcell && !libcell->isInverter() && !libcell->isBuffer())
      arc_delay = units_->timeUnit()->scale();
  }
  return arc_delay;
}

string
UnitDelayNoInvBufCalc::reportCheckDelay(const Pin *drvr_pin,
                                        const TimingArc *,
				        const Slew &,
				        const char *,
				        const Slew &,
				        float,
				        const DcalcAnalysisPt *,
				        int)
{
  string result = "Check = 0.0\n";
  Instance *inst = network_->instance(drvr_pin);
  if (inst) {
    LibertyCell *libcell = network_->libertyCell(inst);
    if (libcell && !libcell->isInverter() && !libcell->isBuffer())
      result = "Check = 1.0\n";
  }
  return result;
}

void
UnitDelayNoInvBufCalc::finishDrvrPin()
{
}

} // namespace
