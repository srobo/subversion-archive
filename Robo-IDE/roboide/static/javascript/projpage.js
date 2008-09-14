// ***** The Project Page *****
var proj_ps_prompt = null;

function ProjPage() {
	this._initted = false;
}

// Initialise the project page -- but don't show it
ProjPage.prototype._init = function() {
	if( this._initted )
		return;
	
	MochiKit.Signal.connect( "project-select", "onchange", bind(this._list_changed, this) );
	this._initted = true;
}

ProjPage.prototype.show = function() {
	this._init();
	
	// Hide the right-hand whilst the file list is loading
	this.rpane_hide();
	
	// If we have a project setup
	if (project != "")
		this._flist();
	
	// Load/refresh the projects list
	this._populate_list();
	
	MochiKit.Style.setStyle('projects-page', {'display':'block'});
}

ProjPage.prototype.hide = function() {
	MochiKit.Style.setStyle('projects-page', {'display':'none'});
}

// Change project
ProjPage.prototype.change_project = function(proj) {
	project = proj;
	
	this._flist();
}

// Retrieves a list of projects and populates the project selection list
ProjPage.prototype._populate_list = function() {
	var d = MochiKit.Async.loadJSONDoc("./projlist", {team : team});
	
	d.addCallback( function(resp) {
		var items = [];
		
		if( project == "" )
			items.unshift( MochiKit.DOM.OPTION( { "id" : "projlist-tmpitem" }, "Select a project." ) );
		
		for( var p in resp["projects"] ) {
			var pname = resp["projects"][p];
			var props = {};
			
			if( pname == project )
				props["selected"] = "selected";
			items[items.length] = ( MochiKit.DOM.OPTION( props, resp["projects"][p] ) );
		}
		
		MochiKit.DOM.replaceChildNodes( "project-select", items );
		
		if( project == "" )
			proj_ps_prompt = status_msg( "Please select a project", LEVEL_INFO );
	} );
	
	d.addErrback( function() {
		status_button( "Error retrieving the project list", LEVEL_ERROR,
			       "retry", bind( this._populate_list, this) );
	} );
}

ProjPage.prototype._list_changed = function(ev) {
	proj_ps_prompt.close();
	
	var src = ev.src();
	
	// Remove the "select a project" item from the list
	var tmp = MochiKit.DOM.getElement( "projlist-tmpitem" );
	if( tmp != null && src != tmp )
		MochiKit.DOM.removeElement( tmp );
	
	projpage.change_project(ev.src().value);
}

// ***** Project Page Right Hand pane *****
ProjPage.prototype.rpane_hide = function() {
	map_1( MochiKit.Style.setStyle,
	       {'display':'none'},
	       ["proj-rpane-header", "proj-filelist"] );
}

ProjPage.prototype.rpane_show = function() {
	map_1( MochiKit.Style.setStyle, 
	       {'display':''},
	       ["proj-rpane-header", "proj-filelist"] );
}

// ***** Project Page File Listing *****
// Request and update the project file listing
ProjPage.prototype._flist = function() {
	var d = MochiKit.Async.loadJSONDoc("./filelist", {team : 1,
		rootpath : project});
	
	d.addCallback( bind( this._flist_received, this ) );
	
	d.addErrback(function (){
		status_button( "Error getting the file list", LEVEL_ERROR,
			       "retry", bind( this._flist, this ) );
	});
}

ProjPage.prototype._flist_received = function(nodes) {
	log( "filelist received" );
	
	MochiKit.DOM.swapDOM( "proj-filelist",
			      MochiKit.DOM.UL( { "id" : "proj-filelist",
						 "style" : "display:none" },
					       map_1( bind(this._flist_dir, this), 0, nodes["tree"] ) ) );
	
	MochiKit.DOM.getElement( "proj-name" ).innerHTML = "Project " + project;
	
	projpage.rpane_show();
}

function map_1( func, arg, arr ) {
	var a = function( item ) {
		return func( item, arg );
	}
	
	return MochiKit.Base.map( a, arr );
}

// Produce an object consisted of "level" levels of nested divs
// the final div contains the DOM object inner 
ProjPage.prototype._flist_nested_divs = function( level, inner ) {
	if (level == 0)
		return inner;
	
	if (level > 1)
		return MochiKit.DOM.DIV( null, this._flist_nested_divs( level-1, inner ) );
	
	return MochiKit.DOM.DIV( null, inner );
}

// Returns a DOM object for the given node 
ProjPage.prototype._flist_dir = function( node, level ) {
	// Assemble the link with divs in it
	var link = MochiKit.DOM.A( { "href" : "#" },
		this._flist_nested_divs( level, node.name + (node.kind == "FOLDER"?"/":"") ) );
	MochiKit.Signal.connect( link, "onclick", bind( this._flist_onclick, this ) );
	
	if( node.kind == "FILE" ) {
		var n = MochiKit.DOM.LI( null, link );
		return n;
	} else
		var n = MochiKit.DOM.LI( null, [ link, 
	    		MochiKit.DOM.UL( { "class" : "flist-l" }, 
			map_1( bind(this._flist_dir, this), level + 1, node["children"] ) ) ] );
	return n;
}

// The onclick event for the filelist items
ProjPage.prototype._flist_onclick = function(ev) {
	// Prevent the browser doing something when someone clicks on this
	ev.preventDefault();
	ev.stopPropagation();
	
	mods = ev.modifier();
	if( mods["ctrl"] )
		MochiKit.DOM.toggleElementClass( "selected", ev.src() );
}
