from turbogears import config, expose
import model
import cherrypy
import sr

def anon_login():
    """LDAP Anonymous Login"""
    password = config.get("ldap.anonpass")
    return (config.get("ldap.anonuser"),password)

# Bind to LDAP with the anonymous user to access user information
sr.set_userinfo( anon_login )

class User(object):
    @expose()
    def index(self):
        return "Bees!"

    @expose("json")
    def info(self):
        """Returns a variety of information about the user
        outputs:
          - teams: dict mapping team numbers to team names."""
        user = get_curuser()
        if user == None:
            return {}

        teams = {}
        for team in getteams():
            try:
                teams[team] = model.TeamNames.get(team).name
            except model.SQLObjectNotFound:
                teams[team] = "Unnamed team"

        # Get the setting values
        svals = model.SettingValues.select( model.SettingValues.q.uname == user )
        settings = {}
        for sval in svals.lazyIter():
            sname = model.Settings.get(sval.id).name
            settings[sname] = sval.value

        return { "user" : user,
                 "teams" : teams, 
                 "settings": settings}

    @expose("json")
    def login(self, usr="",pwd=""):
        SUCCESS = {"login" : 1}
        FAIL = {"login": 0}

        # When not using LDAP, logins are always successful
        if dev_env() and not config.get( "user.use_ldap" ):
            return SUCCESS

        u = sr.user( usr )
        if not u.in_db:
            return FAIL

        if u.bind( pwd ):
            cherrypy.session["user"] = usr
            return SUCCESS
        else:
            return FAIL

    @expose("json")
    def logout(self):
        cherrypy.session.clear()
        return {}

def dev_env():
    """Returns True if we're in a development environment"""
    return config.get("server.environment" ) == "development"

def get_curuser():
    """Returns the user we're currently acting as.
    Returns None if not logged in."""

    if dev_env() and not config.get( "user.use_ldap" ):
        return config.get( "user.default" )
    else:
        # Use LDAP
        if not cherrypy.session.has_key( "user" ):
            return None
        return cherrypy.session["user"]

def getteams():
    """Return a list of the teams the user's in"""
    username = get_curuser()
    if username == None:
        return []

    groups = None

    if dev_env() and not config.get( "user.use_ldap" ):
        # Use the default group list when not using LDAP
        groups = config.get( "user.default_groups" )
    else:
        if username in sr.users.list():
            user = sr.user(username)
            groups = user.groups()
        else:
            return RuntimeError("Could not find user")

    return [int(group[4:]) for group in groups if "team" in group]

def get_svnrepo( team ):
    """Return the subversion repository URL for the current user and given team.
    Given team must be an integer."""
    return config.get( "svn.repos" ) % { "team" : team }
    


    
