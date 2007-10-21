import srusers, sys

class user:
    """Manage user accounts.
    Subcommands:
     - list - List all users
     - info - Display user information
     - add - Add a user
     - rm - Remove a user
     - groups - Display a list of groups a user is in"""

    def __init__(self, args):
        self.commands = { "help" : self.help,
                          "info" : self.info,
                          "add" : self.add,
                          "rm" : self.delete,
                          "list" : self.list }

        if len(args) < 1:
            self.help([])
            return

        if args[0] in self.commands:
            cmd = args[0]

            if len(args) == 1:
                args = []
            else:
                args = args[1:]

            self.commands[cmd](args)
        else:
            print self.__doc__

    def help(self, args):
        if len(args) < 1:
            print self.__doc__
            return

        if args[0] in self.commands:
            print self.commands[args[0]].__doc__

        sys.exit(0)

    def add(self, args):
        """Add a user.
Usage:
	user add USERNAME FIRST_NAME LAST_NAME EMAIL"""

        if len(args) < 4:
            print self.add.__doc__
            return

        u = srusers.user( args[0] )

        if u.in_db:
            print "User '%s' already exists" % (args[0] )
            return

        u.cname = args[1] + " " + args[2]
        u.sname = args[2]
        u.email = args[3]

        if u.save():
            print "User '%s' successfully created." % (args[0])
        else:
            print "Failed to create user '%s'"

    def list(self, args):
        """List all users"""
        if len(args) > 0:
            print self.list.__doc__
            return

        print " ".join(srusers.list())

    def delete(self, args):
        """Delete a user.
Usage:
	user rm USERNAME"""
        if len(args) < 1:
            print self.delete.__doc__
            return

        u = srusers.user( args[0] )
        if u.delete():
            print "User '%s' deleted" % (args[0])
        else:
            print "Error: User '%s' could not be deleted"

    def info(self, args):
        """Print information about a user.
Usage:
	user info USERNAME"""

        if len(args) < 1:
            print self.info.__doc__
            return

        u = srusers.user( args[0] )

        if not u.in_db:
            print "User '%s' not found\n" % (args[0])
        else:
            print u

            
        
        

