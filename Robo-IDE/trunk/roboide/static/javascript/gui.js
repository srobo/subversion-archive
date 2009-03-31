cur_path = ""; //TODO: Replace these with cur_tab

open_files = {}; //A dictionary (hash table) of the currently open
//files - one for each tab

// Initialise to an invalid team number
team = 0; /*The current team number*/

LEVEL_INFO = 0;
LEVEL_OK = 1;
LEVEL_WARN = 2;
LEVEL_ERROR = 3;
MAX_TAB_NAME_LENGTH = 8;

// Number that's incremented every time a new status message is displayed
status_num = 0;
// The ID of the status bar
status_id = "status";

// The tab bar
var tabbar = null;

// The project page
var projpage = null;

// The project tab
var projtab = null;

// The edit page
var editpage = null;

// The user
var user;

// The team selector
var team_selector;

// The initial onchange event ident connected to the tabbar
// Gets disconnected as soon as a team is selected
var tabchange_ident = null;

// onload function
addLoadEvent( function() {
	//On page load - this replaces a onload action of the body tag
	//Hook up the save file button
	connect(window, 'onbeforeunload', beforeunload);

	cur_path = "";

	user = new User();
	var d = user.load();
	// Wait for the user information to come back
	d.addCallback( load_select_team );
	d.addErrback( function() { window.alert("Failed to get user info: TODO: replace this message mechanism"); } );
});

// 1) executed after the user's information has been acquired
function load_select_team() {
	// Got the user information -- now get team information
	team_selector = new TeamSelector();

	projpage = new ProjPage();
	connect( team_selector, "onchange", bind(projpage.set_team, projpage) );
	tabchange_ident = connect( team_selector, "onchange", load_gui );

	// Triggers the signals from the team selector
	team_selector.load();
}

// 2) Executed once we have team
function load_gui() {
	logDebug( "load_gui" );
	// We don't want this function to be called again
	disconnect( tabchange_ident );

	tabbar = new TabBar();

	// Projects tab
	projtab = new Tab( "Projects" );
	connect( projtab, "onfocus", bind( projpage.show, projpage ) );
	connect( projtab, "onblur", bind( projpage.hide, projpage ) );
	tabbar.add_tab( projtab );

	// Edit page
	editpage = new EditPage();

	// The "new" tab button
	var ntab = new Tab( "+ New + " );
	ntab.can_focus = false;
	connect( ntab, "onclick", bind(editpage.new_file, editpage) );
	tabbar.add_tab( ntab );

    //The selection operations
    sel_operations = new ProjOps();

	tabbar.switch_to( projtab );
}

function beforeunload(e) {
	if(editpage != null && !editpage.close_all_tabs(false))
		e.confirmUnload("You should close tabs before closing this window");
}

// **** Status Bar ****

function status_clearclass() {
	var classes = ["status-info", "status-ok", "status-warn", "status-error"];
	var s = $(status_id);

	map( partial( removeElementClass, s ), classes );
}

// Hide the status bar
function status_hide() {
	setStyle( "status-span", {"display":"none"} );

	var s = getElement(status_id);
	status_clearclass();
}

// Show the status bar with the given message, and prepend "warning" or "error"
function status_msg( message, level ) {
	switch(level) {
	case LEVEL_WARN:
		message = [ createDOM( "STRONG", null, "Warning: " ),
			    message ];
		break;
	case LEVEL_ERROR:
		message = [ createDOM( "STRONG", null, "Error: " ),
			    message ];
		break;
	}

	return status_rich_show( message, level );
}

// Replace the status bar's content with the given DOM object
function status_rich_show( obj, level ) {
	var s = getElement(status_id);

	var o = createDOM( "SPAN", { "id" : "status-span",
				     "display" : "" }, obj );
	replaceChildNodes( status_id, o );

	status_clearclass();
	switch(level) {
	case LEVEL_INFO:
		addElementClass( s, "status-info" );
		break;
	case LEVEL_OK:
		addElementClass( s, "status-ok" );
		break;
	case LEVEL_WARN:
		addElementClass( s, "status-warn" );
		break;
	default:
	case LEVEL_ERROR:
		addElementClass( s, "status-error ");
		break;
	}

	// Give it a shake if it's not OK
	if( level > LEVEL_OK )
		shake(s);

	status_num ++;
	var close_f = partial( status_close, status_num );

	return { "close": close_f };
}

// Hide the status if message id is still displayed
function status_close(id) {
	if( status_num == id )
		status_hide();
}

function status_click() {
	status_hide();
}

// Display a status message with a button
// Args:
//    message: The message to display
//      level: The log level of the message (LOG_OK etc)
//      btext: The button text
//      bfunc: The function to call when the button is clicked.
function status_button( message, level, btext, bfunc ) {
	var b = createDOM( "A", { "href" : "#" }, btext );
	connect( b, "onclick", bfunc );

	var m = [ message, " -- ", b ]

	return status_msg( m, level );
}

// The user
function User() {
	// List of team numbers
	this.teams = null;
	// Dictionary of team names (team number => name)
	this.team_names = null;
	// The user's settings
	this._settings = null;

	this._info_deferred = null;

	this.load = function() {
		// Return a deferred that fires when the data's ready
		var retd = new Deferred();

		this._check_logged_in();

		this._info_deferred = retd;
		return this._info_deferred;
	}

	this._request_info = function() {
 		var d = loadJSONDoc("./user/info");

 		d.addCallback( bind( this._got_info, this ) );

 		d.addErrback( bind( function() {
			status_button( "Failed to load user information", LEVEL_ERROR,
				       "retry", bind( this._request_info, this ) );
		}, this ) );
	}

	this._got_info = function( info ) {
		logDebug( "Got user information" );
		this.team_names = info["teams"];

		this.teams = [];
		for( var team_num in info["teams"] )
			this.teams.push( parseInt(team_num, 10) );

		this._settings = info["settings"];
		for( var k in this._settings ) {
			logDebug( k + " = " + this._settings[k] );
		}

		// Connect up the logout button
		disconnectAll( "logout-button" );
		connect( "logout-button", "onclick", bind( this._logout_click, this ) );

 		this._info_deferred.callback(null);
	}

	this.get_setting = function(sname) {
		return this._settings[sname];
	}

	// Asks the server if we're logged in
	this._check_logged_in = function() {
		var d = loadJSONDoc("./user/login", {});

		d.addCallback( bind( this._resp_logged_in, this ) );
		d.addErrback( bind( function() {
			status_button( "Failed to check whether user is logged in.", LEVEL_ERROR,
				       "retry", bind( this._check_logged_in, this ) );
		}, this ) );
	}

	// Handle the response from the server about whether we're logged in
	this._resp_logged_in = function(res) {
		if ( res["login"] ) {
			// We're logged in -- grab user information
			this._request_info();
		}
		else
			this._show_login();
	}

	// Show the login dialog
	this._show_login = function() {
		status_id = "login-feedback";

		// Connect up the onclick event to the login button
		disconnectAll( "login-button" );
		connect( "login-button", "onclick", bind( this._do_login, this ) );

		// Do stuff when the user presses enter
		disconnectAll( "password" );
		connect( "password", "onkeydown", bind( this._pwd_on_keypress, this ) );

		// Show the dialog and hide the top bar, which IE6 has problems with
		setStyle( "login-back", {"display":"block"} );
		setStyle( "top", {"display":"none"} );

		//clear box on focus, replace with 'username' on blur.
		connect("username","onfocus",function(){if ($("username").value==$("username").defaultValue) $("username").value=''});
		connect("username","onblur",function(){if (!$("username").value) $("username").value = $("username").defaultValue});
		//and focus the username
		$("username").focus();
	}

	// Hide the login dialog
	this._hide_login = function() {
		status_id = "status";
		setStyle( "login-back", {"display" :"none"} );
		setStyle( "top", {"display":""} );
	}

	// Grab the username and password from the login form and start the login
	this._do_login = function(ev) {
		if( ev != null ) {
			ev.preventDefault();
			ev.stopPropagation();
		}

		var user = $("username").value;
		var pass = $("password").value;

		var d = loadJSONDoc( "./user/login", {"usr": user, "pwd": pass} );

		d.addCallback( bind( this._login_resp, this ) );
		d.addErrback( bind( function() {
			status_button( "Error whilst logging in", LEVEL_ERROR,
				       "retry", bind( this._do_login, this ) );
		}, this ) );
	}

	this._login_resp = function(resp) {
		if( resp["login"] ) {
			// Logged in -- grab user information
			this._hide_login();
			this._request_info();
		}
		else
			// Something was wrong with username/password
			status_msg( "Incorrect username or password", LEVEL_WARN );
	}

	this._logout_click = function(ev) {
		if( ev != null ) {
			ev.preventDefault();
			ev.stopPropagation();
		}

		var d = loadJSONDoc( "./user/logout", {} );

		d.addCallback( bind( this._logout_resp, this ) );
		d.addErrback( bind( function() {
			status_button( "Failed to log out", LEVEL_ERROR,
				       "retry", partial( bind( this._logout_click, this ),
							 null ) );
		}, this ) );
	}

	this._logout_resp = function(resp) {
		window.location.reload();
	}

	// Do the login if they press enter in the password box
	this._pwd_on_keypress = function(ev) {
		if ( ev.key()["string"] == "KEY_ENTER" )
			this._do_login( null );
	}

};

function TeamSelector() {
	this._prompt = null;

	this.load = function() {
		var teambox = [];

		if( user.teams.length == 1 )
			team = user.teams[0];
		else
		{
			var olist = [];

			if( !this._team_exists(team) ) {
				// Work out what team we should be in
				var team_last = user.get_setting("team.last");
				if( team_last != undefined
				    && this._team_exists( team_last ) ) {
					team = team_last;
					logDebug( "Defaulting to team " + team );
				}
			}

			olist = this._build_options();

			if( !this._team_exists(team) ) {
				// Add a "please select a team" option
				olist.unshift( OPTION( { "id" : "teamlist-tmpitem",
							 "selected" : "selected" },
						       "Please select a team." ) );

				this._prompt = status_msg( "Please select a team", LEVEL_INFO );
			}

			var tsel = SELECT( null, olist );

			connect( tsel, "onchange", bind( this._selected, this ) );
			teambox.push( "Team: " );
			teambox.push( tsel );
		}

		// Span to hold the team name
		var tname = SPAN( { "id" : "teamname" }, null );
		teambox.push( tname );

		replaceChildNodes( $("teaminfo"), teambox );
		this._update_name();

		if( this._team_exists(team) )
			signal( this, "onchange", team );
	}

	this._build_options = function() {
		var olist = [];

		for( t in user.teams ) {
			var props = { "value" : user.teams[t]};

			if( user.teams[t] == team )
				props["selected"] = "selected";

			olist.push( OPTION(props, user.teams[t]) );
		}

		return olist;
	}

	// Returns true if the given team number exists for this user
	this._team_exists = function(team) {
		if( team == 0 )
			return false;

		for( i in user.teams )
			if( user.teams[i] == team )
				return true;
		return false;
	}

	this._selected = function(ev) {
		if( this._prompt != null ) {
			this._prompt.close();
			this._prompt = null;
		}

		var src = ev.src();

		// Remove the "please select a team" item from the list
		var tmpitem = $("teamlist-tmpitem");
		if( tmpitem != null && src != tmpitem )
			removeElement( tmpitem );

		team = parseInt(src.value, 10);
		logDebug( "team changed to " + team );
		this._update_name();

		signal( this, "onchange", team );
	}

	this._update_name = function() {
		var name = "";
		if( this._team_exists(team) ) {
			name = user.team_names[ team ];

			if( user.teams.length == 1 )
				name = "Team " + team + ": " + name
		}

		replaceChildNodes( $("teamname"), " " + name );
	}
}


// Count of outstanding asynchronous requests
var async_count = 0;

function loadJSONDoc( url, qa ) {
	async_count += 1;
	showElement( $("rotating-box") );

	var d = new Deferred();
	var r;

	if( qa == undefined )
		r = MochiKit.Async.loadJSONDoc( url );
	else
		r = MochiKit.Async.loadJSONDoc( url, qa );

	r.addCallback( partial( ide_json_cb, d ) );
	r.addErrback( partial( ide_json_err, d ) );

	return d;
}

function ide_json_cb( d, res ) {
	async_count -= 1;

	if( async_count == 0 )
		hideElement( $("rotating-box") );

	d.callback(res);
}

function ide_json_err( def, res ) {
	async_count -= 1;

	if( async_count == 0 )
		hideElement( $("rotating-box") );

	def.errback(res);
}