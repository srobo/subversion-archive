from turbogears.database import PackageHub
from sqlobject import *

hub = PackageHub("roboide")
__connection__ = hub

# Holds ID to team name mappings
class TeamNames(SQLObject):
    name = StringCol()

# Holds the setting name -> ID mapping
class Settings(SQLObject):
    # The setting name
    name = StringCol()
    # The setting description
    desc = StringCol()

# Holds the settings 
class SettingValues(SQLObject):
    # The setting ID
    setting_id = IntCol()
    # The user that this setting is for
    uname = StringCol()
    # The setting value
    value = StringCol()
