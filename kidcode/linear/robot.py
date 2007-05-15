class motor:
    def __init__(self, id):
        """Create a motor"""
        print "Created a motor with id %d." % id
        self.id = id
        self.speed = 0
        self.rotations = 0

    def set_speed(self, speed):
        print "Motor %d set speed to %d" % (self.id, speed)
        self.speed = speed

    def get_speed(self):
        return self.speed

    def restart_count(self):
        self.rotations = 0

    def get_rotations(self):
        """This returns the number of rotations of the wheel
        since it was reset. Needs implementing"""
        return 5

class digitalio():
    def __init__(self, id):
        print "Created digital io with id %d" % id
        self.id = id
        self.channels = [0 for i in range(0,7)]

    def set_channel(self, number, value):
        self.channels[number] = value

    def get_channel(self, number):
        return self.channels[number]
