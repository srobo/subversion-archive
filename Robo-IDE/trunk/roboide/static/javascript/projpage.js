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
	// The calendar
	this._calendar = null;

	this._iframe = null;

	this.flist = null;
	this.project = "";
	
	this.last_updated	= new Date();

	// Member functions (declared below)
	// Public:
	//  - show: Show and activate the projects page
	//  - hide: Hide the project page
	//  - hide_filelist: Hide the file list
	//  - projects_exist: check that one, or more, projects exist
	// Private:
	//  - _init: Initialises members of the project page
	//  - _rpane_show: Show the right-hand pane
	//  - _rpane_hide: Hide the right-hand pane
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

	this._calendar = new Calendar();
	// Refresh the calendar when the project changes
	connect(this._selector, "onchange", bind( this._calendar.change_proj, this._calendar ) );

	// Connect up the project management buttons
	connect("new-project", 'onclick', bind(this.clickNewProject, this));
	connect("export-project", 'onclick', bind(this.clickExportProject, this));
	connect("check-code", "onclick", bind(this.clickCheckCode, this));

	// We have to synthesize the first "onchange" event from the ProjSelect,
	// as these things weren't connected to it when it happened
	this._on_proj_change( this._selector.project );
	this.flist.update( this._selector.project, this._selector._team );

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

ProjPage.prototype.hide_filelist = function() {
	logDebug( "Hiding the file list" );
	this.flist._hide();
}

ProjPage.prototype.projects_exist = function() {
	if(this._list.projects.length > 0)
		return true;
	else
		return false;
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

ProjPage.prototype.clickNewProject = function() {
	showElement($("new-project-box"));
	showElement($("grey-out"));

	connect("new-project-cancel", "onclick",
		bind(this.clickCancelNewProject, this));
	connect("new-project-create", "onclick",
		bind(this.clickCreateNewProject, this));
}

ProjPage.prototype.clickCancelNewProject = function() {
	hideElement($("new-project-box"));
	hideElement($("grey-out"));
	disconnectAll("new-project-cancel");
	disconnectAll("new-project-create");
}

ProjPage.prototype.clickCreateNewProject = function() {
	hideElement($("new-project-box"));
	hideElement($("grey-out"));
	disconnectAll("new-project-cancel");
	disconnectAll("new-project-create");

	var newProjName = $("new-project-input").value;
	/* Postback to create a new project - then what? */

	var d = loadJSONDoc("./createproj",{ name : newProjName, team : team });
	d.addCallback(bind(this.createProjectSuccess, this, newProjName));
	d.addErrback(bind(this.createProjectFailure, this));
}

ProjPage.prototype.createProjectSuccess = function(newProjName) {
	status_msg("Created project successfully", LEVEL_OK);
	update(team)
	// Transition to the new project once the project list has loaded
	this._selector.trans_project = newProjName;
	this._list.update(team)
}

ProjPage.prototype.createProjectFailure = function() {
	/* XXX - check for preexisting projects perhaps */
	status_msg("Create project failed - svn error", LEVEL_ERROR);
}

ProjPage.prototype.clickExportProject = function() {
	if( $('projlist-tmpitem') != null && $('projlist-tmpitem').selected == true ) {
		status_msg( "No project selected, please select a project", LEVEL_ERROR );
		return;
	}
	if( this._iframe == null ) {
		this._iframe = $('robot-zip');
	}

	this._iframe.src = "./checkout?team=" + team + "&project=" + this.project;
}

ProjPage.prototype.clickCheckCode = function() {
	var d = loadJSONDoc("./checkcode",{ team : team, path : "/" + this.project });

	d.addCallback(bind(this.doneCheckCode, this));
	d.addErrback(bind(this.failCheckCode, this));
}

ProjPage.prototype.doneCheckCode = function(info) {

	if( info["errors"] == 1 )
		var a = new ErrorsTab( info);
	else
		status_msg( "No errors found", LEVEL_OK );
}

ProjPage.prototype.failCheckCode = function() {
	status_button( "Failed to check code", LEVEL_ERROR,
		       "retry", bind( this.clickCheckCode, this ) );
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

	// the project revision we're displaying
	// can be integer or "HEAD"
	this.rev = "HEAD";
	this._rev = "HEAD";

	// The files/folders that are currently selected
	this.selection = [];

	// Member functions:
	// Public:
	//  - change_rev: change the file list revision, uses current project and team info to update
	//  - update: Update the file list to the given project and team
	// Private:
	//  - _received: handler for receiving the file list
	//  - _nested_divs: Returns N nested divs.
	//  - _dir: Returns the DOM object for a directory entry
	//  - _onclick: The onclick handler for a line in the listing
	//  - _hide: Hide the filelist
	//  - _show: Show the filelist
}

ProjFileList.prototype.change_rev = function(revision) {
	this.rev = revision;

    this.update(this._project, this._team, this.rev);
}

// Request and update the project file listing
ProjFileList.prototype.update = function( pname, team, rev ) {
	logDebug( "ProjFileList.update( \"" + pname + "\", " + team + ", "+rev+" )" );
	if( pname == "" ) {
		// No project selected.
		this._hide();
		return;
	}

    if(rev == undefined || rev == null) {
        this.rev = "HEAD";
    } else {
        this.rev = rev;
    }

	if( pname != this._project || team != this._team || rev != this._rev ) {
		// Hide the list whilst we're loading it
		swapDOM( "proj-filelist",
			 DIV( {"id": "proj-filelist",
			       "class" : "loading"},
			      "Loading project file listing..." ) );
	}

	this._project = pname;
	this._team = team;
	this._rev = rev;
	this.refresh();
}

ProjFileList.prototype.refresh = function() {
	if( this._project == "" )
		return;
		
	this.selection = new Array();
	var d = loadJSONDoc("./filelist", {team : this._team,
					   rootpath : this._project, rev : this.rev} );

	d.addCallback( bind( this._received, this ) );

	d.addErrback( bind( function (){
		status_button( "Error retrieving the project file listing", LEVEL_ERROR,
			       "retry", bind( this.refresh, this ) );
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

	// Assemble links to available autosaved versions
	var autosave_link = this._autosave_links( node, level );

	if( node.kind == "FILE" ) {
		var n = LI( null, link , autosave_link );
		return n;
	} else
		var n = LI( null, [ link,
	    		UL( { "class" : "flist-l" },
			map_1( bind(this._dir, this), level + 1, node["children"] ) ) ] );
	return n;
}

// Returns a DOM object for the given node's autosaves
ProjFileList.prototype._autosave_links = function( node, level ) {
	if( node.kind != "FILE" || node.autosave == 0 )
		return null;

	// Assemble the link with divs in it
	var link = A( { "href" : "#",
				"class" : 'autosave',
				"ide_path" : node.path,
				"ide_kind" : 'AUTOSAVE' },
				'AutoSave (r'+node.autosave.revision+' at '+node.autosave.date+')' );
	connect( link, "onclick", bind( this._onclick, this ) );
	return link;
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
		if( !this._is_file_selected( path ) ) {
			addElementClass( src.parentNode, "selected" );

			this._select_path( path, kind );
		} else {
			removeElementClass( src.parentNode, "selected" );

			this._deselect_path( path, kind );
		}
	} else {
		if( kind == "FOLDER" ) {
			this._toggle_dir( src );
		} else if( kind == 'AUTOSAVE' ) {
			editpage.edit_file( this._team, this._project, path, this.rev, 'AUTOSAVE' );
			//do something special
		} else {
			editpage.edit_file( this._team, this._project, path, this.rev, 'SVN' );
		}
	}
}

ProjFileList.prototype._is_file_selected = function( path ) {
	for( var i in this.selection )
		if( this.selection[i] == path )
			return true;
	return false;
}

ProjFileList.prototype._select_path = function(path, kind) {
	this.selection.push( path );
}

ProjFileList.prototype._deselect_path = function(path, kind) {
	var i = findValue( this.selection, path );
	if( i >= 0 ) {
		// Remove from the listn
		this.selection.splice( i, 1 );
	}
}

// Toggles the display of the contents of a directory
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
	// Project to transition to when the projlist changes
	this.trans_project = "";

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

	// Find the project to select
	if( this.trans_project != ""
	    && this._plist.project_exists( this.trans_project ) ) {
		this.project = this.trans_project;

		// Clear the transition default
		this.trans_project = "";

	} else if( this.project == ""
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

//handles all 'selection operations' in sidebar of project page
function ProjOps() {

    //view_log()                    for each item selected in file list it will attempt to open a new log tab
    //receive_newfolder([])         ajax success handler
    //error_receive_newfolder()     ajax fail hanlder
    //newfolder()                   gets folder name & location and instigates new folder on server

    //list of operations
    this.ops = new Array();


    this.init = function() {
        //connect up operations
        for(var i=0; i < this.ops.length; i++) {
            this.ops[i].event = connect(this.ops[i].handle, 'onclick', this.ops[i].action);
        }
    }

    this.view_log = function() {
	//for every file that is selected:
	    if(projpage.flist.selection.length == 0) {
	        status_msg("No file/folders selected", LEVEL_WARN);
	        return;
	    }
	    for(var i = 0; i < projpage.flist.selection.length; i++) {
		    //try to find log file in tabbar
		    var exists = map(function(x){
							    if(x.label == "Log: "+projpage.flist.selection[i]) {
								    return true;}
							    else { return false; }
						    }, tabbar.tabs);
		    var test = findValue(exists, true);
		    //if already present, flash it but don't open a new one
		    if(test > -1) {
			    tabbar.tabs[test].flash();
		    }
		    //not present, open it
		    else{
			    var cow = new Log(projpage.flist.selection[i]);
		    }
	    }
    }

    this.receive_newfolder = function(nodes) {
        logDebug("Add new folder: ajax request successful");
        switch(nodes.success) {
            case 1:
                status_msg("New Directory successfully added", LEVEL_OK);
                projpage.flist.refresh();
                break;
            case 0:
                status_msg("Failed to create new Directory", LEVEL_ERROR);
                break;
        }
    }

    this.error_receive_newfolder = function(new_name, new_message) {
        logDebug("Add new folder: ajax request failed");
       	button_msg("Error contacting server", LEVEL_ERROR, "retry", bind(this.new_folder, this, new_name, new_msg) );
    }

    this.new_folder = function(new_name, new_msg) {
        logDebug("Add new folder: "+new_name+" ...contacting server");
	    if(new_name == null || new_name == undefined) {
		    var browser = new Browser(bind(this.new_folder, this), {'type' : 'isDir'});
	    }
	    else {
	        var d = loadJSONDoc("./newdir", { team : team,
					            path : new_name,
					            msg : new_msg});

	        d.addCallback( this.receive_newfolder);
	        d.addErrback( this.error_receive_newfolder, new_name, new_msg);
	    }
    }

    this.newfile = function() {
        status_msg("TODO: Implement add file");
    }

    this._mv_success = function(nodes) {
        logDebug("_mv_success()");
        logDebug(nodes.status);
        if(nodes.status == 0) {
            status_msg("Move successful!", LEVEL_OK);
            projpage.flist.refresh();
        }
        else {
            status_msg("ERROR: "+nodes.message, LEVEL_ERROR);
        }
    }

    this._mv_cback = function(dest, cmsg) {
        var src = projpage.flist.selection[0];
        var type = null;

        //is it a file or a folder?
        if(src.indexOf(".") < 0) { type = 'isDir'; }
        else { type = 'isFile'; }

        //do we already have a move to location?
        logDebug("type "+type);
        if(dest == "" || dest == null) {
            logDebug("launch file browser to get move destination");
            var b = new Browser(bind(this._mv_cback, this), {'type' : 'isFile'});
            return;
        }
        else {
            //do some sanity checking
            switch(type) {
                case 'isFile' :
                    if(dest.indexOf(".") < 0) {
                        status_msg("Move destination file must have an extension", LEVEL_ERROR);
                        return;
                    }
                    break;
                case 'isDir' :
                    if(dest[dest.length-1] == "/") {
                        dest = dest.slice(0, dest.length-2);
                    }
                    if(dest.indexOf(".") > 0) {
                        status_msg("Move destination must be a folder", LEVEL_ERROR);
                        return;
                    }
                    break;
            }
        }

        status_msg("About to do move..."+src+" to "+dest, LEVEL_OK);

        	var d = loadJSONDoc("./move", {team : team,
					   src : src, dest : dest, msg : cmsg});

	        d.addCallback( bind( this._mv_success, this) );

	        d.addErrback( bind( function (){
		        status_button( "Error moving files/folders", LEVEL_ERROR,
			               "retry", bind( this._mv_cback, this, dest, cmsg ) );
	        }, this ) );
    }
    this.mv = function() {
        //we can only deal with one file/folder at a time, so ignore all but the first
        if(projpage.flist.selection.length == 0 || projpage.flist.selection.length > 1) {
            status_msg("You must sellect a single file/folder", LEVEL_ERROR);
            return;
        }

        var b = new Browser(bind(this._mv_cback, this), {'type' : 'isFile'});
        return;

    }

    this._cp_callback1 = function(nodes) {
        if(nodes.status > 0) {
            status_msg("ERROR COPYING: "+nodes.message, LEVEL_ERROR);
        }
        else {
            status_msg("Successful Copy: "+nodes.message, LEVEL_OK);
            projpage.flist.refresh();
        }
    }
    this._cp_callback2 = function(fname, cmsg) {
        logDebug("copying "+projpage.flist.selection[0]+" to "+fname);

        if(fname == null || fname=="")
            return;

    	var d = loadJSONDoc("./copy", {team : team,
				   src : projpage.flist.selection[0],
				   dest : fname,
				   msg : cmsg,
				   rev : 0  });
	    d.addCallback( bind(this._cp_callback1, this));
	    d.addErrback(function() { status_button("Error contacting server", LEVEL_ERROR, "retry", bind(this._cp_callback2, this, true));});
    }
    this.cp = function() {
        if(projpage.flist.selection.length == 0) {
            status_msg("There are no files/folders selected to copy", LEVEL_ERROR);
            return;
        }
        if(projpage.flist.selection.length > 1) {
            status_msg("Multiple files selected!", LEVEL_ERROR);
            return;
        }
        var b = new Browser(bind(this._cp_callback2, this), {'type' : 'isFile'});
        return;
    }
    this.rm = function(override) {
        if(projpage.flist.selection.length == 0) {
            status_msg("There are no files/folders selected for deletion", LEVEL_ERROR);
            return;
        }
        if(override == false) {
            status_button("Are you sure you want to delete "+projpage.flist.selection.length+" selected files/folders", LEVEL_WARN, "delete", bind(this.rm, this, true));
            return;
        }
        var death_list = "";
        for(var x = 0; x< projpage.flist.selection.length; x++) {
            death_list = death_list + projpage.flist.selection[x] + ",";
        }
        death_list = death_list.slice(0, death_list.length-1);

        logDebug("will delete: "+death_list);

    	var d = loadJSONDoc("./delete", {team : team,
				   files : death_list});
	    d.addCallback( function(nodes) {
		status_msg(nodes.Message, LEVEL_OK)
                projpage.flist.refresh();
	     });

	    d.addErrback(function() { status_button("Error contacting server",
			    LEVEL_ERROR, "retry", bind(this.rm, this, true));});
    }

    this._undel_callback = function(nodes) {
        if(nodes.status > 0) {
            status_msg("ERROR Undeleting: "+nodes.message, LEVEL_ERROR);
        }
        else {
            status_msg("Successful Undelete", LEVEL_OK);
            projpage.flist.refresh();
        }
    }
    this.undel = function() {
        if(projpage.flist.selection.length == 0) {
            status_msg("There are no files/folders selected for undeletion", LEVEL_ERROR);
            return;
        }

    	var d = loadJSONDoc("./copy", {team : team,
				   src : projpage.flist.selection[0],
				   dest : projpage.flist.selection[0],
				   msg : "Undelete File",
				   rev : projpage.flist.rev  });
	    d.addCallback( bind(this._undel_callback, this));
	    d.addErrback(function() { status_button("Error contacting server", LEVEL_ERROR, "retry", bind(this.undel, this, true));});
    }

    this.ops.push({ "name" : "New File",
                        "action" : bind(editpage.new_file, editpage),
                        "handle" : $("op-newfile"),
                        "event" : null});

    this.ops.push({ "name" : "New Directory",
                        "action" : bind(this.new_folder, this, null, null),
                        "handle": $("op-mkdir"),
                        "event" : null});

    this.ops.push({ "name" : "Move",
                        "action" : bind(this.mv, this),
                        "handle": $("op-mv"),
                        "event" : null });

    this.ops.push({ "name" : "Copy",
                        "action" : bind(this.cp, this),
                        "handle": $("op-cp"),
                        "event" : null });

    this.ops.push({ "name" : "Delete",
                        "action" : bind(this.rm, this, false),
                        "handle": $("op-rm"),
                        "event" : null });

    this.ops.push({ "name" : "Undelete",
                        "action" : bind(this.undel, this),
                        "handle": $("op-undel"),
                        "event" : null });

    this.ops.push({ "name" : "View Log",
                        "action" : bind(this.view_log, this),
                        "handle": $("op-log"),
                        "event" : null });

    this.init();
}