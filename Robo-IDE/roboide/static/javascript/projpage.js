// The project page
function ProjPage() {
	// Whether _init has run
	this._initted = false;
	// The prompt 
	this._ps_prompt = null;
	// The projects
	this._projects = null;

	// The signals that we've set up for this object
	// signals that are for DOM objects that weren't created in this instance
	this._connections = [];

	this.flist = null;
	this.project = "";

	// Member functions (declared below)
	// Public:
	//  - show: Show and activate the projects page
	//  - hide: Hide the project page
	//  - change_project: Change to the named project
	// Private:
	//  - _init: Initialises members of the project page
	//  - _populate_list: Retries the list of projects from the server
	//                    and populates it
	//  - _got_list: Handler for when the project list has been received 
	//  - _list_changed: Event handler for when the project list selection is 
	//                   changed 
	//  - _rpane_show: Show the right-hand pane
	//  - _rpane_hide: Hide the right-hand pane
	//  - _project_exists: Returns true if the given project name exists
}

// Initialise the project page -- but don't show it
ProjPage.prototype._init = function() {
	if( this._initted )
		return;

	this._connections.push( connect( "project-select", "onchange", bind(this._list_changed, this) ) );

	this.flist = new ProjFileList();
	this._initted = true;
}

ProjPage.prototype.destroy = function() {
	logDebug( "ProjPage.destroy()" );
	if( !this._initted )
		return;
	this._initted = false;
	this.project = "";
	this._ps_prompt = null;
	this.flist = null;

	// Disconnect all signals to static things
	map( disconnect, this._connections );
	this._connections = []
}

ProjPage.prototype.show = function() {
	logDebug( "Projpage.show: Current project is \"" + this.project + "\"" );
	this._init();
	
	// Hide the right-hand whilst the file list is loading
	this._rpane_hide();
	
	// Load/refresh the projects list
	this._populate_list();

	setStyle('projects-page', {'display':'block'});
}

ProjPage.prototype.hide = function() {
	setStyle('projects-page', {'display':'none'});
}

// Change project
ProjPage.prototype.change_project = function(proj) {
	if( !this._project_exists(proj) ) {
		logDebug( "Change requested to non-existent project \"" + proj + "\"" );
		return;
	}
	logDebug("Changing project to " + proj);
	this.project = proj;
	
	this.flist.update( this.project );
}

// Retrieves a list of projects and populates the project selection list
ProjPage.prototype._populate_list = function() {
	var d = loadJSONDoc("./projlist", {team : team});
	
	d.addCallback( bind( this._got_list, this ) );
	
	d.addErrback( bind( function() {
		status_button( "Error retrieving the project list", LEVEL_ERROR,
			       "retry", bind( this._populate_list, this) );
	}, this ) );
}

// Called once we've received the project list
ProjPage.prototype._got_list = function(resp) {
	var items = [];
	this._projects = resp["projects"];

	if( this._projects.length == 1 ) {
		this.project = this._projects[0];
		this.change_project( this.project );
	}

	if( !this._project_exists(this.project) ) {
		var dp = user.get_setting( "project.last" );

		if( dp != undefined
		    && this._project_exists( dp ) )
			this.change_project( dp );
		else {
			this.project = "";
			this._ps_prompt = status_msg( "Please select a project", LEVEL_INFO );
			items.unshift( OPTION( { "id" : "projlist-tmpitem", 
						 "selected" : "selected" }, "Select a project." ) );
		}
	}

	if( this._project_exists(this.project) )
		logDebug( "_got_list: Using project " + this.project );
	else
		logDebug( "No default project found -- requesting selection" );
		
	for( var p in resp["projects"] ) {
		var pname = resp["projects"][p];
		var props = { "value" : resp["projects"][p] };
			
		if( pname == this.project )
			props["selected"] = "selected";
		items[items.length] = ( OPTION( props, resp["projects"][p] ) );
	}

	replaceChildNodes( "project-select", items );
}

ProjPage.prototype._list_changed = function(ev) {
	if( this._ps_prompt != null ) {
		this._ps_prompt.close();
		this._ps_prompt = null;
	}
	
	var src = ev.src();
	
	// Remove the "select a project" item from the list
	var tmp = $("projlist-tmpitem");
	if( tmp != null && src != tmp )
		removeElement( tmp );
	
	if( src != tmp ) {
		var proj = src.value;
		projpage.change_project(proj);
	}
}

// ***** Project Page Right Hand pane *****
ProjPage.prototype._rpane_hide = function() {
	map_1( setStyle,
	       {'display':'none'},
	       ["proj-rpane-header", "proj-filelist"] );
}

ProjPage.prototype._rpane_show = function() {
	map_1( setStyle, 
	       {'display':''},
	       ["proj-rpane-header", "proj-filelist"] );
}

ProjPage.prototype._project_exists = function(pname) {
	for( i in this._projects ) {
		if( this._projects[i] == pname ) {
			return true;
		}
	}
	return false;
}

// ***** Project Page File Listing *****
function map_1( func, arg, arr ) {
	var a = function( item ) {
		return func( item, arg );
	}
	
	return map( a, arr );
}

// Project page file list
function ProjFileList() {
	this._project = "";
	// Member functions:
	// Public:
	//  - update: Update the file list to the given project
	// Private:
	//  - _received: handler for receiving the file list
	//  - _nested_divs: Returns N nested divs.
	//  - _dir: Returns the DOM object for a directory entry
	//  - _onclick: The onclick handler for a line in the listing
}

// Request and update the project file listing
ProjFileList.prototype.update = function( pname ) {
	var d = loadJSONDoc("./filelist", {team : team,
					   rootpath : pname});
	this._project = pname;
	
	d.addCallback( bind( this._received, this ) );
	
	d.addErrback( bind( function (){
		status_button( "Error getting the project file list", LEVEL_ERROR,
			       "retry", bind( this.update, this ) );
	}, this ) );
}

// Handler for receiving the file list 
ProjFileList.prototype._received = function(nodes) {
	log( "filelist received" );
	
	swapDOM( "proj-filelist",
		 UL( { "id" : "proj-filelist",
		       "style" : "display:none" },
		     map_1( bind(this._dir, this), 0, nodes["tree"] ) ) );
	
	getElement( "proj-name" ).innerHTML = "Project " + this._project;
	
	projpage._rpane_show();
}


// Produce an object consisted of "level" levels of nested divs
// the final div contains the DOM object inner 
ProjFileList.prototype._nested_divs = function( level, inner ) {
	if (level == 0)
		return inner;
	
	if (level > 1)
		return DIV( null, this._nested_divs( level-1, inner ) );
	
	return DIV( null, inner );
}

// Returns a DOM object for the given node 
ProjFileList.prototype._dir = function( node, level ) {
	// Assemble the link with divs in it
	var link = A( { "href" : "#",
			"ide_path" : node.path,
			"ide_kind" : node.kind },
		this._nested_divs( level, node.name + (node.kind == "FOLDER"?"/":"") ) );
	connect( link, "onclick", bind( this._onclick, this ) );
	
	if( node.kind == "FILE" ) {
		var n = LI( null, link );
		return n;
	} else
		var n = LI( null, [ link, 
	    		UL( { "class" : "flist-l" }, 
			map_1( bind(this._dir, this), level + 1, node["children"] ) ) ] );
	return n;
}

// The onclick event for the filelist items
ProjFileList.prototype._onclick = function(ev) {
	// Prevent the browser doing something when someone clicks on this
	ev.preventDefault();
	ev.stopPropagation();
	
	mods = ev.modifier();
	if( mods["ctrl"] )
		toggleElementClass( "selected", ev.src() );
	else {
		src = ev.src();
		kind = getNodeAttribute( src, "ide_kind" );
		path = getNodeAttribute( src, "ide_path" );

		if( kind == "FOLDER" ) {
			// Get a handler on its children
			dir_contents = getFirstElementByTagAndClassName( "UL", null, src.parentNode ); 

			display = "";
			if( getStyle( dir_contents, "display" ) != "none" )
				display = "none";

			setStyle( dir_contents, {"display" : display} );
		}

	}
}

