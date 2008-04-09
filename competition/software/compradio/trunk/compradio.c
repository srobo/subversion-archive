#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <xbee-conn.h>

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
	*check_ping = NULL;

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

	xbc = xbee_conn_new( "/tmp/xbee", NULL );
	xbee_conn_register_callbacks( xbc, &xb_callbacks );

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
