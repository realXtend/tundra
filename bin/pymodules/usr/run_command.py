"""just to overcome the prob that console PyLoad uses import,
and the parser there prevents using '(' in args so can't do reload :p

probably a good idea to add a proper py console soon-ish."""

import input
input = reload(input)