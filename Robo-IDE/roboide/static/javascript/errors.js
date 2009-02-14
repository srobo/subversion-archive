function ErrorsTab(contents) {
	this.tab = null;
	this.signals = new Array();
	this._init(contents);
	this.lines = new Array();
	this.files = new Array();
	this.messages = new Array();
}
var global;
ErrorsTab.prototype._init = function(contents) {
	this.tab = new Tab( "Errors" );
	connect( this.tab, "onfocus", bind( this._onfocus, this ) );
	connect( this.tab, "onblur", bind( this._onblur, this ) );

	tabbar.add_tab( this.tab );

	this.signals.push(connect("close-errors-page", "onclick", bind(this._close, this) ));
	this.signals.push(connect("switch-errors-page", "onclick", bind(this._switch, this) ));

	for(var i = 0; i < contents.messages.length; i++)
	{
		var item = LI({"errno" : i}, contents.messages[i]);
		this.signals.push(connect(item, "onclick", bind(this._select, this)));
		appendChildNodes("errors-listing", item);
	}

}

ErrorsTab.prototype._onfocus = function() {
	setStyle( $("errors-page"), {"display":"block"} );
}

ErrorsTab.prototype._onblur = function() {
	setStyle( $("errors-page"), {"display":"none"} );
}

ErrorsTab.prototype._close = function() {
	for(var i = 0; i < this.signals; i++)
	{
		disconnect(this.signals[i]);
	}
	this.tab.close();
	delete this;
}

ErrorsTab.prototype._switch = function(filename) {
	for(var i=0; i < tabbar.tabs.length; i++)
	{
		if(tabbar.tabs[i].label == "robot.py")
		{
			tabbar.switch_to(tabbar.tabs[i]);
			return;
		}
	}
	status_msg("Couldn't switch to selected file: "+filename, LEVEL_ERROR);
	return;
}

ErrorsTab.prototype._select = function()
{
	status_msg("clicked", LEVEL_OK);
	return;
}
