from turbogears.database import PackageHub
from sqlobject import *

hub = PackageHub("simtest")
__connection__ = hub

# class YourDataClass(SQLObject):
#     pass

