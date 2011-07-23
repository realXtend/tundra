print "--** communication.py **--"

import time

prev = time.time()

def update():
    global prev
    
    if time.time() > prev + 1:
        print ".",
        prev = time.time()
