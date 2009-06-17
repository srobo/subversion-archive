// The simulator page
function SimPage() {
	//the tab for the page
	this.tab = null;
	// Whether _init has run
	this._initted = false;
}

// Initialise the simulator page -- but don't show it
SimPage.prototype._init = function() {
	if( this._initted )
		return;

	this.tab = new Tab( "Simulator" );
	connect( this.tab, "onfocus", bind( simpage.show, simpage ) );
	connect( this.tab, "onblur", bind( simpage.hide, simpage ) );
	tabbar.add_tab( this.tab, {super:true} );

	logDebug( "Simulator page initialised" );
	this._initted = true;
}

// Load a simulation
SimPage.prototype.load = function(project) {
	this._init();
	tabbar.switch_to(this.tab);
	logDebug( "Simulating the "+project+" proejct" );
}

// Unhide the simulator page
SimPage.prototype.show = function() {
	logDebug( "Showing the simulator page" );
	this._init();
	setStyle('simulator-page', {'display':'block'});
}

// Hide the simulator page
SimPage.prototype.hide = function() {
	logDebug( "Hiding the simulator page" );
	setStyle('simulator-page', {'display':'none'});
}
