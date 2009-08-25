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
    description = StringCol()

class RoboPresent(SQLObject):
    team = ForeignKey('TeamNames')
    present = BoolCol()

class RoboLogs(SQLObject):
    #The team
    team = ForeignKey('TeamNames')
    #Time log was written
    date = DateTimeCol(default = DateTimeCol.now)
    #Value written
    value = StringCol()

# Holds the settings
class SettingValues(SQLObject):
    # The setting ID
    setting_id = IntCol()
    # The user that this setting is for
    uname = StringCol()
    # The setting value
    value = StringCol()

# Holds the autosaved files
class AutoSave(SQLObject):
    # The full file name and path
    file_path = StringCol()
    # The revision that the file is based on
    revision = IntCol()
    # The team of the user that saved the file
    team_id = IntCol()
    # The user that saved the file
    uname = StringCol()
    # The date and time of the save, defaults to now
    date = DateTimeCol(default = DateTimeCol.now)
    # The file contents
    content = StringCol()

class FirmwareTargets(SQLObject):
    """Devices that we manage firmware for."""
    # The name of the device.
    name = StringCol()

class FirmwareBlobs(SQLObject):
    # The device (ForeignKey doesn't work in the sqlobject on button)
    device = IntCol() #ForeignKey("FirmwareTargets")

    # The version number
    version = IntCol()

    # The firmware filename
    firmware = StringCol()

    # The revision number in VC that the firmware is built from.
    # Current supported formats:
    #  - "svn:REV" where REV is the subversion commit number.
    revision = StringCol()

    # A description of the firmware.  Could contain a changelog.
    description = StringCol()

class FirmwareState(SQLObject):
    # The firmware this relates to. (ForeignKey doesn't work in the sqlobject on button)
    fw_id = IntCol() #ForeignKey("FirmwareBlobs")

    # The date and time of state change
    date = DateTimeCol()

    # An message to go with the state change
    message = StringCol()

    # The state the firmware changed to
    state = EnumCol( enumValues = [ "ALLOCATED",
                                    "DEVEL",
                                    "TESTING",
                                    "SHIPPING",
                                    "FAILED",
                                    "OLD_RELEASE",
                                    "SUPERCEDED" ] )
class UserBlogFeeds(SQLObject):
	#the team id
	team_id = IntCol()
	#the url of the rss/atom feed
	url = StringCol()
	#validated by student robotics admin
	valid = BoolCol()
