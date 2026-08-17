# SDC 2.1 compliance

This is a chapter-by-chapter checklist of
*Using the Synopsys Design Constraints Format, Application Note,
Version 2.1, December 2017* (`sdc2.1.pdf`) against OpenSTA
(`sdc/Sdc.tcl`, `sdc/Sdc.cc`, `sdc/WriteSdc.cc`, and related search /
delay-calculation code).

The application note contains:

- Preface (SDC 2.1 deltas)
- Chapter 1 — Using the Synopsys Design Constraints Format
- Appendix A — SDC Syntax (the normative command/argument list)

OpenSTA is a gate-level STA. Area, power-optimization, voltage-area,
and level-shifter commands are listed even when they are out of STA
scope, so nothing is omitted by silence.

OpenSTA does not implement an SDC version switch. `set sdc_version` is
ordinary Tcl and `read_sdc` has no `-version`. Version pinning is N/A:
it only selects which commands/options a strict parser accepts, and
does not change STA semantics. Commands are accepted as documented
below regardless of any `set sdc_version` assignment in the file.

## How to read the tables

| Mark | Meaning |
| --- | --- |
| ✅ | Parsed and used for timing, DRC reporting, object access, or written by `write_sdc`. |
| ☑️ | Partial: accepted but incomplete, unused at analysis time, or missing listed options. |
| ❌ | Not implemented, and it is in STA scope. |
| ⚠️ | Explicitly warned or rejected (`sta_warn` / `sta_error`). |
| N/A | Not applicable. Version pinning, synthesis, or other non-STA interchange. |

Options that OpenSTA adds beyond SDC 2.1 are called out as *extra*, not
as compliance gaps.

## Runtime notes

- `read_sdc` evaluates the file as Tcl (`include`). Gzip-compressed
  files are uncompressed automatically. `-echo` and `-mode` are extra
  (MCMM). `-version` is N/A (see Preface).
- `write_sdc` emits SDC. `-gzip`, `-map_hpins`, `-digits`, `-mode`, and
  `-no_timestamp` are extra.
- Units: `set_units` **checks** that command units match the first
  Liberty library; it does not change them. Use `set_cmd_units`.
- Hierarchy: `set_hierarchy_separator` accepts `/ @ ^ # . |`. Object
  access uses the current instance (`current_instance`).
- `-comment` is stored and written for `create_clock`,
  `create_generated_clock`, `group_path`, `set_clock_groups`,
  `set_false_path`, `set_max_delay`, `set_min_delay`, and
  `set_multicycle_path` (and extra `set_path_margin`).
- Interactive `unset_*` / `delete_*` commands are not in SDC 2.1.
  They are extra OpenSTA commands and are not tabulated as gaps.

---

## Preface

### What’s New in This Release

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | `set_clock_sense` removed from SDC 2.1 | Still accepted; warns 415 and calls `set_sense -type clock`. |
| N/A | `read_sdc -version 2.0` to keep old scripts | Version pinning is for strict SDC parsers. OpenSTA accepts `set_clock_sense` with warn 415. |
| ☑️ | `set_sense` | Clock sense ✅. `-type data`, `-pulse`, `-non_unate`, `-clock_leaf` ❌/⚠️. |
| ❌ | `set_clock_latency -dynamic` | Not parsed. |
| ❌ | `set_timing_derate -static` / `-dynamic` / `-increment` | Not parsed. SI/dynamic derate is out of scope. |
| ✅ | `set_max_delay -ignore_clock_latency` | Also extra `-probe` / `-reset_path`. |
| ✅ | `set_min_delay -ignore_clock_latency` | Same extras. |
| ⚠️ | `set_driving_cell -multiply_by` removed in 2.1 | Still parsed; warns 458 and ignores. |

### About This Application Note

Methodology text only. No commands.

---

## Chapter 1 — Using the Synopsys Design Constraints Format

### About the SDC Format

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | SDC is Tcl | Commands are Tcl procs. |
| ✅ | Design must be loaded/linked first | `read_verilog` / `read_liberty` / `link_design` are outside SDC. |
| ✅ | `read_sdc` / `write_sdc` interchange | OpenSTA names; DC uses the same pair. |

### Specifying the SDC Version

| Mark | Feature | Notes |
| --- | --- | --- |
| N/A | `set sdc_version value` | Ordinary Tcl `set`. OpenSTA does not branch on it; STA semantics do not change by version. |

### Specifying Units

| Mark | Feature | Notes |
| --- | --- | --- |
| ☑️ | `set_units` | Validates scales against the library (`sta_warn` 345 on mismatch). Does not set units. Extra `-distance`. See Appendix A. |

### Specifying Design Constraints

Table 1-1 command groups. Per-command status is in Appendix A.
Summary of the table:

| Mark | Group | Notes |
| --- | --- | --- |
| ✅ | Operating conditions | `set_operating_conditions` (no `-object_list`). |
| ☑️ | Wire load models | Model ✅. Mode stored, unused in delay calc. Selection stored, unused. Min block size ⚠️. |
| ☑️ | System interface | Drive / driving cell / input transition / load / port fanout ✅. `set_fanout_load` ⚠️. |
| ✅ | Design rule constraints | Capacitance, fanout, and transition limits are stored and reported by `report_check_types`. They do not change delay calculation. |
| ✅ | Timing constraints | Core STA. Ideal-network commands ❌. |
| ✅ | Timing exceptions | Plus extra `set_path_margin`. |
| ☑️ | Area | `set_max_area` stored and written; unused in analysis. |
| N/A | Multivoltage / power optimization | Floorplan, level-shifter insertion, and power budgets. See A-5. |
| ✅ | Logic assignments | `set_case_analysis` propagates. `set_logic_*` constants do not propagate through gates. |

### Specifying Design Objects

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | Implicit names | Command-specific object lookup. |
| ✅ | Explicit `get_*` / `all_*` | See Appendix A. Extra `-quiet` / `-filter` on most `get_*`. |
| ✅ | Table 1-2 object types | `design`, `clock`, `port`, `cell`, `pin`, `net`, `library`, `lib_cell`, `lib_pin`, `register`. |

### Specifying Multiple Objects

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | Tcl lists | |
| ✅ | `?` one character, `*` any string | `PatternMatch`. |
| ✅ | Omitted pattern means `*` | `get_cells` / `get_pins` / `get_nets` / `get_ports` / `get_clocks` / `get_libs`. |

### Specifying Hierarchical Objects

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | Current instance as name origin | `current_instance`; `find_instances_matching` uses it. |
| ✅ | Separators `/ @ ^ # . \|` | `set_hierarchy_separator` and `-hsc`. |
| ✅ | `-hsc` on object access | `get_cells`, `get_lib_cells`, `get_lib_pins`, `get_nets`, `get_pins`. SDC 2.1 does not list `-hsc` on `get_cells` / `get_lib_pins` (extra there). |

### Specifying Buses

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | Verilog-style `name[index]` in braces | Liberty `bus_naming_style` (typically `%s[%d]`). |

### Adding Comments

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | `#` line comments | Tcl. |
| ✅ | `; #` trailing comments | Tcl. |

### Using the `-comment` Option

| Mark | Command | Notes |
| --- | --- | --- |
| ✅ | `create_clock` | Written by `write_sdc`. |
| ✅ | `create_generated_clock` | |
| ✅ | `group_path` | |
| ✅ | `set_clock_groups` | |
| ✅ | `set_false_path` | |
| ✅ | `set_max_delay` | |
| ✅ | `set_min_delay` | |
| ✅ | `set_multicycle_path` | |

### Managing Large SDC Files

| Mark | Feature | Notes |
| --- | --- | --- |
| ✅ | gzip compressed SDC | `read_sdc` auto-detects. `write_sdc -gzip` (covers Example 1-1). |

---

## Appendix A — SDC Syntax

### General-Purpose Commands (Table A-1)

| Mark | Command / arguments | Notes |
| --- | --- | --- |
| ✅ | `current_instance [instance]` | Sets relative name lookup. Help text saying the instance argument is unsupported is stale. |
| ✅ | `expr arg1 …` | Tcl built-in. |
| ✅ | `list arg1 …` | Tcl built-in. |
| ✅ | `set variable_name value` | Tcl built-in. |
| ✅ | `set_hierarchy_separator separator` | Must be one of `/@^#. \|`. |
| ☑️ | `set_units` `-capacitance` `-resistance` `-time` `-voltage` `-current` `-power` | Check only. Extra `-distance`. |

`current_design` is in Table A-2 (object access), not A-1.

### Object Access Commands (Table A-2)

| Mark | Command | SDC arguments | Notes |
| --- | --- | --- | --- |
| ✅ | `all_clocks` | (none) | |
| ☑️ | `all_inputs` | `-level_sensitive` `-edge_triggered` `-clock` | Those three ❌. Extra `-no_clocks` ✅. |
| ☑️ | `all_outputs` | `-level_sensitive` `-edge_triggered` `-clock` | Those three ❌. |
| ☑️ | `all_registers` | `-no_hierarchy` `-hsc` `-clock` `-rise_clock` `-fall_clock` `-cells` `-data_pins` `-clock_pins` `-slave_clock_pins` `-async_pins` `-output_pins` `-level_sensitive` `-edge_triggered` `-master_slave` | Implemented: `-clock` `-rise_clock` `-fall_clock` `-cells` `-data_pins` `-clock_pins` `-async_pins` `-output_pins` `-level_sensitive` `-edge_triggered`. Missing: `-no_hierarchy` `-hsc` `-slave_clock_pins` `-master_slave`. |
| ☑️ | `current_design [design]` | | Reports/sets the top cell only. Other designs warn 347. |
| ✅ | `get_cells` | `-hierarchical` `-regexp` `-nocase` `-of_objects` `patterns` | Extra `-hsc` `-filter` `-quiet`. `-of_objects` with a patterns argument warns 348. |
| ✅ | `get_clocks` | `-regexp` `-nocase` `patterns` | Extra `-quiet` `-filter`. |
| ✅ | `get_lib_cells` | `-regexp` `-hsc` `-nocase` `patterns` | Extra `-of_objects` `-quiet` `-filter`. |
| ✅ | `get_lib_pins` | `-regexp` `-nocase` `patterns` | Extra `-hsc` `-of_objects` `-quiet` `-filter`. |
| ✅ | `get_libs` | `-regexp` `-nocase` `patterns` | Extra `-quiet` `-filter`. |
| ✅ | `get_nets` | `-hierarchical` `-hsc` `-regexp` `-nocase` `-of_objects` `patterns` | Extra `-quiet` `-filter`. |
| ✅ | `get_pins` | `-hierarchical` `-hsc` `-regexp` `-nocase` `-of_objects` `patterns` | Extra `-quiet` `-filter`. |
| ✅ | `get_ports` | `-regexp` `-nocase` `patterns` | Extra `-of_objects` `-quiet` `-filter`. |

`-nocase` without `-regexp` warns 358 on `get_lib_pins` (and similarly where checked).

### Timing Constraints (Table A-3)

#### `create_clock`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-period` | Required in practice. |
| ✅ | `-name` | Defaults to first pin name. |
| ✅ | `-waveform` | Rise/fall edge list. Default 0 / period/2. |
| ✅ | `-add` | Multiple clocks on the same pin. |
| ✅ | `-comment` | |
| ✅ | `source_objects` | Omitted → virtual clock. |

#### `create_generated_clock`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-name` | |
| ✅ | `-source` | Master pin. |
| ✅ | `-edges` | |
| ✅ | `-divide_by` | |
| ✅ | `-multiply_by` | |
| ✅ | `-duty_cycle` | |
| ✅ | `-invert` | |
| ✅ | `-edge_shift` | |
| ✅ | `-add` | |
| ✅ | `-master_clock` | |
| ✅ | `-combinational` | Implies divide-by 1. |
| ✅ | `-comment` | |
| ✅ | `source_objects` | |

#### `group_path`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-name` / `-default` | Mutually exclusive. Used by `report_checks`. |
| ☑️ | `-weight` | Parsed, ignored. Synthesis cost, not STA. Extra `-critical_range` also ignored. |
| ✅ | `-from` `-rise_from` `-fall_from` | |
| ✅ | `-to` `-rise_to` `-fall_to` | |
| ✅ | `-through` `-rise_through` `-fall_through` | Repeatable. |
| ✅ | `-comment` | |

#### `set_clock_gating_check`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-setup` `-hold` `-rise` `-fall` `-high` `-low` | Global if `object_list` omitted. |
| ✅ | `object_list` | Pins / instances / clocks. Gated by `sta_gated_clock_checks_enabled`. |

#### `set_clock_groups`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-group` | Repeatable. |
| ✅ | `-logically_exclusive` `-physically_exclusive` `-asynchronous` | One required. |
| ✅ | `-allow_paths` | Do not false-path between groups. |
| ✅ | `-name` `-comment` | |

#### `set_clock_latency`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-rise` `-fall` `-min` `-max` | |
| ✅ | `-source` | Insertion delay. |
| ❌ | `-dynamic` | SDC 2.1 addition; not parsed. |
| ✅ | `-late` `-early` | |
| ✅ | `-clock` | Ignored (warn 408) when the object is a clock. |
| ✅ | `delay` `object_list` | Clocks, pins, ports. Clears propagated-clock on those objects. |

#### `set_sense`

| Mark | Argument | Notes |
| --- | --- | --- |
| ☑️ | `-type clock \| data` | `clock` ✅. `data` warns 413. |
| ❌ | `-non_unate` | Not parsed. |
| ✅ | `-positive` `-negative` | Unate sense. |
| ❌ | `-clock_leaf` | Not parsed. |
| ✅ | `-stop_propagation` | |
| ⚠️ | `-pulse pulse_type` | Warns 416; pulse clocks via this command are not supported. Liberty `pulse_clock` on pins is still propagated. |
| ✅ | `-clocks` | Also accepts `-clock` (non-SDC alias). |
| ✅ | `pin_list` | Hierarchical pins error 418. |

#### `set_clock_transition`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-rise` `-fall` `-min` `-max` `transition` `clock_list` | Ideal clocks. Virtual clocks warn 419. |

#### `set_clock_uncertainty`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | Inter-clock `-from`/`-rise_from`/`-fall_from` `-to`/`-rise_to`/`-fall_to` | Must be paired. |
| ✅ | `-rise` `-fall` | Inter-clock only; error 422 on single-clock form. |
| ✅ | `-setup` `-hold` | |
| ✅ | `uncertainty` `[object_list]` | Clocks, ports, pins. |

#### `set_data_check`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-from` `-to` and rise/fall variants | |
| ✅ | `-setup` `-hold` `-clock` `value` | |

#### `set_disable_timing`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-from` `-to` `cell_pin_list` | Cells, instances, ports, pins, lib pins. Hierarchical instance `-from`/`-to` errors 430. |

#### `set_false_path`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-setup` `-hold` `-rise` `-fall` | |
| ✅ | `-from`/`-to`/`-through` and rise/fall variants | At least one required. |
| ✅ | `-comment` | Extra `-reset_path`. |

#### `set_ideal_latency` / `set_ideal_network` / `set_ideal_transition`

| Mark | Command | Notes |
| --- | --- | --- |
| ❌ | `set_ideal_latency` | Parsed, discarded. |
| ❌ | `set_ideal_network [-no_propagate]` | SDC spelling `-no_propagate`; OpenSTA accepts `-no_propagation`. Discarded. |
| ❌ | `set_ideal_transition` | Parsed, discarded. |

Ideal clocks still use `set_clock_latency` / `set_clock_transition` /
`set_propagated_clock`.

#### `set_input_delay` / `set_output_delay`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-clock` `-reference_pin` `-clock_fall` | `-source_latency_included` / `-network_latency_included` ignored with `-reference_pin` (warn 438/439). |
| ❌ | `-level_sensitive` | Not parsed. |
| ✅ | `-rise` `-fall` `-max` `-min` `-add_delay` | |
| ✅ | `-network_latency_included` `-source_latency_included` | |
| ✅ | `delay_value` `port_pin_list` | Direction-checked. Same-pin clock source warns 441. |

#### `set_max_delay` / `set_min_delay`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | Path spec, `-rise` `-fall` | |
| ✅ | `-ignore_clock_latency` | SDC 2.1. |
| ✅ | `-comment` | Extra `-reset_path` `-probe`. |
| ✅ | `delay_value` | |

#### `set_max_time_borrow`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `delay_value` `object_list` | Clocks, instances, pins. Used in latch borrowing (`search/Latches.cc`). |

#### `set_min_pulse_width`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-low` `-high` `value` `[object_list]` | Global if objects omitted. Reported by `report_check_types -min_pulse_width`. |

#### `set_multicycle_path`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-setup` `-hold` `-rise` `-fall` `-start` `-end` | Default: setup uses end clock, hold uses start clock. |
| ✅ | Path spec `-comment` `path_multiplier` | Extra `-reset_path`. Hold multiplier is 0 if neither `-setup` nor `-hold` is given. |

#### `set_propagated_clock`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `object_list` | Clocks, ports, pins. Virtual clocks warn 450. Extra `sta_propagate_all_clocks`. |

#### `set_resistance`

Listed under timing in Table 1-1; syntax is in Table A-4.

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-min` `-max` `value` `net_list` | Net resistance annotation. |

### Environment Commands (Table A-4)

#### `set_case_analysis`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `value` `port_or_pin_list` | `0` `1` `zero` `one` `rise` `rising` `fall` `falling`. Constants propagate. Also drives Liberty mode groups. |

#### `set_drive`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-rise` `-fall` `-min` `-max` `resistance` `port_list` | Input port drive resistance. |

#### `set_driving_cell`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-lib_cell` `-library` `-pin` `-from_pin` | Extra alias `-cell`. |
| ✅ | `-rise` `-fall` `-min` `-max` | |
| ⚠️ | `-dont_scale` `-no_design_rule` `-multiply_by` | Warn 458–460, ignored. `-multiply_by` is not SDC 2.1. |
| ❌ | `-clock` `-clock_fall` | Listed in SDC 2.1; not parsed. |
| ✅ | `-input_transition_rise` `-input_transition_fall` | |
| ✅ | `port_list` | |

#### `set_fanout_load`

| Mark | Command | Notes |
| --- | --- | --- |
| ⚠️ | `set_fanout_load value port_list` | Warns 461; not stored. |

#### `set_input_transition`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-rise` `-fall` `-min` `-max` `transition` `port_list` | |
| ⚠️ | `-clock` `-clock_fall` | Parsed; warn 462/463 not supported. |

#### `set_load`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-min` `-max` `-subtract_pin_load` `-pin_load` `-wire_load` | Port pin/wire cap vs net wire cap. Extra `-rise` `-fall` on ports. |
| ✅ | `value` `objects` | Nets override SPEF for delay calc. |

#### `set_logic_dc` / `set_logic_one` / `set_logic_zero`

| Mark | Command | Notes |
| --- | --- | --- |
| ✅ | `port_list` | Also accepts pins (extra vs port-only SDC wording). Constants do **not** propagate through gates (`set_case_analysis` does). |

#### `set_max_area`

| Mark | Command | Notes |
| --- | --- | --- |
| ☑️ | `area_value` | Stored and written; unused in analysis. |

#### `set_max_capacitance` / `set_min_capacitance`

| Mark | Command | Notes |
| --- | --- | --- |
| ✅ | `value` `object_list` | Ports, cells, pins. DRC via `report_check_types`. Not used in delay calculation. Help text saying they are ignored during timing means path delay, not DRC. |

#### `set_max_fanout`

| Mark | Command | Notes |
| --- | --- | --- |
| ✅ | `value` `object_list` | Ports (must be input/bidirect) and cells. DRC via `report_check_types`. |

#### `set_max_transition`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-clock_path` `-data_path` `-rise` `-fall` `value` `object_list` | Rise/fall and path-type flags apply to clocks only (warn 468 otherwise). `report_check_types -max_slew`. |

#### `set_operating_conditions`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-analysis_type single\|bc_wc\|on_chip_variation` | Default OCV. |
| ✅ | `-library` `[condition]` `-min` `-max` `-min_library` `-max_library` | |
| ❌ | `-object_list` | Listed in SDC 2.1; not parsed. Instance PVT uses extra `set_pvt`. |

#### `set_port_fanout_number`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `value` `port_list` | Extra `-min` `-max`. Used with wire-load estimation. |

#### `set_timing_derate`

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-cell_delay` `-cell_check` `-net_delay` `-data` `-clock` `-early` `-late` `-rise` `-fall` | Default without type flags: net+cell delay, clock+data. Cell checks are not derated unless `-cell_check`. |
| ❌ | `-static` `-dynamic` `-increment` | SDC 2.1; not parsed. |
| ✅ | `derate_value` `[object_list]` | Instances, liberty cells, nets. |

#### Wire load (listed at the end of A-4 in the PDF)

| Mark | Command | Notes |
| --- | --- | --- |
| ⚠️ | `set_wire_load_min_block_size` | Warn 477. |
| ☑️ | `set_wire_load_mode top\|enclosed\|segmented` | Stored and written. Delay calc does not consult the mode (always the selected model). |
| ☑️ | `set_wire_load_model -name [-library] [-min] [-max] [object_list]` | Global model ✅ (`EstimateParasitics` Pi-Elmore). `object_list` ignored. |
| ☑️ | `set_wire_load_selection_group` | Stored. Area-based auto-select is not applied. `object_list` ignored. |

#### `set_voltage`

Syntax is in A-4 in the PDF (after derate), not only in A-5.

| Mark | Argument | Notes |
| --- | --- | --- |
| ✅ | `-min` `-object_list` `max_case_voltage` | Global max always; optional min and power nets. |

### Multivoltage and Power Optimization Commands (Table A-5)

These are synthesis, floorplan, and UPF-era power-intent commands, not
STA. OpenSTA accepts them so DC-written SDC loads.

| Mark | Command | Notes |
| --- | --- | --- |
| N/A | `create_voltage_area` `-name` `-coordinate` `-guard_band_x` `-guard_band_y` `cell_list` | Placement region. Parsed, discarded. UPF `create_power_domain` is the modern form. |
| N/A | `set_level_shifter_strategy [-rule]` | Compile/PnR insertion. Parsed, discarded. UPF `set_level_shifter`. |
| N/A | `set_level_shifter_threshold [-voltage] [-percent]` | Same. `-percent` is not in the OpenSTA argument list. Discarded. |
| ☑️ | `set_max_dynamic_power power [unit]` | Optimization cap, like `set_max_area`. Stored and written. Optional unit ignored (use `set_units`). Unused in analysis. `report_power` is activity-based, not this cap. |
| ☑️ | `set_max_leakage_power power [unit]` | Same. |

---

## Unsupported summary

N/A rows (version pinning, voltage areas, level shifters, path-group
weights, power/area budgets) are omitted here. They are not STA gaps.

Importance is for **gate-level STA without SI or UPF**. Nothing on this
list blocks ordinary clocked STA. **Medium** means it appears in real
SDC or changes slack in a common flow. **Low** is rare, SI-only, or
has an easy workaround.

**2.1** means the option was added in SDC 2.1. Everything else is from
2.0 or earlier.

| Feature | SDC | Importance | Why it matters |
| --- | --- | --- | --- |
| `set_ideal_network` / `set_ideal_latency` / `set_ideal_transition` | ≤2.0 | Medium | Pre-CTS: those nets should not pick up estimated delay. Parsed, discarded. With SPEF, usually moot. |
| `set_operating_conditions -object_list` | ≤2.0 | Medium | Per-instance PVT. Not parsed. Extra `set_pvt` does the same job. |
| `set_voltage` | ≤2.0 | Medium | Stored and written (A-4 ✅). Not used for k-factor or delay calc. |
| `set_timing_derate -increment` | 2.1 | Medium | Add to an existing derate instead of replacing it. Not SI. |
| `set_sense -clock_leaf` | 2.1 | Medium | Treat a pin as a clock endpoint (macros / hierarchical clocks). Not parsed. |
| `set_sense -non_unate` | 2.1 | Low | Both clock edges through a mux/XOR. Not parsed. Liberty/sim unateness still applies. |
| `all_inputs` / `all_outputs` `-clock` `-level_sensitive` `-edge_triggered` | ≤2.0 | Low | Object filters only. Use `get_ports` / extra `all_inputs -no_clocks`. |
| `set_input_delay` / `set_output_delay` `-level_sensitive` | ≤2.0 | Low | Latch-style I/O. Not parsed. |
| `set_clock_latency -dynamic` | 2.1 | Low | SI component of ideal clock latency. Not parsed. |
| `set_timing_derate -static` / `-dynamic` | 2.1 | Low | Derate base delay vs crosstalk separately. Not parsed. |
| `set_sense -pulse` | ≤2.0 | Low | Warn 416. Liberty `pulse_clock` still propagates. |
| `set_sense -type data` | 2.1 | Low | Data-path sense. Warn 413. |
| `set_driving_cell -clock` `-clock_fall` | ≤2.0 | Low | Per-clock drive. Not parsed. |
| `set_input_transition -clock` `-clock_fall` | ≤2.0 | Low | Warn 462/463. |
| `all_registers` `-no_hierarchy` `-hsc` `-slave_clock_pins` `-master_slave` | ≤2.0 | Low | Object filters. Other `all_registers` options work. |
| `set_fanout_load` | ≤2.0 | Low | Warn 461. Obsolete load constraint. |
| `set_wire_load_min_block_size` | ≤2.0 | Low | Warn 477. |
| `set_wire_load_mode` / `set_wire_load_selection_group` | ≤2.0 | Low | Stored. Delay calc uses the selected model only. Irrelevant once SPEF is read. |

