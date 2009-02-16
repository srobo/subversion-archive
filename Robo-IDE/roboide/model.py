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
    desc = StringCol()

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
