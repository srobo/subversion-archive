#include "xbee-conn.h"

static void xbee_conn_instance_init (GTypeInstance *gti, gpointer g_class );

GType xbee_client_get_type (void )
{
  
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeConnClass),
			NULL,			/* base_init */
			NULL, 			/* base_finalise */
			NULL, 			/* class_init */
			NULL, 			/* class_finalise */
			NULL, 			/* class_data */
			sizeof (XbeeConn),
			0,			/* n_preallocs */
			xbee_conn_instance_init 	/* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT, 
					       "XbeeConnType", 
					       &info, 0);

	}

	return (type);
}

gboolean xbee_conn_transmit( XbeeConn* xbc, xb_addr_t addr, uint8_t* data, uint16_t len )
{
	g_print( "Transmit!\n" );
	return TRUE;
}

XbeeConn *xbee_conn_new ( char * addr, GMainContext *context )
{

}

static void xbee_conn_instance_init (GTypeInstance *gti, gpointer g_class )
{

}
