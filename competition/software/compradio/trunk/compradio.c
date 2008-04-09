#include <stdio.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
/* #include <xbee-conn.h> */

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

int main( int argc, char** argv )
{
	GladeXML *xml;
	gtk_init( &argc, &argv );

	xml = glade_xml_new("compradio.glade", NULL, NULL);

	glade_xml_signal_autoconnect(xml);

	gtk_main();

	return 0;
}

gboolean main_window_destroy( GtkObject *object,
			      gpointer user_data )
{
	gtk_main_quit();
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
		printf("Pinging enabled\n");
	} else {
		printf("Pinging disabled\n");
	}
}
