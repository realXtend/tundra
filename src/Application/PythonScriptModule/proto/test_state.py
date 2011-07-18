import state

import state_1
import state_2

print state.x
assert state.x == 0

state_1.change()
print state.x
assert state.x == 1

state_2.change()
print state.x
assert state.x == 2

print "success - sharing state via a module works as expected."
