# suppress and unsuppress message ids

# Run sta_warn/sta_error to test TCL side suppression
proc sta_cmd { msg } {
  puts "sta_cmd call $msg"
  sta::sta_warn 1 "cmd error"
  sta::sta_error 2 "cmd error"
  puts "after error"
}

# Run report_warn/report_error to test C++ side suppression
proc report_cmd { msg } {
  puts "report_cmd call $msg"
  sta::report_warn 1 "cmd error"
  sta::report_error 2 "cmd error"
  puts "after error"
}

# Ensure that TCL side messages are displayed as usual
catch { sta_cmd 1 } error
puts $error

# Ensure that C++ side messages are displayed as usual
catch { report_cmd 1 } error
puts $error

# Suppress messages
suppress_msg 1
suppress_msg 2

# Ensure that TCL side messages are suppressed
catch { sta_cmd 2 } error
puts "caught $error"

# Ensure that C++ side messages are suppressed
catch { report_cmd 2 } error
puts "caught $error"

# Continue on error to avoid having to catch
set sta_continue_on_error 1

# Ensure that TCL side messages are suppressed
# TCL side will make it to "after error"
sta_cmd 3
sta_cmd 4

# Ensure that C++ side messages are suppressed
# C++ will not make it to "after error" as the whole cmd is cancelled
report_cmd 3
report_cmd 4