import ldap, sys, getpass

conn = None
bound = False

if conn == None:
    conn = ldap.initialize("ldap://127.0.0.1/")

def bind():
    global bound, conn

    if not bound:
        try:
            conn.simple_bind_s( "cn=Manager,o=sr", getpass.getpass() )
        except ldap.INVALID_CREDENTIALS:
            print "Incorrect password"
            sys.exit(1)

        bound = True

