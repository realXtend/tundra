from sys import stdout

def send(msg):
    stdout.write('%s\n' % msg)
    stdout.flush()
