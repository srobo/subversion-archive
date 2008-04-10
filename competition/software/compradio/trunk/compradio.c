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

#include "comp-types.h"
#include "comp-mysql.h"
#include "comp-xbee.h"

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

/* Transmits the ping signal */
gboolean tx_ping( gpointer nothing );

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
	*hbox1 = NULL;

/* The spins for the different colours */
GtkWidget *spin_colours[4];

xb_addr_t team_addresses[4];

/* The current match */
gint cur_match = 0;
/* The curent match info */
match_t cur_match_info;

/* Update the match */
void update_match( void );

int main( int argc, char** argv )
{
	GladeXML *xml;
	gtk_init( &argc, &argv );

	xml = glade_xml_new("compradio.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	b_start = glade_xml_get_widget(xml, "b_start");
	b_stop = glade_xml_get_widget(xml, "b_stop");
	check_ping = glade_xml_get_widget(xml, "check_ping");
	spin_match = glade_xml_get_widget(xml, "spin_match");

	spin_colours[RED] = glade_xml_get_widget(xml, "spin_red");
	spin_colours[BLUE] = glade_xml_get_widget(xml, "spin_blue");
	spin_colours[GREEN] = glade_xml_get_widget(xml, "spin_green");
	spin_colours[YELLOW] = glade_xml_get_widget(xml, "spin_yellow");

	hbox1 = glade_xml_get_widget(xml, "hbox1");

	if( !comp_xbee_init() ) {
		fprintf( stderr, "Error connecting to xbd\n" );
		return 1;
	}

	sr_mysql_init();
	change_state( S_IDLE );

	cur_match = 1;
	update_match();

	/* The ping timeout */
	g_timeout_add(100, (GSourceFunc)tx_ping, NULL);

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
		if( state != S_IDLE )
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

gboolean tx_ping( gpointer nothing )
{
	/* Channel 1 */
	if( pinging ) {
		uint8_t i;
		for(i=0; i<4; i++) {
			if( cur_match_info.teams[i] != 0 )
				comp_xbee_ping( &team_addresses[i] );
		}

		printf("PING\n");
	}

	return TRUE;
}

void change_state( state_t n )
{
	state = n;
	printf("Switching to: ");

	switch( state )
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

	case S_STARTED: {
		uint8_t i;
		printf("S_STARTED\n");

		/* Send START signals to everything */
		for( i=0; i<4; i++ ) {
			if( cur_match_info.teams[i] != 0 )
			{
				/* Generate the START string */
				char* s = NULL;

				asprintf( &s,
					  "corner=%u, colour=%u, game=%u",
					  i, i, 0 );

				comp_xbee_start( &team_addresses[i], s );
				free(s);
				s = NULL;
			}
		}

		gtk_widget_set_sensitive( b_start, FALSE );
		gtk_widget_set_sensitive( b_stop, TRUE );
		
		break;
	}
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
	uint8_t i;

	printf("Updating to match %i\n", cur_match);
	
	if( !sr_match_info( cur_match, &cur_match_info ) ) {
		for(i=0; i<4; i++)
			cur_match_info.teams[i] = 0;
		cur_match_info.time = 0;
	}
		
	for( i=0; i<4; i++ )
		gtk_spin_button_set_value( GTK_SPIN_BUTTON(spin_colours[i]), (gdouble)cur_match_info.teams[i] );

	/* Get the new team addresses */
	for(i=0; i<4; i++)
	{
		uint16_t team = 0;
		team = cur_match_info.teams[i];

		if( team != 0 )
			sr_team_get_addr( team, &team_addresses[i] );
	}
}

