# Liberty 2017.06 compliance

This is a construct-by-construct checklist of the
*Liberty User Guides and Reference Manual Suite Version 2017.06*
(`liberty2017.06.pdf`)
against OpenSTA (`liberty/LibertyReader.cc`, `liberty/LibertyBuilder.cc`,
delay calculation, power, and SSTA).

The 2017.06 suite contains:

- Liberty Release Notes 2017.06
- Liberty User Guide Volume 1 (2017.06)
- Liberty User Guide Volume 2 (unchanged since 2007.03; physical / FPGA)
- Liberty Reference Manual (2017.06)

OpenSTA is a gate-level STA. Synthesis, physical, characterization, FPGA
architecture, and noise constructs are listed even when they are out of
STA scope, so nothing is omitted by silence.

User Guide chapters name the features. The Reference Manual is the
authoritative list of every group and attribute. Chapter 7 and
[Timing group lookup tables](#timing-group-lookup-tables-rm-394) list
every table that can appear inside a `timing` group, including the NLDM
retain tables that are easy to miss because they sit *inside* a normal
delay arc rather than using a distinct `timing_type`.

## How to read the tables

| Status | Meaning |
| --- | --- |
| ✅ | Implemented: parsed and used for timing, power, SSTA, or a stored cell/pin property that STA queries. |
| ☑️ | Partial: parsed or stored but incomplete, unused at analysis time, or broken by a known defect. |
| ❌ | Ignored: the parser accepts the group/attribute (no syntax error) and discards it. |
| ⚠️ | Warned: explicitly warned or rejected (`delay_model` not supported, nested `include_file`, unknown enum, …). |

Unknown groups and attributes are ❌ by default. OpenSTA does not emit
“unrecognized attribute” warnings. A `.lib` can therefore appear to
“read cleanly” while dropping retain tables, compact CCS, CCSN, and
constraint LVF.

## Parser and runtime notes

- `read_liberty` uses a generic Liberty parser, then walks only the groups
  and attributes listed as ✅/☑️/⚠️ below.
- Nested `include_file` is an error. Non-nested `include_file` is ✅.
- Table models: scalar and 1–3 axes. `variable_4` / `index_4` are ❌.
- Polynomial tables (`orders`, `coefs`, `variable_n_range`) are ❌ even
  inside a `table_lookup` library.
- Default delay calculator is `dmp_ceff_elmore` (NLDM + DMP Ceff). CCS
  current waveforms are used only after `set_delay_calculator ccs_ceff`
  or `prima`.
- Function expressions: `*`, `&`, juxtaposition (AND); `+`, `|` (OR);
  `^` (XOR); `!`, postfix `'` (NOT); `0`/`1`; parentheses. No dedicated
  XNOR token.

## Known defects that look like missing features

These are implemented in name only, looked up under the wrong name, or
dropped next to data that *is* used.

| Item | What happens |
| --- | --- |
| NLDM retain tables `retaining_rise` / `retaining_fall` / `retain_rise_slew` / `retain_fall_slew` | Live inside a normal delay `timing` group beside `cell_rise` / `cell_fall`. `makeTableModels` never looks them up. Parent delay arcs are still built. Typical use: memory `tRETAIN` vs `tACC`. ❌ |
| Constraint LVF `ocv_sigma_rise_constraint` (and fall / std_dev / mean_shift / skewness) | Reader looks up `ocv_sigma_rise_constraiint` (extra `i`). Real groups are never bound. `doc/Examples.md` documents the correct names. ☑️ |
| `timing_type : nochange_*` | Enum is recognized; `LibertyBuilder` returns `nullptr`; no check arcs. SDF annotation reporting has a nochange role; Liberty no-change analysis is not implemented. ❌ |
| `timing_type : retaining_time` | Enum is recognized; builder returns `nullptr`. Not a documented 2017.06 `timing_type` value (2017.06 models retain with the tables above). ❌ |
| Timing-arc `receiver_capacitance*` | Stored on `GateTableModel`; no delay calculator reads `ReceiverModel`. ☑️ |
| `default_max_capacitance` | `LibertyLibrary::setDefaultMaxCapacitance` exists; the writer can emit it; the reader never sets it. Capacitance DRC then has no library default. ❌ |

---

## Release Notes 2017.06

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `ocv_sigma_retaining_rise` / `ocv_sigma_retaining_fall` | Retain-arc OCV delay. |
| ❌ | `ocv_sigma_retain_rise_slew` / `ocv_sigma_retain_fall_slew` | Retain-arc OCV slew. |
| ❌ | Moment-based retain LVF (`ocv_std_dev_retaining_*`, `ocv_mean_shift_retaining_*`, `ocv_skewness_retaining_*`, `ocv_std_dev_retain_*_slew`, `ocv_mean_shift_retain_*_slew`, `ocv_skewness_retain_*_slew`) | 2017.06 ULV retain moments. |
| ☑️ | Moment-based delay/slew/constraint LVF (`ocv_std_dev_cell_*`, `ocv_mean_shift_cell_*`, `ocv_skewness_cell_*`, same for `*_transition` and `*_constraint`) | Delay and slew ✅. Constraint groups hit the `constraiint` typo. |
| ❌ | `output_signal_level_low` / `output_signal_level_high` in a `timing` group | Partial voltage swing per arc. |

---

## User Guide Volume 1

### Chapter 1 — Sample library description / syntax

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `library (name) { … }` | Missing name is error 1141. Duplicate name warns 1140; a second library is still created (min/max corners). |
| ✅ | Simple attributes (`name : value ;`) | Only those the reader visits. |
| ✅ | Complex attributes (`name (v1, v2);`) | |
| ✅ | Nested groups | |
| ❌ | `define (attr, group, type)` | User-defined attributes are not interpreted. |
| ❌ | `define_group` | |
| ✅ | `include_file (filename)` | Filename only; search path is the process CWD. Nested includes are ⚠️. |
| ⚠️ | Nested `include_file` | Error: nested include_file's are not supported. |
| ❌ | `include_file` as an attribute value (not a statement) | Spec forbids this; not special-cased. |
| ❌ | `comment`, `date`, `revision` | Harmless metadata. Writer emits empty `comment`. |
| ❌ | `user_parameters` | Polynomial / user variables. |
| ❌ | `library_features` | Writer emits `library_features(report_delay_calculation)`; reader ignores it. |
| ✅ | Pin names ending in `!` | Parser accepts. |
| ✅ | `group(name1:name2)` colon arguments | Parser accepts. |
| ✅ | Float written as a string | Parser accepts. |
| ✅ | Backslash-newline continuation | Parser accepts. |

### Chapter 2 — Building a technology library

| Status | Construct | Notes |
| --- | --- | --- |
| ☑️ | `technology (cmos)` | Ignored except `technology (fpga)` forces linear delay model. |
| ☑️ | `technology (fpga)` | Sets `DelayModelType::cmos_linear` only. No FPGA timing. |
| ✅ | `delay_model : table_lookup` | NLDM. Default if omitted is `cmos_linear`. |
| ✅ | `delay_model : generic_cmos` | Linear `intrinsic_*` / `*_resistance`. |
| ⚠️ | `delay_model : piecewise_cmos` | Warned 1160; `DelayModelType::cmos_pwl` stored; no models. |
| ⚠️ | `delay_model : cmos2` | Warned 1161; no models. |
| ⚠️ | `delay_model : polynomial` | Warned 1162; no models. |
| ⚠️ | `delay_model : dcm` | Warned 1163; IBM DCM; no models. |
| ⚠️ | Any other `delay_model` | Warned 1164. |
| ✅ | `bus_naming_style` | Must be 6 characters of the form `%sXdY` (e.g. `%s[%d]`). Other styles warn 1165. |
| ✅ | `input_threshold_pct_rise` / `_fall` | Missing values warn 1145. Scaled by 0.01. |
| ✅ | `output_threshold_pct_rise` / `_fall` | Missing values warn 1146. |
| ✅ | `slew_lower_threshold_pct_rise` / `_fall` | Missing values warn 1147. |
| ✅ | `slew_upper_threshold_pct_rise` / `_fall` | Missing values warn 1148. |
| ✅ | `slew_derate_from_library` | |
| ✅ | `time_unit` | Multiplier 1/10/100 plus k/m/u/n/p/f + `s`. |
| ✅ | `voltage_unit` | Same pattern with `V`. |
| ✅ | `current_unit` | Same pattern with `A`. Default 1 mA. |
| ✅ | `pulling_resistance_unit` | Same pattern with `ohm`. |
| ✅ | `capacitive_load_unit (scale, ff\|pf)` | Other suffixes warn 1154. Missing suffix warns 1156. |
| ✅ | `leakage_power_unit` | Same pattern with `W`. |
| ✅ | `distance_unit` | Same pattern with `m`. Default 1 micron. RM Ch. 1, not UG Ch. 2 TOC. |
| ❌ | `dist_conversion_factor` | RM Ch. 1. |
| ❌ | `power_model` | CCS vs NLDM power selector. Power always uses NLDM tables. |
| ❌ | `is_soi` | Library- or cell-level SOI flag. |
| ❌ | `piece_type` / `piece_define` | Piecewise CMOS breakpoints. |
| ❌ | `routing_layers` | Example in UG Ch. 1 general syntax. |
| ❌ | `define_cell_area` | FPGA / resource area. |
| ❌ | `em_temp_degradation_factor` (library) | |
| ❌ | `default_fpga_isd` / `default_part` | FPGA. |
| ❌ | `default_threshold_voltage_group` | Leakage binning. |
| ✅ | `nom_process` / `nom_voltage` / `nom_temperature` | |
| ✅ | `lu_table_template` | Axes `variable_1`…`variable_3` and `index_1`…`index_3` only. |
| ❌ | `variable_4` / `index_4` on any template | CCSN / 4D tables. |
| ✅ | `output_current_template` | CCS driver. |
| ✅ | `power_lut_template` | |
| ✅ | `ocv_table_template` | |
| ❌ | `compact_lut_template` | Compact CCS / compact CCS power. |
| ❌ | `dc_current_template` | CCSN. |
| ❌ | `em_lut_template` | Electromigration. |
| ❌ | `maxcap_lut_template` / `maxtrans_lut_template` | State-dependent DRC LUTs. |
| ❌ | `pg_current_template` | CCS power. |
| ❌ | `poly_template` | Polynomial delay or noise. |
| ❌ | `iv_lut_template` / `noise_lut_template` / `propagation_lut_template` | NLDM noise. |
| ❌ | `critical_area_lut_template` | Yield. |
| ❌ | `base_curves` | Compact CCS dictionary. |
| ❌ | `char_config` | Characterization. See Ch. 17. |
| ❌ | `input_voltage` / `output_voltage` **library groups** | I/O voltage waveforms, not CCSN `output_voltage_*`. |
| ❌ | `fpga_isd` / `part` | FPGA. |
| ❌ | `timing_range` | Example in UG Ch. 1. |
| ❌ | `sensitization` (library) | Characterization vectors. |

### Chapter 3 — Building environments

#### Library defaults (UG 3-2 plus RM Table 1-1)

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `default_cell_leakage_power` | Only per-cell `cell_leakage_power` is used. Spec default is 0.0 if omitted. |
| ✅ | `default_input_pin_cap` | |
| ✅ | `default_output_pin_cap` | |
| ✅ | `default_inout_pin_cap` | |
| ✅ | `default_max_transition` | Zero warns 1171. |
| ❌ | `default_max_capacitance` | Writer can emit it; reader never calls `setDefaultMaxCapacitance`. |
| ✅ | `default_max_fanout` | Zero warns. |
| ✅ | `default_fanout_load` | Zero warns. |
| ✅ | `default_intrinsic_rise` / `_fall` | Linear model. |
| ✅ | `default_inout_pin_rise_res` / `_fall_res` | |
| ✅ | `default_output_pin_rise_res` / `_fall_res` | |
| ❌ | `default_connection_class` | |
| ✅ | `default_operating_conditions` | Missing named OC warns 1144. |
| ✅ | `default_wire_load` | Missing named load warns 1142. |
| ✅ | `default_wire_load_mode` | `top` / `enclosed` / `segmented`. Unknown warns 1174. |
| ✅ | `default_wire_load_selection` | Missing named selection warns 1143. |
| ❌ | `default_wire_load_area` | |
| ❌ | `default_wire_load_capacitance` | |
| ❌ | `default_wire_load_resistance` | |

#### Operating conditions

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `operating_conditions (name)` | |
| ✅ | `process` / `voltage` / `temperature` | |
| ✅ | `tree_type` | `worst_case` / `best_case` / `balanced`. |
| ❌ | Other opcond attrs (`process_label`, `calc_mode`, `power_rail`, interpolation, …) | |

#### Power supply / wire load

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `power_supply` group | Legacy rail group. |
| ✅ | `wire_load (name)` | Missing name warns 1184. |
| ✅ | `resistance` / `capacitance` / `slope` | |
| ❌ | `wire_load` `area` | |
| ☑️ | `fanout_length (fanout, length, …)` | Only the first two numbers. Extra args (avg cap, stddev, net count) ignored. |
| ❌ | `interconnect_delay` inside `wire_load` | |
| ✅ | `wire_load_selection` / `wire_load_from_area` | Triple `(min_area, max_area, wireload_name)`. |
| ❌ | `wire_load_table` | Independent R/C/area vs fanout. |

#### PVT k-factors

OpenSTA reads `k_{process|volt|temp}_{type}` and rise/fall variants for
these `ScaleFactorType` values only: `pin_cap`, `wire_cap`, `wire_res`,
`min_period`, `cell_{rise|fall}`,
`{hold|setup|recovery|removal|nochange|skew}_{rise|fall}`,
`leakage_power`, `internal_power`, `{rise|fall}_transition`,
`min_pulse_width_{high|low}`.

Named `scaling_factors (name)` groups and cell `scaling_factors : name`
are ✅.

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `k_*_propagation` (`k_process_fall_propagation`, …) | UG 3-10 NLDM list. No `ScaleFactorType` for propagation. |
| ❌ | `k_*_intrinsic_*`, `k_*_slope_*`, `k_*_pin_res` | Linear / piecewise leftovers. |
| ❌ | `k_*_wire_resistance` (that spelling) | OpenSTA uses `k_*_wire_res`. |
| ✅ | `scaled_cell (cell, opcond)` | Ports, functions, and timing must match the unscaled cell (warns 1206–1208). |

### Chapter 4 — Defining core cells

#### Cell attributes named in this chapter

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `cell (name)` | |
| ✅ | `area` | |
| ✅ | `cell_footprint` | |
| ✅ | `dont_use` | |
| ❌ | `dont_touch` / `dont_fault` / `preferred` / `map_only` / `use_for_size_only` | Synthesis. RM Ch. 2. |
| ✅ | `is_macro_cell` | |
| ✅ | `pad_cell` / `is_pad` (cell) | Either sets pad. |
| ❌ | `auxiliary_pad_cell` / `pad_type` / `io_type` | |
| ✅ | `clock_gating_integrated_cell` | `latch_posedge*` / `latch_negedge*` / other (`stringBeginEqual`). |
| ❌ | `is_clock_gating_cell` | Separate RM flag; not read. |
| ✅ | `is_clock_cell` | Stored; not in 2017.06 RM Ch. 2 TOC. |
| ❌ | `contention_condition` | |
| ❌ | `pin_equal` / `pin_opposite` | |
| ❌ | `single_bit_degenerate` | Layout multibit. |
| ❌ | `is_decap_cell` / `is_filler_cell` / `is_tap_cell` | UG 4-51. |
| ❌ | Mux library attrs / `pin_func_type` (`mux_in`, `mux_select`, …) | UG 4-49. |
| ✅ | `user_function_class` | Stored; not in 2017.06 RM Ch. 2 TOC. |
| ✅ | `interface_timing` | |
| ✅ | `is_memory` | Also set if a `memory` subgroup exists; the subgroup body is ❌. |
| ❌ | `timing_model_type` (`abstracted` / `extracted` / `quick`) | Needed for `tlatch`. |
| ✅ | `cell_leakage_power` | |
| ✅ | `always_on` (cell) | Ch. 9. Pin-level `always_on` is ❌. |

The full RM Ch. 2 cell-attribute list is in
[Reference Manual Chapter 2](#reference-manual-chapter-2--cell-and-model).

#### `type` / `bus` / `bundle` / `pin` / `mode_definition`

| Status | Construct | Notes |
| --- | --- | --- |
| ☑️ | `type (name)` | Only `bit_from` and `bit_to`. Missing either warns 1179/1180. |
| ❌ | `type` `downto` / `bit_width` / `data_type` / `base_type` | Writer emits some of these; reader does not need them if from/to exist. |
| ✅ | `bus (name)` + `bus_type` | Library or cell `type`. Unknown `bus_type` warns 1235. |
| ✅ | `pin` inside `bus` | Bits must already exist from the bus declaration. |
| ✅ | `bundle (name)` + `members` | Missing name warns 1313. `members` is required (dereferenced). |
| ✅ | `pin` inside `bundle` | |
| ✅ | `mode_definition` / `mode_value` | `when` and `sdf_cond` on the value. |
| ❌ | `mode_value` `pg_setting` | UG Ch. 9. |

#### Pin attributes named in this chapter

Full RM Ch. 3 pin list:
[Reference Manual Chapter 3](#reference-manual-chapter-3--pin-and-timing).

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `direction` `input` / `output` / `inout` / `internal` | Missing direction is allowed (bus bits). Unknown warns 1240. |
| ✅ | `capacitance` | |
| ✅ | `rise_capacitance` / `fall_capacitance` | Applied to min and max. |
| ✅ | `rise_capacitance_range` / `fall_capacitance_range` | |
| ✅ | `max_capacitance` / `min_capacitance` | DRC. |
| ✅ | `max_transition` / `min_transition` | `max_transition 0` warns 1241. |
| ❌ | `max_cap` / `max_trans` **groups** (state-dependent DRC LUTs) | |
| ✅ | `max_fanout` / `min_fanout` / `fanout_load` | |
| ✅ | `clock` | |
| ✅ | `min_period` (scalar) | |
| ✅ | `min_pulse_width_high` / `min_pulse_width_low` (scalars) | Become width check arcs. |
| ❌ | `min_pulse_width` / `minimum_period` **groups** (`when`, `sdf_cond`, `mode`, `constraint_high`/`low`) | Distinct from `timing_type : min_pulse_width` tables. |
| ✅ | `function` | Size mismatch warns 1195. |
| ✅ | `three_state` | Output becomes tristate; enable is inverted disable. |
| ✅ | `pulse_clock` | `rise_triggered_high_pulse`, `rise_triggered_low_pulse`, `fall_triggered_high_pulse`, `fall_triggered_low_pulse`. Else warn 1242. |
| ✅ | `clock_gate_clock_pin` / `clock_gate_enable_pin` / `clock_gate_out_pin` | |
| ❌ | `clock_gate_test_pin` / `clock_gate_obs_pin` | |
| ☑️ | `signal_type` | Only inside `test_cell`. Values: `test_scan_{enable,enable_inverted,clock,clock_a,clock_b,in,in_inverted,out,out_inverted}`. Else warn 1299. |
| ❌ | `test_output_only` | |
| ❌ | `complementary_pin` | |
| ❌ | `connection_class` | |
| ❌ | `driver_type` | Pad/open-drain/etc. |
| ❌ | `fault_model` / `is_analog` / `pin_func_type` | |
| ❌ | `steady_state_resistance_high` / `_low` / `_above_high` / `_below_low` | Noise. |
| ❌ | `inverted_output` / `internal_node` | Sequential internals. |
| ❌ | `cell_degradation` group | Fanout degradation of delay. |
| ☑️ | `driver_waveform_rise` / `driver_waveform_fall` (pin) | Name lookup into library `normalized_driver_waveform`. Unqualified pin `driver_waveform` is ❌. |
| ❌ | Pin `is_pad` | Cell-level pad flags are ✅; `LibertyPort::setIsPad` exists but is not called. |

### Chapter 5 — Sequential cells

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `ff (IQ, IQN)` / `ff_bank (IQ, IQN, bits)` | Third numeric arg is bus size. If the third param is a string, it is treated as `in, out, out_inv` (size 1). |
| ✅ | `latch (IQ, IQN)` / `latch_bank (IQ, IQN, bits)` | Same parameter rules. |
| ✅ | `clocked_on` / `next_state` (ff) | Width mismatch warns 1196. |
| ❌ | `clocked_on_also` | Dual-clock / scan clock. |
| ❌ | `nextstate_type` | |
| ✅ | `enable` / `data_in` (latch) | |
| ❌ | `data_in_type` | |
| ✅ | `clear` / `preset` | |
| ✅ | `clear_preset_var1` / `clear_preset_var2` | `L` / `H` / `X` only. Other values warn 1282. |
| ❌ | `power_down_function` on ff/latch | |
| ❌ | `reference_pin_names` / `related_inputs` / `related_outputs` / `typical_capacitances` | Macro sequential extras. |
| ☑️ | `lut` group | Creates IQ/IQN ports only. Not an FPGA LUT delay model. |
| ✅ | `statetable (inputs, internals)` + `table` | Rows are `inputs : current : next` comma-separated. Used for sequential identification / equiv / power. |
| ✅ | Statetable input tokens | `L` `H` `-` `L/H` `H/L` `R` `F` `~R` `~F`. |
| ✅ | Statetable internal tokens | `L` `H` `-` `L/H` `H/L` `X` `N`. |
| ✅ | `function` referencing IQ/IQN | |
| ❌ | `state_function` / `internal_node` / `input_map` / `inverted_output` | Statetable output-pin extras. |
| ☑️ | Latch inference (`read_liberty -infer_latches`) | Compensates for missing `latch` groups; not a Liberty construct. |

### Chapter 6 — Test cells

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `test_cell { … }` | Nested cell; ports + ff/latch + functions. Redefinition warns 1262. Used by power activity. |
| ☑️ | `signal_type` on test-cell pins | See Ch. 4. Ignored on the functional cell. |
| ❌ | Extra scan modeling beyond `test_cell` + `signal_type` | LSSD / clocked-scan are ordinary timing if arcs exist. UG 6-17 examples work only to the extent their `timing` groups are NLDM. |

### Chapter 7 — Timing arcs

This is the largest STA-relevant chapter. **NLDM retain tables live
here**, inside a delay `timing` group, not as a separate `timing_type`.

#### Timing group identity

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `timing ()` on pin/bus/bundle | |
| ✅ | `related_pin` | One-to-one or bus aligned. Space-separated names. Output-from warns 1212–1215. Size mismatch warns 1216. |
| ✅ | `related_bus_pins` | Cross product. |
| ☑️ | `related_output_pin` | Pin stored on the arc set. Check delay calc uses that pin’s load as `related_out_cap` when the table axis is `related_out_total_output_net_capacitance`. Length / wire-cap / pin-cap related-out axes are ❌. Gate-delay 3D related-out load is not wired the same way as checks. |
| ✅ | `timing_sense` `positive_unate` / `negative_unate` / `non_unate` | Else inferred from `function`. Unknown warns 1245. |
| ✅ | `when` / `sdf_cond` | |
| ✅ | `sdf_cond_start` / `sdf_cond_end` | |
| ❌ | `when_start` / `when_end` | Conditional checks (UG 7-70). |
| ❌ | `sdf_edges` | |
| ✅ | `mode (mode_name, mode_value)` | Requires two strings (warn 1249). Only the first `mode` attr is used. |
| ❌ | `default_timing` | `sta_disable_cond_default_arcs` is a separate STA heuristic. |
| ❌ | `interdependence_id` | Interdependent setup/hold (UG 7-58). |
| ❌ | `clock_gating_flag` | |
| ❌ | `fpga_arc_condition` | |
| ❌ | `output_signal_level_low` / `_high` on the arc | 2017.06 partial swing. |
| ✅ | `ocv_arc_depth` on the arc | Also library and cell. |
| ⚠️ | Combinational `timing` to an input | Warn 1209. |
| ⚠️ | Missing `related_pin` / `related_bus_pins` | Warn 1243 unless type is min_pulse_width / minimum_period / min/max_clock_tree_path. |
| ⚠️ | Empty table-lookup `timing` group | Warn 1311 if no `cell_*`, `*_transition`, or `*_constraint` tables. |

#### `timing_type` (UG 7-26 / RM 3-103)

Omitted `timing_type` defaults to combinational.

| Status | Value | Analysis |
| --- | --- | --- |
| ✅ | `combinational` / `combinational_rise` / `combinational_fall` | |
| ✅ | `rising_edge` / `falling_edge` | FF/latch clock→Q. |
| ✅ | `preset` / `clear` | |
| ✅ | `setup_rising` / `setup_falling` / `hold_rising` / `hold_falling` | |
| ✅ | `recovery_rising` / `recovery_falling` / `removal_rising` / `removal_falling` | |
| ✅ | `non_seq_setup_rising` / `_falling` / `non_seq_hold_rising` / `_falling` | |
| ✅ | `skew_rising` / `skew_falling` | `report_check_types` max skew. |
| ✅ | `min_pulse_width` / `minimum_period` | As **timing groups** with `rise_constraint` / `fall_constraint`. |
| ✅ | `min_clock_tree_path` / `max_clock_tree_path` | Insertion delay. |
| ✅ | `three_state_disable` / `_rise` / `_fall` / `three_state_enable` / `_rise` / `_fall` | |
| ❌ | `nochange_high_high` / `nochange_high_low` / `nochange_low_high` / `nochange_low_low` | Enum ✅; builder returns `nullptr`. Latch-enable pulse checks (UG 7-63). |
| ❌ | `retaining_time` | Enum ✅; builder returns `nullptr`. Not listed as a 2017.06 UG `timing_type`; retain is modeled with the tables below. |
| ⚠️ | Any other string | Warn 1244; treated as combinational. |

#### NLDM delay / slew / constraint tables (UG 7-38)

Two characterization methods exist. OpenSTA implements Method 1 only.

**Method 1 (supported):** cell delay independent of transition delay —
`cell_rise` / `cell_fall` plus `rise_transition` / `fall_transition`.

**Method 2 (not supported):** total delay as
`rise_propagation`/`fall_propagation` plus transition.

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `cell_rise` / `cell_fall` | Method 1 cell delay. Missing on a gate `timing_type` warns 1211. |
| ✅ | `rise_transition` / `fall_transition` | Missing on a gate `timing_type` warns 1210. |
| ❌ | `rise_propagation` / `fall_propagation` | Method 2. Entirely ignored. `k_*_propagation` is also ❌. |
| ✅ | `rise_constraint` / `fall_constraint` | Checks. |
| ❌ | `retaining_rise` / `retaining_fall` | **NLDM retain delay.** Time the output keeps its old value (0 after a rise / 1 after a fall). Must not exceed parent `cell_*` delay. Typical RAM `tRETAIN` vs `tACC` (UG Figure 7-7). Silently dropped. Parent `cell_rise`/`cell_fall` still used. For `clear`, spec says only fall retain groups are valid; for `preset`, only rise. OpenSTA never reads either. |
| ❌ | `retain_rise_slew` / `retain_fall_slew` | **NLDM retain slew**, separate from parent `*_transition`. Silently dropped. |
| ❌ | `cell_degradation` | |
| ✅ | Linear `intrinsic_rise` / `intrinsic_fall` / `rise_resistance` / `fall_resistance` | `generic_cmos` only. Falls back to library defaults. |
| ❌ | Piecewise `rise_delay_intercept` / `rise_pin_resistance` / `fall_*` (indexed pieces) | `piecewise_cmos`. |
| ❌ | `orders` / `coefs` / `variable_n_range` | Polynomial table body. |

#### Transparent latch clock model (UG 7-86)

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `tlatch (enable_pin)` | |
| ❌ | `edge_type` / `tdisable` | |

OpenSTA infers latch enables from `latch` groups and D→Q / enable arcs,
not from `tlatch`.

#### Driver waveforms (UG 7-88)

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `normalized_driver_waveform` | Axes must be `input_net_transition` × `normalized_voltage` (warn 1265/1225). |
| ✅ | `driver_waveform_name` | Empty name is the library default. |
| ❌ | Cell `driver_waveform` / `driver_waveform_rise` / `driver_waveform_fall` | |
| ☑️ | Pin `driver_waveform_rise` / `_fall` | ✅ if the named waveform exists. Unqualified pin `driver_waveform` ❌. |
| ❌ | `normalized_voltage` as a Liberty **variable** declaration (non-axis) | Axis value ✅. |
| ❌ | `rise_net_delay` / `fall_net_delay` | Library interconnect tables. |

#### Transition degradation (library)

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `rise_transition_degradation` / `fall_transition_degradation` | Axes must be `output_pin_transition` and `connect_delay` (`checkSlewDegradationAxes`). Else warn 1254. |

#### Sensitization (UG 7-94)

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `sensitization` / `pin_names` / `vector` | |
| ❌ | `sensitization_master` / `pin_name_map` | Cell or timing. |
| ❌ | `wave_rise` / `wave_fall` / `wave_rise_sampling_index` / `wave_fall_sampling_index` / `wave_rise_time_interval` / `wave_fall_time_interval` | |

#### PLL attributes (UG 7-103)

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `is_pll_cell` | |
| ❌ | `is_pll_reference_pin` / `is_pll_output_pin` | |
| ✅ | `is_pll_feedback_pin` | Stored only. |

#### Impossible transitions (UG 7-76)

No dedicated construct. Unreachable `when` combinations are ordinary
conditional arcs; OpenSTA does not diagnose mutual exclusion.

### Chapter 8 — Power and electromigration

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `power_lut_template` | |
| ✅ | `internal_power` | |
| ✅ | `related_pin` / `when` / `related_pg_pin` on internal power | Missing related pin still creates a power object. |
| ✅ | `rise_power` / `fall_power` / `power` | `power` copies the same table to rise and fall. |
| ❌ | `equal_or_opposite_output` | |
| ❌ | `falling_together_group` / `rising_together_group` / `switching_together_group` | |
| ❌ | `switching_interval` / `power_level` | |
| ❌ | `mode` on `internal_power` | |
| ✅ | `cell_leakage_power` | |
| ✅ | `leakage_power` + `value` + `when` + `related_pg_pin` | Missing `value` warns 1307. |
| ❌ | `mode` on `leakage_power` | |
| ❌ | `threshold_voltage_group` / `default_threshold_voltage_group` | Leakage binning. |
| ❌ | `em_lut_template` / `electromigration` / `em_max_toggle_rate` / `em_temp_degradation_factor` | |
| ✅ | Clock-gate pin flags used with `report_power` | See Ch. 4. Test/obs pins ❌. |

`report_power` is NLDM static power (internal + switching + leakage).

### Chapter 9 — Advanced low-power modeling

STA does not run UPF. Flags below are stored for queries /
classification unless marked ❌.

#### PG pins (UG 9-3)

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `voltage_map (name, voltage)` | CCS delay calc requires `VDD`. Non-float voltage warns 1166. |
| ✅ | `pg_pin (name)` | Missing name warns 1314. |
| ✅ | `pg_type` | `primary_power` / `primary_ground` / `backup_*` / `internal_*` / `nwell` / `pwell` / `deepnwell` / `deeppwell`. Anything else errors 1291. |
| ✅ | `voltage_name` | |
| ❌ | `physical_connection` / `related_bias_pin` / `user_pg_type` | |
| ❌ | `pg_pin` `is_pad` / `is_insulated` / `tied_to` | |
| ✅ | `related_power_pin` / `related_ground_pin` on signal pins | Unknown port warns 1290. |
| ❌ | `related_pg_pin` on **timing** groups | Used on power groups. |
| ❌ | `output_signal_level` / `input_signal_level` (and `_low`/`_high`) on pins | |
| ❌ | `pg_function` / `std_cell_main_rail` | |
| ❌ | Partial PG / SOI (`is_soi`) / feedthrough `short` | |
| ❌ | `voltage_state_range_list` / `voltage_state_range` | |
| ❌ | `pg_setting_definition` / `pg_setting_value` / `pg_setting_transition` / `default_pg_setting` / `illegal_transition_if_undefined` | |
| ❌ | `power_down_function` on pins | |

#### Level shifter / isolation / switch / retention / always-on / antenna

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `is_level_shifter` | |
| ✅ | `level_shifter_type` `HL` / `LH` / `HL_LH` | Else warn 1228. |
| ✅ | `level_shifter_data_pin` | |
| ❌ | `level_shifter_enable_pin` | |
| ❌ | `input_voltage_range` / `output_voltage_range` / `ground_input_voltage_range` / `ground_output_voltage_range` | |
| ✅ | `is_isolation_cell` | |
| ✅ | `isolation_cell_data_pin` / `isolation_cell_enable_pin` | |
| ❌ | `isolation_enable_condition` / `is_isolated` / `permit_power_down` / `alive_during_*` / clamp functions | |
| ❌ | `is_clock_isolation_cell` / `clock_isolation_cell_clock_pin` | |
| ✅ | `switch_cell_type` `coarse_grain` / `fine_grain` | Else warn 1229. |
| ✅ | `switch_pin` | |
| ❌ | `switch_function` / `power_gating_cell` / `power_gating_pin` / `power_cell_type` | |
| ❌ | `retention_cell` / `retention_pin` / `retention_condition` / `clock_condition` / `preset_condition` / `clear_condition` | |
| ❌ | `save_action` / `restore_action` / `save_condition` / `restore_condition` / `restore_edge_type` | |
| ✅ | `always_on` (cell) | |
| ❌ | `always_on` (pin) | |
| ❌ | Antenna (`antenna_diode_type`, `antenna_diode_related_*`, `has_builtin_pad`) | |
| ❌ | `generated_clock` **Liberty group** | SDC `create_generated_clock` is separate. |
| ❌ | Macro isolation extras / `is_insulated` | UG 9-144. |

### Chapter 10 — Composite current source modeling

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `output_current_template` | `variable_1`/`_2`/`_3`; no `_4`. |
| ☑️ | `output_current_rise` / `output_current_fall` + `vector` | ✅ if each vector has `reference_time` and `index_1`/`index_2` of size 1 (one slew, one cap). Dense 3D current in one vector warns 1223. Missing `reference_time` warns 1224. Compact CCS is ❌. Used only by `ccs_ceff` / `prima`. |
| ☑️ | Timing-level `receiver_capacitance_rise` / `_fall` | Alias: `receiver_capacitance_{rise\|fall}`. |
| ☑️ | Timing-level `receiver_capacitance1_*` / `receiver_capacitance2_*` | Two-segment. `segment` attr can override index. Stored; **not used in delay calc**. Unsupported axes warn 1219. |
| ❌ | Pin-level `receiver_capacitance () { … }` | Conditional / shared receiver C (`when`, `mode`, `char_when`). Common on complex cells. |
| ❌ | `receiver_capacitance_rise_threshold_pct` / `_fall_threshold_pct` | Multisegment thresholds. |
| ❌ | Receiver C beyond two segments except via `segment` | Index is stored; delay calc still unused. |
| ❌ | `ccs_retain_rise` / `ccs_retain_fall` + `vector` | CCS retain currents (UG 10-21). |
| ❌ | `compact_ccs_retain_rise` / `compact_ccs_retain_fall` | Plus `base_curves_group`. |

### Chapter 11 — Advanced CCS

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `base_curves` / `base_curve_type` / `curve_x` / `curve_y` | Compact CCS dictionary. `base_curve_type : ccs_timing_half_curve`. |
| ❌ | `compact_lut_template` | |
| ❌ | `compact_ccs_rise` / `compact_ccs_fall` | How most foundry CCS libs are shipped. `ccs_ceff` then falls back to NLDM. |
| ❌ | `timing_based_variation` / `pin_based_variation` | |
| ❌ | `va_compact_ccs_rise` / `va_compact_ccs_fall` | |
| ❌ | `va_receiver_capacitance1/2_rise/fall` | |
| ❌ | `va_rise_constraint` / `va_fall_constraint` | |
| ❌ | `va_parameters` / `nominal_va_values` / `va_values` | |
| ❌ | `va_compact_ccs_retain_rise` / `va_compact_ccs_retain_fall` | |

### Chapter 12 — Nonlinear signal integrity (NLDM noise)

OpenSTA has no noise/crosstalk engine.

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `iv_lut_template` / steady-state current groups | |
| ❌ | `poly_template` (noise or delay) | |
| ❌ | Steady-state resistance attrs (see Ch. 4) | |
| ❌ | `tied_off` | |
| ❌ | `noise_lut_template` / noise immunity table groups | |
| ❌ | Hyperbolic noise groups | |
| ❌ | `input_noise_width` range limits | Axis enum exists; unused. |
| ❌ | `propagation_lut_template` / propagated noise tables | |
| ☑️ | Axis names `input_noise_width` / `input_noise_height` / `iv_output_voltage` | Recognized as axis types; no noise models. |

### Chapter 13 — CCS noise (CCSN)

A CCSN `.lib` can be read without extra warnings (`test/liberty_ccsn.tcl`).
All CCSN groups are ❌.

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `ccsn_first_stage` / `ccsn_last_stage` | |
| ❌ | `is_needed` / `is_inverting` / `stage_type` | |
| ❌ | `miller_cap_rise` / `miller_cap_fall` | |
| ❌ | `dc_current` / `dc_current_template` | |
| ❌ | `output_voltage_rise` / `output_voltage_fall` | CCSN, not CCS timing. |
| ❌ | `propagated_noise_high` / `propagated_noise_low` | |
| ❌ | `is_unbuffered` / `has_pass_gate` / `is_pass_gate` | |
| ❌ | `input_ccb` / `output_ccb` | Referenced CCB models. |
| ❌ | `related_ccb_node` | |
| ❌ | `propagating_ccb` / `active_input_ccb` / `active_output_ccb` | Timing-level CCB refs. |
| ❌ | `when` / `mode` on CCSN stages | Conditional CCSN. |
| ❌ | `variable_4` on CCSN templates | UG 13-7. |

### Chapter 14 — CCS power

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `leakage_current` / `gate_leakage` / `input_low_value` / `input_high_value` | |
| ❌ | `intrinsic_parasitic` / `total_capacitance` / `lut_values` | |
| ❌ | `dynamic_current` / `switching_group` / `pg_current` | |
| ❌ | `pg_current_template` | |
| ❌ | `compact_ccs_power` | |
| ❌ | `index_output` / `min_input_switching_count` / `max_input_switching_count` | |
| ❌ | `input_switching_condition` / `output_switching_condition` | |

### Chapter 15 — OCV / LVF / POCV

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `ocv_table_template` | Axes include `path_depth` / `path_distance`. |
| ✅ | `ocv_derate` / `ocv_derate_factors` | `rf_type` (`rise` / `fall` / `rise_and_fall`), `derate_type` (`early` / `late` / `early_and_late`), `path_type` (`clock` / `data` / `clock_and_data`). |
| ✅ | `default_ocv_derate_group` / cell `ocv_derate_group` | Missing name warns 1284/1237. |
| ✅ | `ocv_arc_depth` | Library, cell, timing. |
| ❌ | `default_ocv_derate_distance_group` / `ocv_derate_distance_group` | Distance AOCV. |
| ✅ | `ocv_sigma_cell_rise` / `_fall` | `sigma_type` `early` / `late` / `early_and_late` (default both). |
| ✅ | `ocv_sigma_rise_transition` / `ocv_sigma_fall_transition` | |
| ☑️ | `ocv_sigma_rise_constraint` / `ocv_sigma_fall_constraint` | Lookup typo `constraiint`. |
| ❌ | `ocv_sigma_retaining_rise` / `_fall` | |
| ❌ | `ocv_sigma_retain_rise_slew` / `ocv_sigma_retain_fall_slew` | |
| ✅ | `ocv_std_dev_cell_*` / `ocv_mean_shift_cell_*` / `ocv_skewness_cell_*` | Skew-normal SSTA (`sta_pocv_mode`). Skewness is not time-scaled. |
| ✅ | Same three for `*_transition` | |
| ☑️ | Same three for `*_constraint` | Same typo. |
| ❌ | Same three for retaining delay and retain slew | |

SSTA: `sta_pocv_mode` `scalar` / `normal` / `skew_normal`,
`sta_pocv_quantile` (default 3).

### Chapter 16 — I/O pads

| Status | Construct | Notes |
| --- | --- | --- |
| ✅ | `is_pad` / `pad_cell` on the **cell** | Classification. Timing is ordinary NLDM if present. |
| ❌ | Pin `is_pad` | |
| ❌ | `driver_type` | Includes programmable driver functions (UG 16-11). |
| ❌ | `hysteresis` / `drive_current` / `slew_control` | |
| ❌ | `pulling_current` / `pulling_resistance` | |
| ❌ | `multicell_pad_pin` / `has_builtin_pad` / `prefer_tied` / `primary_output` | |
| ❌ | `x_function` / `contention_condition` | I/O conflict (UG 16-21). |

### Chapter 17 — Characterization configuration

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `char_config` (library, cell, pin, or timing) | Entire chapter. |
| ❌ | `driver_waveform` / `driver_waveform_rise` / `_fall` **inside `char_config`** | Distinct from library `normalized_driver_waveform`. |
| ❌ | `input_stimulus_transition` / `input_stimulus_interval` | |
| ❌ | `unrelated_output_net_capacitance` | |
| ❌ | `default_value_selection_method` / `_rise` / `_fall` | |
| ❌ | `merge_tolerance_abs` / `merge_tolerance_rel` / `merge_selection` | |
| ❌ | `ccs_timing_segment_voltage_tolerance_rel` / `ccs_timing_delay_tolerance_rel` / `ccs_timing_voltage_margin_tolerance_rel` | |
| ❌ | `capacitance_voltage_lower_threshold_pct_*` / `capacitance_voltage_upper_threshold_pct_*` | |
| ❌ | `three_state_disable_measurement_method` / `three_state_disable_current_threshold_*` / `three_state_disable_monitor_node` / `three_state_cap_add_to_load_index` | |
| ❌ | `internal_power_calculation` | |

---

## User Guide Volume 2 (2007.03)

Volume 2 did not change after 2007.03. It covers physical libraries and
FPGA architecture data, not CMOS NLDM/CCS STA. All constructs ❌.

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `phys_library` | Entire physical library. |
| ❌ | `topological_design_rules` | |
| ❌ | `contact_min_spacing` / `same_net_min_spacing` | |
| ❌ | `layer_rule` (`wire_width`, `min_spacing`, `wire_extension`) | |
| ❌ | `wire_rule` / `via` / `via_rule_generate` / `routing_layer_formula` | |
| ❌ | `routing_layer` / `device_layer` / `poly_layer` / `cont_layer` | Also RM Ch. 1. |
| ❌ | `resource` / `array` / `tracks` / `track_pattern` | |
| ❌ | `macro` (physical) | |
| ❌ | `critical_area_lut_template` / `critical_area_table` | Yield. |
| ❌ | `part` / `default_part` | FPGA device. |
| ❌ | `fpga_isd` / `default_fpga_isd` | FPGA I/O standard. |
| ❌ | `fpga_cell_type` / `fpga_isd` (cell) / `fpga_condition` / `fpga_arc_condition` | |
| ❌ | FPGA `drive` / `io_type` / `slew` on parts | |
| ❌ | `valid_speed_grade` / `valid_step_levels` / `num_rows` / `num_cols` / `num_luts` / `num_blockrams` / `max_count` | |

`technology (fpga)` does not enable any of the above.

---

## Timing group lookup tables (RM 3-94)

Every group the 2017.06 Reference Manual lists inside a pin `timing`
group. This is the list that must not lose items like retain tables.

| Status | Group | Notes |
| --- | --- | --- |
| ✅ | `cell_rise` / `cell_fall` | |
| ✅ | `rise_transition` / `fall_transition` | |
| ✅ | `rise_constraint` / `fall_constraint` | |
| ❌ | `rise_propagation` / `fall_propagation` | Method 2. |
| ❌ | `retaining_rise` / `retaining_fall` | NLDM retain delay. |
| ❌ | `retain_rise_slew` / `retain_fall_slew` | NLDM retain slew. |
| ❌ | `cell_degradation` | |
| ☑️ | `output_current_rise` / `output_current_fall` | Expanded CCS only; compact unused. |
| ❌ | `ccs_retain_rise` / `ccs_retain_fall` | |
| ❌ | `compact_ccs_rise` / `compact_ccs_fall` | |
| ❌ | `compact_ccs_retain_rise` / `compact_ccs_retain_fall` | |
| ☑️ | `receiver_capacitance_rise` / `_fall` | Parsed; unused in dcalc. |
| ☑️ | `receiver_capacitance1_rise` / `_fall` / `receiver_capacitance2_rise` / `_fall` | Same. |
| ✅ | `ocv_sigma_cell_rise` / `_fall` | |
| ✅ | `ocv_sigma_rise_transition` / `ocv_sigma_fall_transition` | |
| ☑️ | `ocv_sigma_rise_constraint` / `ocv_sigma_fall_constraint` | Typo in reader. |
| ❌ | `ocv_sigma_retaining_rise` / `_fall` | |
| ❌ | `ocv_sigma_retain_rise_slew` / `ocv_sigma_retain_fall_slew` | |
| ✅ | `ocv_std_dev_cell_*` / `ocv_mean_shift_cell_*` / `ocv_skewness_cell_*` | |
| ✅ | Same three for `*_transition` | |
| ☑️ | Same three for `*_constraint` | Typo. |
| ❌ | Same three for retaining delay and retain slew | |
| ❌ | `va_compact_ccs_*` / `va_receiver_capacitance*` / `va_rise_constraint` / `va_fall_constraint` / `va_compact_ccs_retain_*` | Ch. 11. |
| ❌ | `char_config` nested in timing | Ch. 17. |
| ❌ | `tlatch` | Not a delay table; listed next to timing groups in RM. |

---

## Reference Manual Chapter 1 — library

Attributes and groups in RM Ch. 1 that are not already given a row
above, plus a complete default-attribute recap. Status ❌ unless
noted.

### Simple attributes

`comment` ❌, `date` ❌, `revision` ❌,
`current_unit` ✅, `time_unit` ✅, `voltage_unit` ✅,
`leakage_power_unit` ✅, `pulling_resistance_unit` ✅,
`bus_naming_style` ✅, `delay_model` ✅/⚠️,
`distance_unit` ✅, `dist_conversion_factor` ❌,
`em_temp_degradation_factor` ❌,
`input_threshold_pct_{rise,fall}` ✅,
`output_threshold_pct_{rise,fall}` ✅,
`slew_lower_threshold_pct_{rise,fall}` ✅,
`slew_upper_threshold_pct_{rise,fall}` ✅,
`slew_derate_from_library` ✅,
`is_soi` ❌, `power_model` ❌,
`nom_process` ✅, `nom_temperature` ✅, `nom_voltage` ✅,
`default_fpga_isd` ❌, `default_threshold_voltage_group` ❌.

### Defaults (RM Table 1-1)

`default_cell_leakage_power` ❌, `default_connection_class` ❌,
`default_fanout_load` ✅, `default_inout_pin_cap` ✅,
`default_input_pin_cap` ✅, `default_max_capacitance` ❌,
`default_max_fanout` ✅, `default_max_transition` ✅,
`default_operating_conditions` ✅, `default_output_pin_cap` ✅,
`default_wire_load` ✅, `default_wire_load_area` ❌,
`default_wire_load_capacitance` ❌, `default_wire_load_mode` ✅,
`default_wire_load_resistance` ❌, `default_wire_load_selection` ✅.

Linear-model defaults not in Table 1-1 but read:
`default_intrinsic_{rise,fall}` ✅,
`default_inout_pin_{rise,fall}_res` ✅,
`default_output_pin_{rise,fall}_res` ✅.

### Complex attributes

`capacitive_load_unit` ✅, `default_part` ❌, `define` ❌,
`define_cell_area` ❌, `define_group` ❌,
`library_features` ❌,
`receiver_capacitance_{rise,fall}_threshold_pct` ❌,
`technology` ☑️, `voltage_map` ✅.

### Groups

`base_curves` ❌, `compact_lut_template` ❌, `char_config` ❌,
`dc_current_template` ❌, `em_lut_template` ❌,
`fall_net_delay` ❌, `fall_transition_degradation` ✅,
`input_voltage` ❌, `fpga_isd` ❌, `lu_table_template` ✅,
`maxcap_lut_template` ❌, `maxtrans_lut_template` ❌,
`normalized_driver_waveform` ✅, `operating_conditions` ✅,
`output_current_template` ✅, `output_voltage` ❌, `part` ❌,
`pg_current_template` ❌, `power_lut_template` ✅,
`rise_net_delay` ❌, `rise_transition_degradation` ✅,
`sensitization` ❌, `type` ☑️, `user_parameters` ❌,
`voltage_state_range_list` ❌, `wire_load` ✅,
`wire_load_selection` ✅, `wire_load_table` ❌,
`ocv_derate` ✅, `ocv_table_template` ✅,
`critical_area_lut_template` ❌,
`device_layer` / `poly_layer` / `routing_layer` / `cont_layer` ❌.

---

## Reference Manual Chapter 2 — cell and model

### Simple attributes (RM 2-4)

| Status | Attribute |
| --- | --- |
| ✅ | `always_on` (cell) |
| ❌ | `antenna_diode_type` |
| ✅ | `area` |
| ❌ | `auxiliary_pad_cell` |
| ❌ | `base_name` |
| ❌ | `bus_naming_style` (cell override) |
| ✅ | `cell_footprint` |
| ✅ | `cell_leakage_power` |
| ✅ | `clock_gating_integrated_cell` |
| ❌ | `contention_condition` |
| ❌ | `dont_fault` |
| ❌ | `dont_touch` |
| ✅ | `dont_use` |
| ❌ | `driver_type` (cell) |
| ❌ | `driver_waveform` / `driver_waveform_rise` / `driver_waveform_fall` (cell) |
| ❌ | `em_temp_degradation_factor` |
| ❌ | `fpga_cell_type` / `fpga_isd` |
| ✅ | `interface_timing` |
| ❌ | `io_type` |
| ✅ | `is_pad` (cell) |
| ❌ | `is_pll_cell` |
| ❌ | `is_clock_gating_cell` |
| ❌ | `is_clock_isolation_cell` |
| ✅ | `is_isolation_cell` |
| ✅ | `is_level_shifter` |
| ✅ | `is_macro_cell` |
| ❌ | `is_soi` |
| ✅ | `level_shifter_type` |
| ❌ | `map_only` |
| ✅ | `pad_cell` |
| ❌ | `pad_type` |
| ❌ | `power_cell_type` / `power_gating_cell` |
| ❌ | `preferred` |
| ❌ | `retention_cell` |
| ❌ | `sensitization_master` |
| ❌ | `single_bit_degenerate` |
| ❌ | `slew_type` |
| ✅ | `switch_cell_type` |
| ❌ | `threshold_voltage_group` |
| ❌ | `timing_model_type` |
| ❌ | `use_for_size_only` |
| ✅ | `is_clock_cell` (read; not in 2017.06 RM TOC) |
| ✅ | `is_memory` (read; not in 2017.06 RM TOC) |
| ✅ | `user_function_class` (read; not in 2017.06 RM TOC) |
| ✅ | `ocv_derate_group` / `ocv_arc_depth` / `scaling_factors` |

### Complex attributes (RM 2-25)

`input_voltage_range` ❌, `output_voltage_range` ❌,
`pin_equal` ❌, `pin_name_map` ❌, `pin_opposite` ❌,
`resource_usage` ❌.

### Groups (RM 2-28)

`bundle` ✅, `bus` ✅, `char_config` ❌,
`clear_condition` ❌, `clock_condition` ❌,
`critical_area_table` ❌, `dynamic_current` ❌,
`ff` / `ff_bank` / `latch` / `latch_bank` ✅,
`fpga_condition` ❌, `generated_clock` ❌,
`intrinsic_parasitic` ❌, `total_capacitance` ❌,
`leakage_current` / `gate_leakage` ❌, `leakage_power` ✅,
`lut` ☑️, `mode_definition` ✅,
`pg_setting_definition` ❌, `pg_pin` ✅,
`preset_condition` ❌, `retention_condition` ❌,
`statetable` ✅, `test_cell` ✅, `type` ☑️,
`compact_ccs_power` / `pg_current` / `switching_group` ❌,
`memory` ☑️ (presence sets `is_memory`; body ❌).

### `model` group (RM 2-125)

Entire `model` group ❌ (`cell_name`, `short`).

---

## Reference Manual Chapter 3 — pin and timing

### Pin simple attributes (RM 3-2)

| Status | Attribute |
| --- | --- |
| ❌ | `alive_during_power_up` / `alive_during_partial_power_down` |
| ❌ | `always_on` (pin) |
| ❌ | `antenna_diode_related_ground_pins` / `_power_pins` / `antenna_diode_type` |
| ❌ | `bit_width` |
| ✅ | `capacitance` |
| ❌ | `clamp_0_function` / `clamp_1_function` / `clamp_z_function` / `clamp_latch_function` / `illegal_clamp_condition` |
| ✅ | `clock` |
| ✅ | `clock_gate_clock_pin` / `clock_gate_enable_pin` / `clock_gate_out_pin` |
| ❌ | `clock_gate_test_pin` / `clock_gate_obs_pin` |
| ❌ | `clock_isolation_cell_clock_pin` |
| ❌ | `complementary_pin` / `connection_class` / `data_in_type` |
| ✅ | `direction` |
| ❌ | `dont_fault` / `drive_current` / `driver_type` |
| ❌ | `driver_waveform` (unqualified) |
| ☑️ | `driver_waveform_rise` / `driver_waveform_fall` |
| ✅ | `fall_capacitance` / `rise_capacitance` |
| ❌ | `fall_current_slope_after_threshold` / `_before_threshold` / `fall_time_after_threshold` / `_before_threshold` (and rise twins) |
| ✅ | `fanout_load` |
| ❌ | `fault_model` |
| ✅ | `function` |
| ❌ | `has_builtin_pad` / `has_pass_gate` / `hysteresis` |
| ❌ | `input_map` / `input_signal_level` |
| ❌ | `input_threshold_pct_*` **on a pin** (library-level ✅) |
| ❌ | `input_voltage` (pin) / `internal_node` / `inverted_output` / `is_analog` |
| ❌ | `is_pad` (pin) |
| ✅ | `is_pll_feedback_pin` |
| ❌ | `is_pll_reference_pin` / `is_pll_output_pin` / `is_unbuffered` |
| ✅ | `isolation_cell_data_pin` / `isolation_cell_enable_pin` |
| ❌ | `permit_power_down` / `is_isolated` / `isolation_enable_condition` |
| ✅ | `level_shifter_data_pin` |
| ❌ | `level_shifter_enable_pin` / `map_to_logic` |
| ✅ | `max_capacitance` / `max_fanout` / `max_transition` |
| ✅ | `min_capacitance` / `min_fanout` / `min_period` |
| ✅ | `min_pulse_width_high` / `min_pulse_width_low` (scalars) |
| ❌ | `multicell_pad_pin` / `nextstate_type` |
| ❌ | `output_signal_level` / `_high` / `_low` / `output_voltage` |
| ❌ | `pg_function` / `pin_func_type` / `power_down_function` |
| ❌ | `prefer_tied` / `primary_output` / `pulling_current` / `pulling_resistance` |
| ✅ | `pulse_clock` |
| ✅ | `related_ground_pin` / `related_power_pin` |
| ❌ | `restore_action` / `restore_condition` / `restore_edge_type` |
| ❌ | `save_action` / `save_condition` |
| ☑️ | `signal_type` (test_cell only) |
| ❌ | `slew_control` |
| ❌ | `slew_lower/upper_threshold_pct_*` on a pin |
| ❌ | `state_function` / `std_cell_main_rail` / `switch_function` |
| ✅ | `switch_pin` |
| ❌ | `test_output_only` |
| ✅ | `three_state` |
| ❌ | `x_function` |

### Pin complex attributes

`fall_capacitance_range` ✅, `rise_capacitance_range` ✅,
`power_gating_pin` ❌, `retention_pin` ❌.

### Pin groups other than `timing`

`ccsn_first_stage` / `ccsn_last_stage` ❌, `char_config` ❌,
`electromigration` / `em_max_toggle_rate` ❌,
`input_ccb` / `output_ccb` ❌, `internal_power` ✅,
`max_cap` / `max_trans` ❌, `min_pulse_width` **group** ❌,
`minimum_period` **group** ❌, pin-level `receiver_capacitance` ❌.

### Timing simple / complex attributes (RM 3-93)

Already listed in [Chapter 7](#chapter-7--timing-arcs). Additional RM
entries:

| Status | Construct | Notes |
| --- | --- | --- |
| ❌ | `function` **complex** attribute on a timing group | Distinct from pin `function`. |
| ❌ | `reference_input` | |
| ❌ | `related_pg_pin` on timing | |

Lookup tables: [Timing group lookup tables](#timing-group-lookup-tables-rm-394).

---

## Cross-cutting matrices

### Delay calculators vs Liberty models

| Calculator | Liberty data used |
| --- | --- |
| `unit` | None (delay = 1). |
| `lumped_cap` | NLDM `cell_*` / `*_transition` vs Cload. |
| `dmp_ceff_elmore` (default) | NLDM tables + SPEF pi model. CCS waveforms unused. |
| `dmp_ceff_two_pole` | Same NLDM + two-pole interconnect. |
| `arnoldi` | NLDM + Arnoldi interconnect. |
| `ccs_ceff` | Expanded `output_current_*` vectors if present and in-bounds; else NLDM. Receiver C unused. Compact CCS unused. |
| `prima` | Same CCS currents if present; else NLDM. |

### Table axis variables

Recognized (`stringTableAxisVariable`):

| Axis | Used by |
| --- | --- |
| `total_output_net_capacitance` | NLDM delay (✅), receiver C (☑️), CCS current (☑️) |
| `input_net_transition` / `input_transition_time` | NLDM delay (✅) |
| `related_pin_transition` / `constrained_pin_transition` | Checks (✅) |
| `related_out_total_output_net_capacitance` | Check delay (✅); allowed on gate axes (☑️ for cell delay) |
| `output_pin_transition` / `connect_delay` | Slew degradation (✅) |
| `time` | CCS current vectors (☑️) |
| `normalized_voltage` | Driver waveforms (✅) |
| `path_depth` / `path_distance` | AOCV templates (✅) |
| `equal_or_opposite_output_net_capacitance` | Recognized; not in `GateTableModel::checkAxis` (❌ for delay) |
| `iv_output_voltage` / `input_noise_width` / `input_noise_height` | Noise only (❌) |
| `input_voltage` / `output_voltage` | CCSN / I-V (❌) |

❌ (unknown axis, warn 1297): `output_net_length`, `output_net_wire_cap`,
`output_net_pin_cap`, `related_out_output_net_length`,
`related_out_output_net_wire_cap`, `related_out_output_net_pin_cap`,
`frequency`, `temperature`, `voltage`, `ccs_receiver_voltage*`, and any
other spec variable not in the table.

`GateTableModel::checkAxis` allows only Cload, input transition, and
`related_out_total_output_net_capacitance`. Unsupported axes warn 1251
but the table object is still returned.

`CheckTableModel::checkAxis` allows `constrained_pin_transition`,
`related_pin_transition`, and `related_out_total_output_net_capacitance`
only. Check tables indexed by `input_net_transition` or
`total_output_net_capacitance` warn 1251.

`ReceiverModel::checkAxes` allows 1D `input_net_transition` or 2D
transition × Cload (either axis order).

Unknown template name warns 1253. Missing `values` warns 1257.
Non-monotonic indexes warn 1173/1178.

### `pg_type` values

**Y:** `primary_power`, `primary_ground`, `backup_power`, `backup_ground`,
`internal_power`, `internal_ground`, `nwell`, `pwell`, `deepnwell`,
`deeppwell`. Anything else errors (1291).

---

## What a typical digital STA `.lib` needs

The following subset is the practical “supported Liberty” for OpenSTA
NLDM STA + optional expanded CCS + LVF delay/slew + NLDM power:

- `library` units, thresholds, `delay_model : table_lookup`
- `lu_table_template` (≤3 axes: transition × capacitance)
- `operating_conditions`, k-factors, optional `wire_load*`
- `cell`, `pin`/`bus`/`bundle`, `function`, `ff`/`latch`
- `timing` with `related_pin`, `timing_type`, `when`/`sdf_cond`/`mode`
- `cell_rise`/`cell_fall`, `rise_transition`/`fall_transition`
- `rise_constraint`/`fall_constraint` for checks
- Optional `pg_pin` + `voltage_map (VDD, …)`
- Optional `ocv_sigma_cell_*` / `ocv_sigma_*_transition` and moment tables
  (not constraint groups, until the `constraiint` typo is fixed)
- Optional expanded `output_current_*` vectors **and**
  `set_delay_calculator ccs_ceff`
- Optional `internal_power` / `leakage_power` for `report_power`

Everything else in 2017.06 is ❌, ⚠️, or ☑️ as tabulated above.

Foundry CCS libraries that ship **compact CCS** (`base_curves` +
`compact_ccs_rise`/`fall`) are treated as NLDM unless they also contain
expanded `output_current_*` vectors.

Memory libraries that rely on `retaining_rise` / `retaining_fall` /
`retain_rise_slew` / `retain_fall_slew` for `tRETAIN` get access delay
only; retain is dropped with no warning.

## Maintenance

When adding Liberty support, update the matching row in this file in the
same change as `liberty/LibertyReader.cc` (or the delay/power consumer).
The previous miss of `retaining_rise` / `retaining_fall` /
`retain_rise_slew` / `retain_fall_slew` is why every RM timing subgroup
is listed explicitly in Chapter 7 and in
[Timing group lookup tables](#timing-group-lookup-tables-rm-394).
