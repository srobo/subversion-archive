function Switchboard()
{
	//hold the tab object
	this.tab = null;

	//hold signals for the page
	this._signals = new Array();

	//hold status message for the page
	this._prompt = null;

	/* Initialize a new tab for switchboard - Do this only once */
	logDebug("Switchboard: Initializing");
	this.tab = new Tab( "Switchboard" );
	connect( this.tab, "onfocus", bind( this._onfocus, this ) );
	connect( this.tab, "onblur", bind( this._onblur, this ) );
	connect( this.tab, "onclickclose", bind( this._close, this ) );
	tabbar.add_tab( this.tab );

	//hold the dictionary of timeline events
	this.events = null;

}

Switchboard.prototype.changeEvent = function(id)
{
	$("timeline-description").innerHTML = "<strong>"+this.events[id].title+": </strong>"+this.events[id].desc;
}
/* Tab events */
Switchboard.prototype._onfocus = function()
{
	setStyle($("switchboard-page"), {'display':'block'});
	this.GetMessages();	//TODO: Only do this if we are focing a reload
	this.GetTimeline();
}

Switchboard.prototype._onblur = function()
{
	setStyle($("switchboard-page"), {'display':'none'});
}

Switchboard.prototype._close = function()
{
	setStyle($("switchboard-page"), {'display':'none'});
}
/* End Tab events */

/* Message Feed code */ 
Switchboard.prototype.receiveMessages = function(nodes)
{
/*	message-list
 *		|----li ('t')
 *			| ---- Blog title link ('a')
 *			| ---- UL ('u')
 *				| ---- LI ('l')
 *					| ----  Blog post body
 */
	// TODO: Remove existing messages before adding new ones
	for(m in nodes.msgs)
	{
		item = nodes.msgs[m];
		var a = A({'href':item.link}, item.title);		
		var t = LI({},"");				
		appendChildNodes(t, a);				
		var u = UL({},"");					
		var l = LI({'class':'null'},"");			
		l.innerHTML = item.body+" [by "+item.author+"]";
		u = appendChildNodes(u, l);		
		appendChildNodes(t, u);
		var p = appendChildNodes($("message-list"),t);	
	}
}

Switchboard.prototype.errorReceiveMessages = function()
{
	status_msg("Unable to load messages", LEVEL_ERROR);
	logDebug("Switchboard: Failed to retrieve messages");
}

Switchboard.prototype.GetMessages = function()
{
	logDebug("Switchboard: Retrieving SR message feed");
	var d = loadJSONDoc("../switchboard/getmessages", {});

	d.addCallback( bind(this.receiveMessages, this));
	d.addErrback( bind(this.errorReceiveMessages, this));
}

/* Timeline Code */
Switchboard.prototype.receiveTimeline = function(nodes)
{
/*	Overview: build the timeline showing key milestones
 *	Description: Each milestone event is converted into a <div>
 *	with an offset from the parent proportional to its date.
 */
	logDebug("Generating Timeline..");		

	/* Store the events in object */
	this.events = nodes.events;

	/* Date manipulation */
	var start_date = new Date(nodes.start);
	logDebug("Timeline start: "+ start_date);
	var end_date = new Date(nodes.end);
	logDebug("Timeline end: "+ end_date);
	var duration = end_date - start_date;
	logDebug("Timeline Duration: "+duration);	

	/* get the maximum progress bar width in pixels */
	var bar_width = rstrip(getStyle($("timeline-bar-out"), 'width'), "px");

	/* Convert a date into a pixel offset */
	function getOffset(event_date) 
		{
			d = new Date(event_date);
			o = Math.floor(((d - start_date)/duration)*bar_width)+"px";
			return o;
		}
	
	/* set the progress bar width */
	var today = new Date();
	if(today < start_date)
	{
		//not yet at timeline - default to arbitrary date 
		today = new Date("November 12, 2009");
	}
	else if(today > end_date)
	{
		//past the end, default to 100%
		today = end_date;
	}
	setStyle($("timeline-bar-in"), {'width': Math.floor(((today-start_date)/duration)*bar_width)+"px"});
	

	/* Add the events */
	for(m in nodes.events)
	{	/* create and position a new <div> for each timeline event */
		var e = DIV({"class":"timeline-bar-event", 
				"id":"timeline-ev-"+m,
				"title":nodes.events[m].title}, "");
		connect( e, "onmouseover", bind( this.changeEvent, this, m) );
		appendChildNodes($("timeline-bar-in"), e);
		setStyle(e,
			{'margin-left': getOffset(nodes.events[m].date)});
	}
}

Switchboard.prototype.errorReceiveTimeline = function()
{
	status_msg("Unable to load timeline", LEVEL_ERROR);
	logDebug("Switchboard: Failed to load timeline data");
}

Switchboard.prototype.GetTimeline = function()
{
	logDebug("Switchboard: Retrieving SR timeline");
	var d = loadJSONDoc("../switchboard/timeline", {});

	d.addCallback( bind(this.receiveTimeline, this));
	d.addErrback( bind(this.errorReceiveTimeline, this));
}

