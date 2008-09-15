from turbogears.database import PackageHub
from sqlobject import *

hub = PackageHub("roboide")
__connection__ = hub

# class YourDataClass(SQLObject):
#     pass

class TeamNames(SQLObject):
    name = StringCol()
