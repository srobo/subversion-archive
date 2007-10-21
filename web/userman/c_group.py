import srgroup, sys

class group:
    """Manage groups.
    Subcommands:
     - list
     - info
     - create
     - rm"""

    def __init__(self, args):
        self.commands = { "help" : self.help,
                          "info" : self.info,
                          "create" : self.create,
                          "rm" : self.delete,
                          "list" : self.list
                          }

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

    def list(self, args):
        """List all groups."""

        if len(args) > 0:
            print self.__doc__
            return

        print " ".join(srgroup.list())

    def help(self, args):
        if len(args) < 1:
            print self.__doc__
            return

        if args[0] in self.commands:
            print self.commands[args[0]].__doc__
        else:
            print "Command not found."
            print self.__doc__

        sys.exit(0)

    def create(self, args):
        """Create a group.
Usage:
	group create GROUP_NAME [USERS]"""

        if len(args) < 1:
            print self.create.__doc__
            return

        g = srgroup.group( args[0] )

        if g.in_db:
            print "Group '%s' already exists" % (args[0] )
            return

        if len(args) > 1:
            users = args[1:]
            g.user_add(users)

        g.save()

    def delete(self, args):
        """Delete a group
Usage:
	group rm GROUPNAME"""
        if len(args) < 1:
            print self.delete.__doc__
            return

        g = srgroup.group( args[0] )

        if not g.in_db:
            print "Group '%s' doesn't exist" % ( args[0] )
            return

        g.rm()
        print "Group '%s' deleted." % (args[0] )

    def info(self, args):
        """Print information about a group.
Usage:
	group info USERNAME"""

        if len(args) < 1:
            print self.info.__doc__
            return

        g = srgroup.group( args[0] )

        if not g.in_db:
            print "Group '%s' not found\n" % (args[0])
        else:
            print g

