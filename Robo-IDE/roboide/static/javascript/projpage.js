// The project page
function ProjPage() {
	// Whether _init has run
	this._initted = false;

	// The signals that we've set up for this object
	// signals that are for DOM objects that weren't created in this instance
	this._connections = [];

	// The project list -- a ProjList instance
	this._list = null;
	// The project selector -- a ProjSelect instance
	this._selector = null;

	this.flist = null;
	this.project = "";

	// Member functions (declared below)
	// Public:
	//  - show: Show and activate the projects page
	//  - hide: Hide the project page
	//  - change_project: Change to the named project
	// Private:
	//  - _init: Initialises members of the project page
	//  - _rpane_show: Show the right-hand pane
	//  - _rpane_hide: Hide the right-hand pane
	//  - _project_exists: Returns true if the given project name exists
	//  - _on_proj_change: Handler for when the selected project changes
	//                     Hides/shows the right-hand pane as necessary
}

// Initialise the project page -- but don't show it
ProjPage.prototype._init = function() {
	if( this._initted )
		return;

	// Hide the right-hand whilst we're loading
	this._rpane_hide();

	// The list of projects
	this._list = new ProjList();

	// The selection box for selecting a project
	this._selector = new ProjSelect(this._list, $("project-select"));
	connect( this._selector, "onchange", bind( this._on_proj_change, this ) );

	this.flist = new ProjFileList();
	// Update the file list when the project changes
	connect( this._selector, "onchange", bind( this.flist.update, this.flist ) );

	// We have to synthesize the first "onchange" event from the ProjSelect,
	// as these things weren't connected to it when it happened
	this._on_proj_change( this._selector.project );
	this.flist.update( this._selector.project );

	this._initted = true;
}

ProjPage.prototype.show = function() {
	logDebug( "Projpage.show: Current project is \"" + this.project + "\"" );
	this._init();
	
	setStyle('projects-page', {'display':'block'});
}

ProjPage.prototype.hide = function() {
	logDebug( "Hiding the projects page" );
	setStyle('projects-page', {'display':'none'});
}

ProjPage.prototype._on_proj_change = function(proj, team) {
	logDebug( "ProjPage._on_proj_change(\"" + proj + "\", " + team + ")" );
	this.project = proj;

	if( proj == "" )
		this._rpane_hide();
	else {
		$("proj-name").innerHTML = "Project " + this.project;
		this._rpane_show();
	}
}

ProjPage.prototype.set_team = function(team) {
	logDebug( "ProjPage.set_team( " + team + " )" );
	if( team == null || team == 0 )
		return;

	this._init();

	// Start the chain of updates
 	this._list.update(team);
	// The selector and filelist are connected to onchange on the list,
	// so they will update when it's updated
}

// ***** Project Page Right Hand pane *****
ProjPage.prototype._rpane_hide = function() {
	setStyle( "proj-rpane", {'display':'none'} );
}

ProjPage.prototype._rpane_show = function() {
	setStyle( "proj-rpane", {'display':''} );
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
	this._team = null;
	// Member functions:
	// Public:
	//  - update: Update the file list to the given project and team
	// Private:
	//  - _received: handler for receiving the file list
	//  - _nested_divs: Returns N nested divs.
	//  - _dir: Returns the DOM object for a directory entry
	//  - _onclick: The onclick handler for a line in the listing
	//  - _hide: Hide the filelist
	//  - _show: Show the filelist
}

// Request and update the project file listing
ProjFileList.prototype.update = function( pname, team ) {
	logDebug( "ProjFileList.update( \"" + pname + "\", " + team + " )" );
	if( pname == "" ) {
		// No project selected.
		this._hide();
		return;
	}

	if( pname != this._project || team != this._team ) {
		// Hide the list whilst we're loading it
		swapDOM( "proj-filelist",
			 DIV( {"id": "proj-filelist", 
			       "class" : "loading"}, 
			      "Loading project file listing..." ) );
	}

	this._project = pname;
	this._team = team;

	var d = loadJSONDoc("./filelist", {team : this._team,
					   rootpath : pname});
	
	d.addCallback( bind( this._received, this ) );
	
	d.addErrback( bind( function (){
		status_button( "Error retrieving the project file listing", LEVEL_ERROR,
			       "retry", bind( this.update, this ) );
	}, this ) );
}

ProjFileList.prototype._hide = function() {
	setStyle( "proj-filelist", {"display":"none"} );
}

ProjFileList.prototype._show = function() {
	setStyle( "proj-filelist", {"display":""} );
}

// Handler for receiving the file list 
ProjFileList.prototype._received = function(nodes) {
	log( "filelist received" );
	
	swapDOM( "proj-filelist",
		 UL( { "id" : "proj-filelist",
		       "style" : "display:none" },
		     map_1( bind(this._dir, this), 0, nodes["tree"] ) ) );
	
	this._show();
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

	src = ev.src();
	kind = getNodeAttribute( src, "ide_kind" );
	path = getNodeAttribute( src, "ide_path" );

	if( mods["ctrl"] ) {
		toggleElementClass( "selected", ev.src() );

		if( kind == "FOLDER" ) {
			// Select it's contents too
			
		} else {
			// Check that it's parent isn't already selected
		}

	}
	else {
		if( kind == "FOLDER" ) {
			this._toggle_dir( src );

		} else {
			editpage.edit_file( this._team, this._project, path );
		}
	}
}

ProjFileList.prototype._toggle_dir = function(src) {
	// Get a handler on its children
	var dir_contents = getFirstElementByTagAndClassName( "UL", null, src.parentNode ); 

	display = "";
	if( getStyle( dir_contents, "display" ) != "none" ) {
		display = "none";

		var nc = this._ul_get_num_children( dir_contents );

		var c = " child";
		if( nc != 1 )
			c = c + "ren";

		var div = this._get_innerdiv( src );
		appendChildNodes( div, 
				  SPAN({"class":"proj-filelist-dir-collapse"},
				       " [ " + nc + c + " hidden ]"));

	} else {
		removeElement( getFirstElementByTagAndClassName( "SPAN", null, src ) );
	}

	setStyle( dir_contents, {"display" : display} );
}

// Returns the innermost DIV within in given element
// Assumes that there's only one DIV per level
ProjFileList.prototype._get_innerdiv = function(elem) {
	var d = getFirstElementByTagAndClassName( "DIV", null, elem );
	if ( d == null )
		return elem;
	else
		return this._get_innerdiv( d );
}

ProjFileList.prototype._ul_get_num_children = function(ul) {
	var count = 0;
	for( var i in ul.childNodes ) {
		if( ul.childNodes[i].tagName == "LI" )
			count++;
	}
	return count;
}

// Object that grabs the project list
// Signals:
//  - onchange: when the projects list changes.
//              First argument is the team number
function ProjList() {
	// Array of project names (strings)
	this.projects = [];
	// Whether we've loaded
	this.loaded = false;
	// The team
	this._team = null;

	// Member functions:
	// Public:
	//  - project_exists(pname): Returns true if the given project exists.
	// Private:
	//  - _init
	//  - _grab_list: Grab the project list and update.
	//  - _got_list: Handler for the project list response.
}

// Update the list to the given team
ProjList.prototype.update = function(team) {
	this._grab_list(team);
}

ProjList.prototype._grab_list = function(team) {
	this.loaded = false;
	this._team = team;

	var d = loadJSONDoc("./projlist", {team : team});
	
	d.addCallback( bind( this._got_list, this ) );
	
	d.addErrback( bind( function() {
		status_button( "Error retrieving the project list", LEVEL_ERROR,
			       "retry", bind( this._grab_list, this) );
	}, this ) );
}

ProjList.prototype._got_list = function(resp) {
 	this.projects = resp["projects"];
 	this.loaded = true;

	signal( this, "onchange", this._team );
}

ProjList.prototype.project_exists = function(pname) {
	for( i in this.projects )
		if( this.projects[i] == pname )
			return true;
	return false;
}

// The project selector.
// Arguments:
//  - plist: The project list (ProjList)
//  - elem: The DOM object for the select box.
function ProjSelect(plist, elem) {
	this._elem = elem;
	this._plist = plist;

	// Project selection prompt
	this._ps_prompt = null;

	// The project that's selected
	// Empty string means none selected
	this.project = "";

	// The team that we're currently listing
	this._team = null;

	// Signals:
	//  - onchange: when the project selection changes.
	//              Handler passed the name of the new project.

	// Member functions:
	// Public:

	// Private:
	//  - _init: Initialisation.
	//  - _onchange: Handler for the select box onchange event.
	//  - _plist_onchange: Handler for when the list of projects changes.

	this._init();
}

ProjSelect.prototype._init = function() {
	connect( this._elem, "onchange", bind( this._onchange, this ) );
	connect( this._plist, "onchange", bind( this._plist_onchange, this ) );

	// If the list is already loaded when we're called, force update
	if( this._plist.loaded )
		this._plist_onchange();
}

// Called when the project list changes
ProjSelect.prototype._plist_onchange = function(team) {
	logDebug( "ProjSelect._plist_onchange" );
	var startproj = this.project;
	var startteam = this._team;
	var items = [];

	// Find the default project to be selected
	if( this.project == "" 
	    || !this._plist.project_exists( this.project )
	    || team != this._team ) {
		this.project = "";

		var dp = this._get_default();
		if( dp == null ) {
			// Add "Please select..."
			this._ps_prompt = status_msg( "Please select a project", LEVEL_INFO );
			items.unshift( OPTION( { "id" : "projlist-tmpitem",
						 "selected" : "selected" }, "Select a project." ) );
		} else
			this.project = dp;
	}
	this._team = team;

	// Rebuild the select box options
	for( var p in this._plist.projects ) {
		var pname = this._plist.projects[p];
		var props = { "value" : pname };
			
		if( pname == this.project )
			props["selected"] = "selected";
		items[items.length] = ( OPTION( props, pname ) );
	}

	replaceChildNodes( this._elem, items );

	logDebug( "ProjList._plist_onchange: Now on project " + this._team + "." + this.project );

	if( startproj != this.project 
	    || startteam != this._team )
		signal( this, "onchange", this.project, this._team );
}

// Handler for the onchange event of the select element
ProjSelect.prototype._onchange = function(ev) {
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

		if( proj != this.project ) {
			this.project = proj;
			signal( this, "onchange", this.project, this._team );
		}
	}
}

ProjSelect.prototype._get_default = function() {
	if( this._plist.projects.length == 1 )
		return this._plist.projects[0];

	var dp = user.get_setting( "project.last" );

	if( dp != undefined
	    && this._plist.project_exists( dp ) )
		return dp;

	return null;		
}

