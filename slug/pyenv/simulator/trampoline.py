import types
import robot
from events import TimeoutEvent
import time

class Trampoline:
    def schedule(self):
        """
        Run the main function until it yields, then go through the polling
        functions until one of them yields / returns an event or the yield times
        out.
        """

        stack = []

        robot.event = None
        
        stack.append(robot.main(corner=0,colour=0,game=0))
        
        while True:
            try:
                #Try to run the function on the top of the stack
                args = stack[-1].next() #Advance to the first yield statement
            except StopIteration:
                #Remove the function on the top of the stack
                stack.pop()
                if len(stack) == 0:
                    #Run out of things to trampoline
                    #Put the main function back in...
                    stack.append(robot.main(corner=0,colour=0,game=0))
                    #Go back to start of while loop
                    continue

            if args[0].__class__ == types.FunctionType:
                #Push the function onto the stack
                #Passing the rest of the yield as arguments
                stack.append(args[0](*args[1:]))
            else:
                polls = []
                timeout = None
                for arg in args:
                    if arg.__class__ == types.GeneratorType:
                        polls.append(arg)
                    elif isinstance(arg, int) or isinstance(arg, float):
                        timeout = time.time() + arg
                
                #Check we have something to wait on
                if len(polls) > 0 or timeout != None:
                    #Loop waiting for something to happen
                    result = None
                    while True:
                        if timeout != None and timeout < time.time():
                            #Timed out
                            result = TimeoutEvent(timeout)
                        else:
                            for poll in polls:
                                try:
                                    result = poll.next()
                                except StopIteration:
                                    polls.remove(poll)

                        #See if anything happened
                        if result != None:
                            robot.event = result
                            #Break out of this inner while loop
                            break
                    
if __name__ == "__main__":
    t = Trampoline()
    t.schedule()
