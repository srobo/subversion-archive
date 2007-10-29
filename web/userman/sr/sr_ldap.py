import ldap, sys, getpass

conn = None
bound = False

if conn == None:
    conn = ldap.initialize("ldap://127.0.0.1/")

def default_pass():
    return ("cn=Manager,o=sr",getpass.getpass())

user_callback = default_pass

def set_userinfo( fn ):
    global user_callback
    user_callback = fn

def bind():
    global bound, conn, user_callback

    if not bound:
        info = user_callback()
        try:
            conn.simple_bind_s( info[0], info[1] )
        except ldap.INVALID_CREDENTIALS:
            print "Incorrect password"
            sys.exit(1)

        bound = True

