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

}

/* Tab events */
Switchboard.prototype._onfocus = function()
{
	setStyle($("switchboard-page"), {'display':'block'});
	this.GetMessages();
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
	
	/* get the maximum progress bar width in pixels */
	var bar_width = rstrip(getStyle($("timeline-bar-out"), 'width'), "px");

	/* set the progress bar width accordingly*/
	$("timeline-bar-in").style.width = nodes.progress+"%";
	
	/* Add the events */
	for(m in nodes.dates)
	{	/* create and position a new <div> for each timeline event */
		var e = DIV({"class":"timeline-bar-event", 
				"id":"timeline-ev-"+m,
				"title":nodes.dates[m].title}, "");
		logDebug(Math.floor((nodes.dates[m].date/100)*bar_width)+"px");
		appendChildNodes($("timeline-bar-in"), e);
		setStyle(e,
			{'margin-left': Math.floor((nodes.dates[m].date/100)*bar_width)+"px"});
	}
}

Switchboard.prototype.errorReceiveTimeline = function()
{
	logDebug("Switchboard: Failed to load timeline data");
}

Switchboard.prototype.GetTimeline = function()
{
	logDebug("Switchboard: Retrieving SR message feed");
	var d = loadJSONDoc("../switchboard/timeline", {});

	d.addCallback( bind(this.receiveTimeline, this));
	d.addErrback( bind(this.errorReceiveTimeline, this));
}

