/* Student Robotics - Competition radio management */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <glade/glade.h>
#include <xbee-conn.h>
#include <mysql/mysql.h>

enum {
	RADIO_CMD_START,
	RADIO_CMD_STOP,
	RADIO_CMD_PING,
	RADIO_CMD_PING_RESP
};

/* Window destruction */
gboolean main_window_destroy( GtkObject *object,
			      gpointer user_data );

/* Window deletion */
gboolean main_window_delete_event( GtkWidget *widget,
				   GdkEvent *event,
				   gpointer user_data );


/* The ping enable/disable checkbox */
void on_check_ping_toggled( GtkToggleButton *togglebutton,
			    gpointer user_data );

void on_b_start_clicked( GtkButton *button,
			 gpointer user_data );

void on_b_stop_clicked( GtkButton *button,
			gpointer user_data );

void spin_match_value_changed( GtkSpinButton *spinbutton,
			       gpointer user_data );

const xb_conn_callbacks_t xb_callbacks = 
{
	.rx_frame = NULL,
	.chan_set = NULL
};

/* Transmits the ping signal */
gboolean tx_ping( XbeeConn *xbc );

/* Whether we're currently pinging: */
gboolean pinging = FALSE;

/* States */
typedef enum {
	/* Nothing happening */
	S_IDLE,

	/* Pinging robots */
	S_INIT,

	/* Robots currently running */
	S_STARTED
} state_t;

state_t state = S_IDLE;

void change_state( state_t n );

GtkWidget *b_start = NULL,
	*b_stop = NULL,
	*check_ping = NULL,
	*spin_match = NULL,
	*spin_red = NULL,
	*spin_blue = NULL,
	*spin_green = NULL,
	*spin_yellow = NULL;

typedef struct {
	uint16_t red, green, blue, yellow;
	uint32_t time;
} match_t;

/* The current match */
gint cur_match = 0;
/* The curent match info */
match_t cur_match_info;

/*** Mysql related things ***/
/* Initialise the mysql connection */
void sr_mysql_init( void );

/* Get the teams playing in match N.
   Fills in the struct pointed to by m.
   Returns true if match exists.
 */
gboolean sr_match_info( uint16_t N, match_t* m );

/* Update the match */
void update_match( void );

/* The mysql handle */
MYSQL *db = NULL;

int main( int argc, char** argv )
{
	GladeXML *xml;
	XbeeConn *xbc;
	gtk_init( &argc, &argv );

	xml = glade_xml_new("compradio.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	b_start = glade_xml_get_widget(xml, "b_start");
	b_stop = glade_xml_get_widget(xml, "b_stop");
	check_ping = glade_xml_get_widget(xml, "check_ping");
	spin_match = glade_xml_get_widget(xml, "spin_match");

	spin_red = glade_xml_get_widget(xml, "spin_red");
	spin_blue = glade_xml_get_widget(xml, "spin_blue");
	spin_green = glade_xml_get_widget(xml, "spin_green");
	spin_yellow = glade_xml_get_widget(xml, "spin_yellow");

	xbc = xbee_conn_new( "/tmp/xbee", NULL );
	if( xbc == NULL ) {
		fprintf( stderr, "Error connecting to xbd\n" );
		return 1;
	}
	xbee_conn_register_callbacks( xbc, &xb_callbacks );

	sr_mysql_init();
	change_state( S_IDLE );

	cur_match = 1;
	update_match();

	/* The ping timeout */
	g_timeout_add(100, (GSourceFunc)tx_ping, (gpointer)xbc);

	gtk_main();

	return 0;
}

gboolean main_window_destroy( GtkObject *object,
			      gpointer user_data )
{
	gtk_main_quit();
	return TRUE;
}

gboolean main_window_delete_event( GtkWidget *widget,
				   GdkEvent *event,
				   gpointer user_data )
{
	return FALSE;
}

void on_check_ping_toggled( GtkToggleButton *tb,
			    gpointer user_data )
{
	if( gtk_toggle_button_get_active( tb ) ) {

		if( state == S_IDLE )
			change_state( S_INIT );

		pinging = TRUE;
	} else {
		change_state( S_IDLE );
		pinging = FALSE;
	}
}

void on_b_start_clicked( GtkButton *button,
			 gpointer user_data )
{
	change_state( S_STARTED );
}

void on_b_stop_clicked( GtkButton *button,
			gpointer user_data )
{
	change_state( S_IDLE );
}

gboolean tx_ping( XbeeConn *xbc )
{
	uint8_t data[] = { RADIO_CMD_PING };
	xb_addr_t addr =
		{
			.type = XB_ADDR_64,
			.addr = {0x00,0x13,0xA2,0x00,0x40,0x02,0x64,0x1E}
		};

	assert( xbc != NULL );

	/* Channel 1 */
	if( pinging )
		xbee_conn_transmit( xbc, addr, data, 1, 1 );

	return TRUE;
}

void change_state( state_t n )
{
	printf("Switching to: ");

	switch( n )
	{
	case S_IDLE:
		printf("S_IDLE\n");

		gtk_widget_set_sensitive( b_start, FALSE );
		gtk_widget_set_sensitive( b_stop, FALSE );
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(check_ping), FALSE );
		break;

	case S_INIT:
		printf("S_INIT\n");
		gtk_widget_set_sensitive( b_start, TRUE );
		gtk_widget_set_sensitive( b_stop, FALSE );
		
		break;

	case S_STARTED:
		printf("S_STARTED\n");

		gtk_widget_set_sensitive( b_start, FALSE );
		gtk_widget_set_sensitive( b_stop, TRUE );
		
		break;
	}
}

gboolean sr_match_info( uint16_t N, match_t* m )
{
	char* q = NULL;
	MYSQL_RES *res;
	MYSQL_FIELD *fields;
	MYSQL_ROW row;
	unsigned int n_fields, i;
	assert( m != NULL );
	m->time = 0;
	m->red = m->green = m->blue = m->yellow = 0;

	asprintf(&q, "SELECT * FROM matches WHERE number = %hu LIMIT 1;", N );
	if( mysql_query( db, q ) != 0 ) {
		fprintf(stderr, "Failed to grab match %hhu info.", N );
		return FALSE;
	}
      
	res = mysql_store_result( db );

	if( mysql_num_rows( res ) == 0 ) {
		printf("Match %hhu not found\n", N);
		return FALSE;
	}

	n_fields = mysql_num_fields( res );
	fields = mysql_fetch_fields( res );
	while((row = mysql_fetch_row(res))) {
		for(i=0; i<n_fields; i++) {
			/* printf("%s = %s\n", fields[i].name, row[i]); */

			if( strcmp(fields[i].name,"time") == 0 )
				m->time = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"red")==0)
				m->red = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"blue")==0)
				m->blue = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"green")==0)
				m->green = strtoul(row[i], NULL, 10);
			else if (strcmp(fields[i].name,"yellow")==0)
				m->yellow = strtoul(row[i], NULL, 10);
		}
	}
	mysql_free_result(res);
	free(q);
	return TRUE;
}

void sr_mysql_init( void )
{
	db = mysql_init( NULL );
	assert(db != NULL);

	if( mysql_real_connect( db, 
				"127.0.0.1", 
				"comp",
				"lemmings",
				"comp",
				8000,
				NULL,
				0 ) == NULL ) {
		fprintf(stderr, "Couldn't connect to mysql: %s\n",
			mysql_error(db));
		exit(1);
	}
}

void spin_match_value_changed( GtkSpinButton *spinbutton,
			       gpointer user_data )
{
	cur_match = gtk_spin_button_get_value_as_int(spinbutton);
	update_match();
}

void update_match( void )
{
	printf("Updating to match %i\n", cur_match);
	if( !sr_match_info( cur_match, &cur_match_info ) ) {
		cur_match_info.red = cur_match_info.green = cur_match_info.blue = cur_match_info.yellow = 0;
		cur_match_info.time = 0;
	}
		
	gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_red), (gdouble)cur_match_info.red );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_green), (gdouble)cur_match_info.green );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_yellow), (gdouble)cur_match_info.yellow );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_blue), (gdouble)cur_match_info.blue );
}
