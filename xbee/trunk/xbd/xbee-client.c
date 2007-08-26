#include "xbee-client.h"
#include "commands.h"
#include "libxcli.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class );
static gboolean xbee_client_sock_incoming( XbeeClient *client );
static gboolean xbee_client_sock_outgoing( XbeeClient *client );
static gboolean xbee_client_sock_error( XbeeClient *client );

/* Returns TRUE if data is ready to be transmitted */
static gboolean xbee_client_data_ready( XbeeClient *client );

/* Reads available bytes from the input.
 * When a full frame is achieved, returns 0.
 * When a full frame has not been acheived, it returns 1.
 * When an error occurs, it returns -1 */
static int xbee_client_read_frame( XbeeClient *client );

static void xbee_client_dispose( GObject *obj );
static void xbee_client_finalize( GObject *obj );
static void xbee_client_class_init( XbeeClientClass *klass );

static GObjectClass *parent_class = NULL;

/* Write frame to FD */
static int xbee_client_write_frame ( XbeeClient *client );

GType xbee_client_get_type( void )
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (XbeeClientClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc)xbee_client_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (XbeeClient),
			0,      /* n_preallocs */
			xbee_client_instance_init    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT,
					       "XbeeClientType",
					       &info, 0);
	}
	return type;
}

static void xbee_client_instance_init( GTypeInstance *gti, gpointer g_class )
{
	XbeeClient *client = (XbeeClient*)gti;
	assert( gti != NULL );

	client->in_frames = g_queue_new();
	client->out_frames = g_queue_new();
	client->inpos = 0;
	client->outpos = 0;
	client->server = NULL;
	client->dispose_has_run = FALSE;

	parent_class = g_type_class_peek_parent (g_class);

	g_print( "Wooo.  Client initialised\n" );
}

XbeeClient* xbee_client_new( GMainContext *context, 
			     int sock, 
			     XbeeServer *server,
			     xbee_client_disconnect_t dis )
{
	XbeeClient* client;
	assert( server != NULL && dis != NULL );
	
	client = g_object_new( XBEE_CLIENT_TYPE, NULL );

	client->fd = sock;
	client->disconn = dis;
	client->server = server;

	client->source = xbee_fd_source_new( client->fd,
					     context,
					     (gpointer)client,
					     (xbee_fd_callback)xbee_client_sock_incoming,
					     (xbee_fd_callback)xbee_client_sock_outgoing,
					     (xbee_fd_callback)xbee_client_sock_error,
					     (xbee_fd_callback)xbee_client_data_ready );

	return client;
}

static int xbee_client_read_frame( XbeeClient *client )
{
	int r;
	uint8_t d;
	gboolean whole_frame = FALSE;
	assert( client != NULL );

	while( !whole_frame )
	{
		r = read( client->fd, &d, 1 );
		
		if( r == -1 )
		{
			if( errno == EAGAIN )
				break;
			fprintf( stderr, "Error: Failed to read client input: %m\n" );
			return -1;
		}

		if( r == 0 ) {
			client->disconn( client, client->server );
			return 1;
		}


		client->inbuf[ client->inpos ] = d;
		client->inpos++;

		/* Calculate frame length */
		if( client->inpos == 2 )
		{
			client->flen = (((uint16_t)client->inbuf[0]) << 8) | client->inbuf[1];
			if( client->flen + 2 > XBEE_MAX_FRAME )
			{
				printf( "Frame too long (%u) :-S\n", client->flen );
				return -1;
			}
		}
		
		if( client->inpos == client->flen + 2 ) 
			whole_frame = TRUE;
	}

	if( !whole_frame )
		return 1;
	return 0;		/* Whole frame */
}

static gboolean xbee_client_sock_incoming( XbeeClient *client )
{
	assert( client != NULL );

	while( xbee_client_read_frame( client ) == 0 )
	{
		//g_debug( "xbee_client: Frame received.\n" );

		if( client->flen > 0 )
		{
			uint8_t *f = &client->inbuf[2];
			switch( f[0] )
			{
			case XBEE_COMMAND_TEST:
			{
				char* c, *s = (char*)f+1;
				int len = client->flen - 1;

				printf( "Received: " );
				for( c=s; c-s < len; c++ )
					putc( (int)*c, stdout );
				putc( (int)'\n', stdout );
			}
			case XBEE_COMMAND_TRANSMIT:
			{
				/* Transmit Frame Layout 
				   0: Command Code: XBEE_COMMAND_TRANSMIT
				   * 1: Address type
				   *** 16-bit address format:
				   *     2-3: Address - MSB first.
				   *     4->(3+len): Data for transmission.
				   *** 64-bit address format:
				   *     2-9: Address - MSB first
				   *     10->(9+len): Data for transmission */

				uint8_t len;
			        xb_addr_t addr;
				uint8_t* data;
				addr.type = f[1];
				
				if (addr.type == XB_ADDR_16)
				{
					memmove (addr.addr, &f[2], 2);
					len = client->flen - 4;
					data = &f[4];
				}
				else
				{
					memmove (addr.addr, &f[2], 8);
					len = client->flen - 10;
					data = &f[10];
				}

				xbee_server_transmit (client->server, &addr, data, len);
					
			}
			break;
			}
		}

		/* Discard frame - it's been processed */
		client->inpos = client->flen = 0;
	}

	return TRUE;
}

static gboolean xbee_client_sock_outgoing( XbeeClient *client )
{
	assert( client != NULL );
	
	int ret_val = 1;
	
	while ( xbee_client_data_ready (client) && (ret_val == 1))
	{
		ret_val = xbee_client_write_frame ( client );
		if (ret_val == -1)
			return FALSE;
	}

	return TRUE;
}

static gboolean xbee_client_data_ready( XbeeClient *client )
{
	assert (client != NULL);
	
	if ( g_queue_peek_tail ( client->out_frames ) == NULL )
		return FALSE;

	return TRUE;
}

static gboolean xbee_client_sock_error( XbeeClient *client )
{
	fprintf( stderr, "Error with client socket.  Handle this...\n" );
	return FALSE;
}

static void xbee_client_class_init( XbeeClientClass *klass )
{
	assert( klass != NULL );
	GObjectClass *gobject_class = G_OBJECT_CLASS( klass );

	gobject_class->dispose = xbee_client_dispose;
	gobject_class->finalize = xbee_client_finalize;	
}

static void xbee_client_dispose( GObject *obj )
{
	XbeeClient *self = (XbeeClient*)obj;

	if( self->dispose_has_run ) 
		return;

	self->dispose_has_run = TRUE;

	/* Unreference things here. */

	G_OBJECT_CLASS(parent_class)->dispose(obj);
}

static void xbee_client_finalize( GObject *obj )
{
	XbeeClient *self = (XbeeClient*)obj;

	/* Free stuff */
	/* Free the queues etc. */

	/* Delete the xb-fd-source */
	xbee_fd_source_free( self->source );
	
	G_OBJECT_CLASS (parent_class)->finalize (obj);	
}

void xbee_client_transmit ( XbeeClient *client, uint8_t *data, xb_rx_info_t *info, uint8_t len)
{
	
	xb_frame_t *frame;
	frame = (xb_frame_t*)g_malloc (sizeof (xb_frame_t) );
	assert (client != NULL && data != NULL && info != NULL && frame != NULL);
	
	uint8_t broadcast;

	/* Frame Structure */
	/* Byte: */
	/* 0: address type */
	/* 16Bit: */
	/* 1-2: Source Address */
	/* 3: RSSI */
	/* 4: PAN_BROADCAST */
	/* 5: ADDRESS_BROADCAST */
	/* 6-(len-1): RX data */
	/* 64Bit */
	/* 1-8: Source Address */
	/* 9: RSSI */
	/* 10: PAN_BROADCAST */
	/* 11: ADDRESS_BROADCAST */
	/* 12-(len-1): RX data */
	
	
	if (info->src_addr.type == XB_ADDR_16)
	{
		frame->data = (uint8_t*) g_malloc ( (len + 8) * sizeof (uint8_t) );
		g_memmove ( &frame->data[0], &info->src_addr.type, 1);
		g_memmove ( &frame->data[1], &info->src_addr.addr, 2);
		g_memmove ( &frame->data[3], &info->rssi, 1);
		broadcast = (info->pan_broadcast) ? 1 : 0;
		g_memmove ( &frame->data[4], &broadcast, 1);
		broadcast = info->address_broadcast ? 1 : 0;
		g_memmove ( &frame->data[5], &broadcast, 1);
		frame->data[6] = info->src_channel;
		frame->data[7] = info->dst_channel;
		g_memmove ( &frame->data[8], data, len );

		len = len + 8;
	}
	else
	{
		frame->data = (uint8_t*) g_malloc ( (len + 14) * sizeof (uint8_t) );
		g_memmove ( &frame->data[0], &info->src_addr.type, 1);
		g_memmove ( &frame->data[1], &info->src_addr.addr, 8);
		g_memmove ( &frame->data[9], &info->rssi, 1);
		broadcast = (info->pan_broadcast) ? 1 : 0;
		g_memmove ( &frame->data[10], &broadcast, 1);
		broadcast = info->address_broadcast ? 1 : 0;
		g_memmove ( &frame->data[11], &broadcast, 1);
		frame->data[12] = info->src_channel;
		frame->data[13] = info->dst_channel;
		g_memmove ( &frame->data[14], data, len);
		len = len + 14;
	}

	frame->len = len;

	g_queue_push_head ( client->out_frames, frame );	
	printf ("Frame Pushed\n");

	/* Data (in queue) ready to write */
	xbee_fd_source_data_ready ( client->source );
	
}

static int xbee_client_write_frame ( XbeeClient *client )
{
	xb_frame_t *frame;
	assert (client != NULL && client->out_frames != NULL);

	frame = (xb_frame_t*)g_queue_peek_tail (client->out_frames);
	assert (frame != NULL);

	while ( client->outpos < (frame->len + 2) )
	{
		
		int b = 0;

		if ( client->outpos < 2)
		{
			uint8_t flen[2];

			/* MSByte for frame length 1st */
			flen[0] = (frame->len >> 8 ) & 0xFF;
			flen[1] = frame->len & 0xFF;

			/* Write Frame Length */
			b = write ( client->fd, &flen [client->outpos], 2 - client->outpos );
		}
		else
			/* Write Frame Data */
			b = write ( client->fd, &frame->data [ client->outpos - 2], frame->len + 2 - client->outpos );

		/* If error occurs */
		if (b == -1)
		{
			if ( errno == EAGAIN )
				return 0;
			
			fprintf ( stderr, "Error: Failed to write %m\n" );
			return -1;
		}
		
		/* do not do anything in this iteration if nothing written */
		if (b==0)
			continue;
		
		client->outpos += b;
	}


	
	g_queue_pop_tail ( client->out_frames );

	g_free (frame->data);
	g_free (frame);

	client->outpos = 0;
	//g_debug ("Client: Frame written");
	return 1;
}
