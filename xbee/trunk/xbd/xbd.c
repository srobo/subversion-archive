#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <signal.h>
#include "xbee-server.h"

void parse_config_file( void );
void config_create( int argc, char **argv );
static void xbd_kill( int signum );

static gchar *config = NULL, 
	*sdev = NULL,
	*listen = NULL;

static GMainLoop *ml = NULL;
static XbeeServer *server = NULL;

static GOptionEntry entries[] = 
{
	{ "config", 'c', 0, G_OPTION_ARG_FILENAME, &config, "Config file filename" },
	{ "serial", 's', 0, G_OPTION_ARG_FILENAME, &sdev, "Serial device" },
	{ "listen", 'l', 0, G_OPTION_ARG_STRING, &listen, "Socket path to listen on" },
	{ NULL }
};

int main( int argc, char** argv )
{
	XbeeModule *xb;
	GMainContext *context;

	g_type_init();

	config_create( argc, argv );

	ml = g_main_loop_new( NULL, FALSE );
	context = g_main_loop_get_context( ml );

	server = xbee_server_new( context, listen );
	g_return_val_if_fail( server != NULL, 3 );

	if( sdev != NULL )
	{
		xb = xbee_module_open( sdev, context );
		g_return_val_if_fail( xb != NULL, 1 );
		
		xbee_server_attach( server, xb );
	}

	if( signal( SIGINT, xbd_kill ) == SIG_ERR )
		g_print("ERROR\n");
	if( signal( SIGTERM, xbd_kill ) == SIG_ERR )
		g_print("ERROR\n");

	g_main_loop_run( ml );

	xbee_server_free( server );
	xbee_module_close( xb );

	return 0;
}

void config_create( int argc, char **argv )
{
	GOptionContext *opt_context;
	GError *error = NULL;
	
	opt_context = g_option_context_new( "" );  
	g_option_context_set_summary( opt_context, "xbee server" );
	g_option_context_add_main_entries(opt_context, entries, NULL);

	if(!g_option_context_parse( opt_context, &argc, &argv, &error ))
	{
		g_print("Error: %s\n", error->message);
		exit(1);
	}

	if( config != NULL )
		parse_config_file();
	
	if( sdev == NULL )
		g_print( "Warning: No serial port specified = no xbee!\n" );

	if( listen == NULL )
		listen = "/tmp/xbee";
}

void parse_config_file( void )
{
	GKeyFile *f;
	GError *error = NULL;

	f = g_key_file_new();
	if( !g_key_file_load_from_file( f, 
					config,
					G_KEY_FILE_NONE,
					&error ) )
		g_print("Failed to parse config file: %s\n", error->message);

	
	if( g_key_file_has_group( f, "server" ) 
	    && g_key_file_has_key( f, "server",
				   "listen", NULL ) )
		listen = g_key_file_get_value(f, "server", "listen", NULL);

	if( g_key_file_has_group( f, "xbee" ) 
	    && g_key_file_has_key( f, "xbee",
				   "sdev", NULL ) )
		sdev = g_key_file_get_value(f, "xbee", "sdev", NULL);
}

static void xbd_kill( int signum )
{
	g_print( "\n xbd shutting down.\n" );

	if( ml != NULL )
	{
		g_main_loop_quit(ml);

		/* Delete the server socket */
		if( server != NULL )
			g_object_unref( server );
	}
}
