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
	//  - _new_etab: Creates a new instance of an EditTab and wire it up to a Tab
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
		//now let textbox point point to actual textbox
		this.textbox = $("editpage-editarea"); 
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
			min_width:600,
			min_height:400
 		});
		
		this._ea_initted = true;
	}
	//actually get the file contents
	this._receive_file_contents = function(fpath, nodes) {
		this._open_files[fpath].contents = nodes.code;
		this._open_files[fpath].original = nodes.code;
		this._open_files[fpath]._update_contents();
	}

	this._error_receive_file_contents = function() {
		this._file_contents = "Error - Cannot load contents";
	}

	this._get_file_contents = function(fpath, revision) {
		var d = loadJSONDoc("./filesrc", { team : team,
								file : fpath, 
								revision : revision});

			d.addCallback( bind(this._receive_file_contents, this, fpath));	
			d.addErrback( bind(this._error_receive_file_contents, this));		
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
	this.edit_file = function( team, project, path ) {
		// TODO: We don't support files of the same path being open in
		// different teams at the moment.
		var etab = this._file_get_etab( path );

		if( etab == null ) {
			etab = this._new_etab( team, project, path );
			// TODO: Load file contents
		}

		tabbar.switch_to( etab.tab );
	}

	// Create a new tab with a new file open in it
	this.new_file = function() {
		this._new_count ++;
		var fname = "New File " + this._new_count;
		var etab = this._new_etab( team, null, fname );

		tabbar.switch_to( etab.tab );
	}

	// Create a new tab that's one of ours
	// Doesn't load the tab
	this._new_etab = function(team, project, path) {
		var etab = new EditTab(path);
		etab.path = path;
		etab.project = project;
		logDebug("Path: "+path);		
		this._get_file_contents(path, 0);

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

// Represents a tab that's being edited
// Managed by EditPage -- do not instantiate outside of EditPage
function EditTab(path) {
	// The team
	this.team = null;
	// The project (TODO)
	this.project = null;
	// The path
	this.path = path;

	//The commit message
	this.commitMsg;

	//the original contents (before edits)
	this.original;

	// The current contents
	this.contents;

	// The tab representing us
	this.tab = null;

	// true if tab has been modified
	this.dirty = true;	//

	//true if file is new (unsaved)
	this.isNew = true;	//TODO

	// All our current signal connection idents
	this._signals = [];

	this._init = function() {
		this.tab = new Tab( this.path );
		tabbar.add_tab( this.tab );

		// Mark the tab as a edit tab
		this.tab.__edit = true;

		// Link ourselves to the tab so the EditPage can find us
		this.tab.__etab = this;

		connect( this.tab, "onfocus", bind( this._onfocus, this ) );
		connect( this.tab, "onblur", bind( this._onblur, this ) );
	}

	this._check_syntax = function() {
		status_msg( "Check syntax of " + this.path, LEVEL_WARN );
	}

	this._receive_new_fname = function(fpath, commitMsg) {
		this.path = fpath;
		this.commitMsg = commitMsg;
		this.dirty = false;
		this._save();
	}
	
	this._save = function() {
		//do an update
		this._save_contents();
		//if new file	-- TODO
		if(this.dirty) {
			status_msg( "Enter a filename for new file", LEVEL_OK );
			var fileBrowser = new Browser(team, bind(this._receive_new_fname, this), {'isFile' : 'true'});	
		}
		else {
			//TODO - call to actual save function
			status_msg( this.path +" Saved OK", LEVEL_OK );
		}
	}

	this._close = function(override) {
		//update
		this._save_contents();
		var obj = this;
		if(this.dirty == true && !override ) {
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

		//display filepath
		replaceChildNodes( $("tab-filename"), this.project + "::" + this.path );

		//load file contents
		this._update_contents();		
	}

	// Handler for when the tab loses focus
	this._onblur = function() {
		//don't loose changes to file content
		this._save_contents();
		// Disconnect all the connected signal
		map( disconnect, this._signals );
		this._signals = [];
	}

	this._update_contents = function() {
		logDebug("Updating editarea contents: ");
	 	editAreaLoader.setValue("editpage-editarea", this.contents);
	}
	this._save_contents = function() {
		this.contents = editAreaLoader.getValue("editpage-editarea");
		if(this.contents == this.original) {
			this.dirty = false;
		}
		else {
			this.dirty = true;
		}
	}
	this._init();
}
