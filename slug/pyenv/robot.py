from dio import diopoll

currentevent = None

def main(trampoline):
    print "Hello there, this is the start of the cheese"
    print "Going into a loop"

    while 1:
        #First, wait for 2 seconds
        print "Waiting for 2 seconds"
        yield 2
        print "Waited for 2 seconds"
        #Now, wait for a number
        trampoline.addtask(diopoll())
        print "Waiting for a number (timeout = 0)"
        yield 0
        print "Got event of class:"
        print currentevent.__class__
        print "Value: %d" % currentevent.value
