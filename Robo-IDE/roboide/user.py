from turbogears import config
import cherrypy
import sr

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

    if dev_env() and not config.get( "user.user_ldap" ):
        # Use the default group list when not using LDAP
        groups = config.get( "user.default_groups" )
    else:
        def ldap_login():
            """LDAP Anonymous Login"""
            password = config.get("ldap.anonpass")
            return (config.get("ldap.anonuser"),password)
    
        sr.set_userinfo(ldap_login)

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
    


    
