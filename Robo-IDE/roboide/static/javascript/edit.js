// The class for the EditPage
function EditPage() {

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
			min_width:600,
			min_height:400
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
	this.edit_file = function( path ) {
		var etab = this._file_get_etab( path );

		if( etab == null ) {
			etab = this._new_etab( path );
			// TODO: Load file contents
		}

		tabbar.switch_to( etab.tab );
	}

	// Create a new tab with a new file open in it
	this.new_file = function() {
		this._new_count ++;
		var fname = "New File " + this._new_count;
		var etab = this._new_etab( fname );

		tabbar.switch_to( etab.tab );
	}

	// Create a new tab that's one of ours
	// Doesn't load the tab
	this._new_etab = function(path) {
		var tab = new Tab( path );
		connect( tab, "onclose", bind( this._tab_close, this ) );
		tabbar.add_tab( tab );

		var etab = new EditTab(tab);
		etab.path = path;
		etab.contents = "TODO: Do the contents stuff";

		this._open_files[path] = etab;
		return etab;
	}

	// Return the tab for the given file path
	// returns null if not open
	this._file_get_etab = function( path ) {
		for( var i in this._open_files ) {
			if( i == path )
				return this._open_files[i];
		}
		return null;
	}

	this._tab_close = function( tab ) {
		
	}

	this._tab_switch = function( fromtab, totab ) {
		if( !this._is_edit( totab ) ) {
			this._hide();
			return;
		}

		if( !this._is_edit( fromtab ) )
			this._show();

		this._etab_focus( totab.__etab );
	}

	// Called (not by TabBar) when the file that's being edited is changed  
	this._etab_focus = function( etab ) {
		// Disconnect signal handlers from old tab
		this._disconnect_etab();
		// Connect signal handler to the new tab
		this._connect_etab(etab);
	}

	// Connect the buttons etc on the page to the given EditTab
	this._connect_etab = function( etab ) {
		//display filepath
		replaceChildNodes( $("tab-filename"), etab.project + "::" + etab.path );

		// Connect up the buttons (TODO)
		connect($("close-edit-area"), 'onclick', bind(etab._close, etab));
		connect($("check-syntax"), 'onclick', bind(etab._check_syntax, etab));
		connect($("save-file"), 'onclick', bind(etab._save, etab));
	}

	// Disconnect the buttons etc on the page
	this._disconnect_etab = function() {
		disconnectAll($("close-edit-area"));
		disconnectAll($("check-syntax"));
		disconnectAll($("save-file"));		
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
function EditTab(tab) {
	// The path
	this.path;

	// The project (TODO)
	this.project = "TODO";

	// The current contents
	this.contents;

	// The tab representing us
	this.tab = tab;

	this._init = function() {
		// Mark the tab as a edit tab
		this.tab.__edit = true;

		// Link ourselves to the tab so the EditPage can find us
		this.tab.__etab = this;
	}

	this._check_syntax = function() {
		status_msg( "Check syntax of " + this.path, LEVEL_WARN );
	}
	
	this._save = function() {
		status_msg( "Save " + this.path, LEVEL_WARN );
	}

	this._close = function() {
		status_msg( "Close " + this.path, LEVEL_WARN );
	}

	this._init();
}