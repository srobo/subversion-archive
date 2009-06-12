// The simulator page
function SimPage() {
	// Whether _init has run
	this._initted = false;
}

// Initialise the simulator page -- but don't show it
SimPage.prototype._init = function() {
	if( this._initted )
		return;
	logDebug( "Simulator page initialised" );
	this._initted = true;
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
