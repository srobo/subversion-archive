import robot
from events import *
import time

BIG_TIME = 9999999999999

class trampoline:
    q = [] #A list of polling tasks to execute when main stalled

    def addtask(self, name, task):
        """
        This function adds a task to the queue. Tasks must be generators.
        """
        task.next() #Advance generator to first yield
        self.q.append((name, task))

    def gettimeouttime(self, timeout):
        if timeout > 0:
            timeouttime = time.time() + timeout
        else:
            timeouttime = BIG_TIME #Largest possible time.
        
        return timeouttime


    def schedule(self):
        """
        Run the main function until it yields, then go through the polling
        functions until one of them yields / returns an event or the yield times
        out.
        """

        main = robot.main(self)
        robot.currentevent = None
        timeout = main.next() #Advance to the first yield statement.
        timeouttime = self.gettimeouttime(timeout)
        
        while 1:
            if time.time() > timeouttime: #Check to see if the yield has timed out
                robot.currentevent = TimeoutEvent()
                try:
                    timeout = main.next()
                    timeouttime = self.gettimeouttime(timeout)
                except StopIteration:
                    return

            for name, task in self.q:
                try:
                    result = task.next()
                except StopIteration:
                    self.q.remove(task)
                    continue

                if result != None:
                    #Got an event for the mainloop
                    robot.currentevent = result
                    robot.currentevent.name = name
                    try:
                        timeout = main.next()
                    except StopIteration:
                        return
                    timeouttime = self.gettimeouttime(timeout)
                    break #Break out of the for loop
            
if __name__ == "__main__":
    t = trampoline()
    t.schedule()
