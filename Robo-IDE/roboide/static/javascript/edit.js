// The class for the EditPage
function EditPage() {

	// Member functions:
	// Public functions:
	//  - edit_file: Open the given team, project and file path.
	//  - new_file: Create a new edit tab with no filename.

	// Private functions:
	//  - _init: Initialises the edit page (called on instantiation)
	//  - _init_ea: Initialises the editarea
	//  - _show: Show the edit page.
	//           Triggers initialisation of the editarea if necessary
	//  - _hide: Hide the edit page.
	//  - _new_etab: Creates a new instance of an EditTab and wire it 
	// 	up to a Tab
	//               TODO: Can we get EditTab to do this for us?
	//  - _file_get_etab: Given a file path, returns the tab for it.
	//                    If the file isn't currently open, return null.
	//  - _tab_switch: Handler for the onswitch event of the tab bar.
	//		   TODO: Make this remove the tab from our list.
	//  - _is_edit: Returns try if the given tab is an edit tab

	// Private properties:
	// Dict of open files.  Keys are paths, values are EditTab instances.
	this._open_files = {};	

	this._ea_initted = false;
	this.textbox = null;
	this._visible = false;

	// The number of new files so far
	this._new_count = 0;

	// Initialise the edit page
	this._init = function() {
		connect( tabbar, "onswitch", bind( this._tab_switch, this ) );
	}

	// Initialise the editarea
	// Assumes that we're visible at the time
	this._init_ea = function() {
		logDebug( "initting textarea" );
		this.textbox = TEXTAREA({"id" : "editpage-editarea",
					 "value" : "" });
		appendChildNodes($("edit-mode"), this.textbox);

		//initialize new instance of editArea			
		editAreaLoader.init({
	 		id : "editpage-editarea",
	 		syntax : "python",
	 		language : "en",
	 		start_highlight : true,
	 		allow_toggle : false,
	 		allow_resize : "no",
			display : 'onload',
	 		replace_tab_by_spaces : 4,
			min_width: "100",   //NOTE: HAD TO EDIT 'edit_area_loader.js' line:573 to get % width
			min_height:500,
			change_callback: "txt_has_changed"
 		});
		this._ea_initted = true;
	}

	// Show the edit page
	this._show = function() {
		if( this.visible )
			return;

		setStyle($("edit-mode"), {"display" : "block"});

		if( !this._ea_initted )
			this._init_ea();

		logDebug( "showing editarea" );
		editAreaLoader.show( "editpage-editarea" );

		this._visible = true;
	}

	// Hide the edit page
	this._hide = function() {
		if( !this._visible ) 
			return;

		if( this._ea_initted ) {
			logDebug( "hiding editarea" );
			editAreaLoader.hide( "editpage-editarea" );
		}

		this._visible = false;
		setStyle($("edit-mode"), {"display" : "none"});
	}

	// Open the given file and switch to the tab
	// or if the file is already open, just switch to the tab
	this.edit_file = function( team, project, path, rev ) {
		// TODO: We don't support files of the same path being open in
		// different teams at the moment.
		var etab = this._file_get_etab( path );

		if( etab == null ) {
			etab = this._new_etab( team, project, path, rev);
			// TODO: Load file contents
		}

		tabbar.switch_to( etab.tab );
	}

	// Create a new tab with a new file open in it
	this.new_file = function() {
		this._new_count ++;
		var fname = "New File " + this._new_count;
		var etab = this._new_etab( team, null, fname, 0 );
		tabbar.switch_to( etab.tab );
	}

	// Create a new tab that's one of ours
	// Doesn't load the tab
	this._new_etab = function(team, project, path, rev) {
		var etab = new EditTab(team, project, path, rev);
		
		connect( etab, "onclose", bind( this._on_tab_close, this ) );

		this._open_files[path] = etab;
		return etab;
	}

	// Return the tab for the given file path
	// returns null if not open
	this._file_get_etab = function( path ) {
		for( var i in this._open_files ) {
			if( i == path && this._open_files[i] !== null )
				return this._open_files[i];
		}
		return null;
	}

	// Handler for when the tab has been closed
	this._on_tab_close = function( etab ) {
		// Remove tab from our list
		for( var i in this._open_files ) {
			if( this._open_files[i] === etab ) {
				this._open_files[i] = null;
				break;
			}
		}
	}

	this._tab_switch = function( fromtab, totab ) {
		if( !this._is_edit( totab ) ) {
			this._hide();
			return;
		}

		if( !this._is_edit( fromtab ) )
			this._show();
	}

	// Return true if the given tab is an edit tab
	this._is_edit = function(tab) {
		if( tab !== null && tab !== undefined 
		    && tab.__edit === true )
			return true;
		return false;
	}

	this._init();

}

function txt_has_changed(id) {
    //relay editArea's signal so that we can actually use it
    //editArea forces you to pass string
    signal(this, "txt_changed", this);
}

// Represents a tab that's being edited
// Managed by EditPage -- do not instantiate outside of EditPage
function EditTab(team, project, path, rev) {
	// Member functions:
	// Public:
	//  None.
	// Private:
	//  - _init: Constructor.
	//  - _check_syntax: Handler for when the "check syntax" button is clicked
	//  - _update_contents: Update the contents of the edit area.
	//  - _capture_code: Store the contents of the edit area.

	//  ** File Contents Related ** 
	//  - _load_contents: Start the file contents request.
	//  - _recv_contents: Handler for file contents reception.
	//  - _recv_contents_err: Handler for file contents reception errors.

	//  ** Save Related **
	//  - _save: Handler for when the save button is clicked.
	//  - _receive_new_fname: Handler for save dialog.
	//  - _receive_commit_msg: Handler for save dialog.
	//  - _svn_save: Save the file to the server.
	//  - _receive_svn_save: Handler for successfully sending to server. 
	//  - _error_receive_svn_save: Handler for when a save fails.

	//  ** Tab related **
	//  - _close: Handler for when the tab is closed.
	//  - _onfocus: Handler for when the tab receives focus.
	//  - _onblur: Handler for when the tab is blurred.

	// *** Public Properties ***
	if(rev == null || rev == undefined)
	    this.rev = 0; 
	else
	    this.rev = rev;
	
	// The team
	this.team = team;
	// The project
	this.project = project;
	// The path
	this.path = path;
	// The current contents
	this.contents = "";
	// The tab representing us
	this.tab = null;
	
	// *** Private Properties ***
	// true if tab has been modified
	this._dirty = false;
	//true if file is new (unsaved)
	this._isNew = false;
	//The commit message
	this._commitMsg = "Default Commit Message";
	//the original contents (before edits)
	this._original = "";
	// All our current signal connection idents
	this._signals = [];
	// The "Failed to load contents" of file status message:
	this._stat_contents = null;

	this._init = function() {
		this.tab = new Tab( this.path );
		tabbar.add_tab( this.tab );

		// Mark the tab as a edit tab
		this.tab.__edit = true;

		// Link ourselves to the tab so the EditPage can find us
		this.tab.__etab = this;

		connect( this.tab, "onfocus", bind( this._onfocus, this ) );
		connect( this.tab, "onblur", bind( this._onblur, this ) );

		if( this.project == null ) {
			// New file
			this._isNew = true;
			this.contents = "";
			this._original = "";
			this._dirty = false;
		} else
			// Existing file
			this._load_contents();
	}

	// Start load the file contents
	this._load_contents = function() {
		var d = loadJSONDoc("./filesrc", { team : this.team,
						   file : this.path,
						   revision : this.rev});

		d.addCallback( bind(this._recv_contents, this));	
		d.addErrback( bind(this._recv_contents_err, this));
	}

	// Handler for the reception of file contents
	this._recv_contents = function(nodes) {
		if( this._stat_contents != null ) {
			this._stat_contents.close();
			this._stat_contents = null;
		}

		this.contents = nodes.code;
		this._original = nodes.code;
		this._isNew = false;
		this._dirty = false;

		this._update_contents();
		
		status_msg("File: "+this.path+" Opened at revision: "+this.rev, LEVEL_OK);
	}

	// Handler for errors in receiving the file contents
	this._recv_contents_err = function() {
		this._stat_contents = status_button( "Failed to load contents of file " + this.path,
						     LEVEL_ERROR,
						     "retry", bind( this._load_contents, this ) );
	}

	this._check_syntax = function() {
		status_msg( "Check syntax of " + this.path, LEVEL_WARN );
	}

	this._receive_new_fname = function(fpath, commitMsg) {
		var a = fpath.split( "/", 2 );

		if (a.length == 2 ) {
			this.project = a[1];
			this.path = fpath;
			this._commitMsg = commitMsg;
			this._svn_save();
		} else
			status_msg( "No project specified", LEVEL_ERROR );
	}

	this._receive_commit_msg = function(commitMsg) {
		this._commitMsg = commitMsg;
		this._svn_save();
	}
	
	this._save = function() {
		//do an update
		this._capture_code();
		//if new file	-- TODO
		if(this._isNew) {
			var fileBrowser = new Browser(bind(this._receive_new_fname, this), {'type' : 'isFile'});	
		}
		else {
			var fileBrowser = new Browser(bind(this._receive_commit_msg, this), {'type' : 'isCommit'});	
		}
	}

	//ajax event handler for saving to server
	this._receive_svn_save = function(nodes){
		projpage.flist.refresh();

		switch(nodes.success){
			case "True": 
				status_msg("File Saved successfully (New Revision: "+nodes.new_revision+")", LEVEL_OK);
				this._dirty = false;
				this._original = this.contents;
				this._isNew = false;
 				this._update_contents();
				break;
			case "Merge":
				status_msg("File Merge successful (New Revision: "+nodes.new_revision+")", LEVEL_OK);
				this._dirty = false;
				this._original = this.contents;
				this._isNew = false;
 				this._update_contents();
				break;
			case "Error creating new directory":
				status_msg("Error creating new directory (New Revision: "+nodes.new_revision+")", LEVEL_ERROR);
				break;
			case "Invalid filename" :
				status_msg("Save operation failed, Invalid filename (New Revision: "+nodes.new_revision+")", LEVEL_ERROR);
				break;
		}
	}
	
	//ajax event handler for saving to server
	this._error_receive_svn_save = function() {
		button_status("Error contacting server", LEVEL_ERROR, "retry", bind(this._svn_save, this));
	}
	
	//save file contents to server as new revision
	this._svn_save = function() {
	    var d = loadJSONDoc("./savefile", { team : team,
					        file : this.path, 
						    rev : 0,				//TODO: make this dynamic
					        message : this._commitMsg,
						    code: this.contents});

	    d.addCallback( bind(this._receive_svn_save, this));	
	    d.addErrback( bind(this._error_receive_svn_save, this));		
	}

	this._close = function(override) {
		//update
		this._capture_code();
		var obj = this;
		if(this._dirty == true && !override ) {
			status_button(this.path+" has been modified!", LEVEL_WARN, "Close Anyway", bind(this._close, this, true));
		}
		else{
			signal( this, "onclose", this );
			this.tab.close();
			disconnectAll(this);
			status_hide();
		}
	}

	// Handler for when the tab receives focus
	this._onfocus = function() {
		// Close handler
		this._signals.push( connect( $("close-edit-area"),
					     "onclick",
					     bind( this._close, this, false ) ) );
		// Check syntax handler
		this._signals.push( connect( $("check-syntax"),
					     "onclick",
					     bind( this._check_syntax, this ) ) );
		// Save handler
		this._signals.push( connect( $("save-file"),
					     "onclick",
					     bind( this._save, this ) ) );
		// change revision handler
		this._signals.push( connect( "history",
					     "onclick",
					     bind( this._change_revision, this ) ) );
		//code has been changed
		this._signals.push( connect(window, 
					    "txt_changed", 
					    bind(this._content_changed, this) ) );				     						  

		//load file contents
		this._update_contents();		
	}

	// Handler for when the tab loses focus
	this._onblur = function() {
		//don't loose changes to file content
		this._capture_code();   
		// Disconnect all the connected signal
		map( disconnect, this._signals );
		this._signals = [];
	}

	this._update_contents = function() {
		logDebug("Updating editarea contents: ");
	 	editAreaLoader.setValue("editpage-editarea", this.contents);
	 	this._get_revisions();

		// Display file path
		replaceChildNodes( $("tab-filename"), this.project + "::" + this.path );
		this.tab.set_label( this.path );
	}

	//call this to update this.contents with the current contents of the edit area
	this._capture_code = function() {
		this.contents = editAreaLoader.getValue("editpage-editarea");
	}

	this._change_revision = function() {
		switch($("history").value) {
		case "-2":
			var d = new Log(this.path);
			break;
		case "-1":
			break;
		default:
			this.rev = $("history").value;
			status_msg("Opening history .."+$("history").value, LEVEL_OK);
           		this._load_contents();
           		break;
		}
	}
	
	this._receive_revisions = function(nodes) {
		if(nodes.history.length == 0) {
			replaceChildNodes("history", OPTION({'value' : -1}, "No File History!"));
		} else {
			replaceChildNodes("history", OPTION({'value' : -1}, "Select File Revision"));	    
			for(var i=0; i < nodes.history.length; i++)
				appendChildNodes("history", OPTION({'value' : nodes.history[i].rev, 'title' : "Log Msg: "+nodes.history[i].message}, nodes.history[i].date+" ["+nodes.history[i].author+"]"));

			appendChildNodes("history", OPTION({'value' : -2}, "--View Full History--"));			
		}
	}

	this._error_receive_revisions = function() {
		status_msg("Couldn't retrieve file history", LEVEL_ERROR);
	}
    
	this._get_revisions = function() {
		logDebug("retrieving file history");
		var d = loadJSONDoc("./gethistory", { team : team,
						      file : this.path, 
						      user : null,
						      offset : 0});
		d.addCallback( bind(this._receive_revisions, this));	
		d.addErrback( bind(this._error_receive_revisions, this)); 			    
	}

	//editAreaLoader triggers onchange event, now handle it:
	this._content_changed = function() {
		this._dirty = true;
		logDebug("Current File Tab is now dirty");
		//now we don't need to listen out for event:
		var sig = this._signals.pop();
		disconnect(sig);
	}
	
	//initialisation
	this._init();
}
