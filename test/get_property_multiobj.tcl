# get_property on multiple objects at once
read_liberty asap7_small.lib.gz
puts {[get_property [get_lib_cells *] name]}
puts [get_property [get_lib_cells *] name]
