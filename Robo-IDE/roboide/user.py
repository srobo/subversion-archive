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
        teams = {}
        for team in getteams():
            teams[team] = model.TeamNames.get(team).name

        # Get the setting values
        svals = model.SettingValues.select( model.SettingValues.q.uname == get_curuser() )
        settings = {}
        for sval in svals.lazyIter():
            sname = model.Settings.get(sval.id).name
            settings[sname] = sval.value

        return {"teams" : teams, "settings": settings}

    @expose("json")
    def login(self, usr="",pwd=""):
        def ldap_login():
            """LDAP Anonymous Login"""
            password = config.get("ldap.anonpass")
            return (config.get("ldap.anonuser"),password)

        u = sr.user( usr )
        if not u.in_db:
            return {"login": 0}

        if u.bind( pwd ):
            return {"login": 1}
        else:
            return {"login": 0}

def dev_env():
    """Returns True if we're in a development environment"""
    return config.get("server.environment" ) == "development"

def get_curuser():
    """Returns the user we're currently acting as"""

    # Default to the development user if the header isn't present
    if not cherrypy.request.headers.has_key( config.get("user.header" ) ):
        if dev_env():
            return config.get( "user.default" )
        else:
            return None
    else:
        return cherrypy.request.headers["X-Forwarded-User"]

def getteams():
    """Return a list of the teams the user's in"""
    username = get_curuser()

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
    


    
