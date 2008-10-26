function ErrorsTab(contents) {
	this.tab = null;

	this._init(contents);	
}

ErrorsTab.prototype._init = function(contents) {
	this.tab = new Tab( "Errors" );
	tabbar.add_tab( this.tab );

	connect( this.tab, "onfocus", bind( this._onfocus, this ) );
	connect( this.tab, "onblur", bind( this._onblur, this ) );

	replaceChildNodes( "errors-listing", contents );
}

ErrorsTab.prototype._onfocus = function() {
	setStyle( $("errors-page"), {"display":"block"} );
}

ErrorsTab.prototype._onblur = function() {
	setStyle( $("errors-page"), {"display":"none"} );
}