cur_path = ""; //TODO: Replace these with cur_tab

open_files = {}; //A dictionary (hash table) of the currently open
//files - one for each tab

//POLLING
POLL_TIME = 2000; //In ms.
poll_data = {}; /*The data to be shipped with the poll.
files : comma seperated list of open files*/

// Initialise to an invalid team number
team = 0; /*The current team number*/

LEVEL_INFO = 0;
LEVEL_OK = 1;
LEVEL_WARN = 2;
LEVEL_ERROR = 3;

TABLIST = new Array();

// Number that's incremented every time a new status message is displayed
status_num = 0;

// The project page
var projpage = null;
// The user
var user;
// The team selector
var team_selector;

// onload function
MochiKit.DOM.addLoadEvent( function() {
	//On page load - this replaces a onload action of the body tag
	//Hook up the save file button
	MochiKit.Signal.connect(window, 'onbeforeunload', beforeunload);

	cur_path = "";

	user = new User();
	var d = user.load_info();
	// Wait for the user information to come back
	d.addCallback( load_team_info );
	d.addErrback( function() { window.alert("Failed to get user info: TODO: replace this message mechanism"); } );
});

// 1) executed after the onload function
function load_team_info() {
	// Got the user information -- now get team information
	team_selector = new TeamSelector();

	team_selector.load(load_project_pane);
}

// 2) executed after team information has been acquired/changed
function load_project_pane() {
	if( projpage != null )
		projpage.destroy();
	else
		projpage = new ProjPage();

	projpage.show();
}

function beforeunload(e) {
	savecurrenttab();
	for (var tab in open_files) {
		if(!closetab(tab)){
			e.confirmUnload("You should close tabs before closing this window");
			break;
		}
	}
}

function checkout() {
	/*Checkout a list of files.
		TODO: Security munging. Path character escaping.
		inputs: none
		returns: none, but causes a file download by changing
			document.location to a source that provides an attachment*/
	//files is an array of paths of selected files
	var files = get_selected();
	if(files.length > 0){
		document.location = "./checkout?team=" + team + "&files=" + files.join(",");
	} else {
		alert("No files selected.");
	}
}

//OPEN AND SAVE FILES
function saveFile(e) {
	/*Save the current tab back to the subversion server.
		inputs: e - A load of data from MochiKit. Not used
		returns: none. Creates a deferred which in turn calls filesaved
	*/

	/* TODO: Figure out why this was here.
	if(open_files[cur_path].revision){
		alert("Invalid revision.");
		return;
	}*/

	document.body.style.cursor = "wait";

	//TODO: Check cur_path is valid

	//Put data from the current tab into its open_files entry
	savecurrenttab();

	//See if the file has been altered.
	if(open_files[cur_path].dirty ||
			(open_files[cur_path].editedfilename != cur_path)){

		//Disable the button
		MochiKit.DOM.getElement("savefile").disabled = true;

		//TODO:Cope with saving as a new file name!
		//Could have:
		//cur_path = MochiKit.DOM.getElement("filename").value;
		//But need to rename open_files data etc

		//TODO: When commit message in open_files, read it from there

		var keys = ["file", "rev", "message", "code", "team"];
		var values = [open_files[cur_path].editedfilename, //File
					  open_files[cur_path].revision, //rev
					  MochiKit.DOM.getElement("message").value, //message
					  open_files[cur_path].tabdata, //Code
					  team]; //Team
		var content = MochiKit.Base.queryString(keys, values);

		//Using doXHR (New in MochiKit 1.4) to do a post request
		var d = MochiKit.Async.doXHR("./savefile",
						   {"method" : "POST",
							"mimeType" : "application/x-www-form-urlencoded",
							"sendContent" : content,
							"headers" : {'Content-Type' :
							'application/x-www-form-urlencoded'}});
		d.addCallback(filesaved);
		d.addErrback(function (){
			alert("Error connecting to studentrobotics.org. Please refresh.");
			});

	} else {
		alert("File not changed, not saving.");
	}
}

// **** Status Bar ****

// Hide the status bar
function status_hide() {
    MochiKit.Style.setStyle( "status-span", {"display":"none"} );

    var s = MochiKit.DOM.getElement("status");
    s.className = "";
}

// Show the status bar with the given message, and prepend "warning" or "error"
function status_msg( message, level ) {
    switch(level) {
    case LEVEL_WARN:
	message = [ MochiKit.DOM.createDOM( "STRONG", null, "Warning: " ),
		    message ];
	break;
    case LEVEL_ERROR:
	message = [ MochiKit.DOM.createDOM( "STRONG", null, "Error: " ), 
		    message ];
	break;
    }

    return status_rich_show( message, level );
}

// Replace the status bar's content with the given DOM object
function status_rich_show( obj, level ) {
    var s = MochiKit.DOM.getElement("status");

    var o = MochiKit.DOM.createDOM( "SPAN", { "id" : "status-span",
					      "display" : "" }, obj );
    MochiKit.DOM.replaceChildNodes( "status", o );

    switch(level) {
    case LEVEL_INFO:
	s.className = "info";
	break;
    case LEVEL_OK:
	s.className = "ok";
	break;
    case LEVEL_WARN:
	s.className = "warn";
	break;
    default:
    case LEVEL_ERROR:
	s.className = "error";
	break;
    }

    // Give it a shake if it's not OK
    if( level > LEVEL_OK )
	MochiKit.Visual.shake(s);

    status_num ++;
    var close_f = MochiKit.Base.partial( status_close, status_num );

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
    var b = MochiKit.DOM.createDOM( "A", { "href" : "#" }, btext );
    MochiKit.Signal.connect( b, "onclick", bfunc );

    var m = [ message, " -- ", b ]

    return status_msg( m, level );
}

// ****Login Screen****

TAB_NONE = 0;
TAB_PROJECTS = 1;
TAB_LOGIN = 2;
TAB_EDIT = 3;
function tabChange(num) {

		switch(num) {
			case TAB_NONE:
				MochiKit.Style.setStyle('edit-mode', {'display':'none'});
				projpage.hide();
				MochiKit.Style.setStyle('grey-out', {'display':'none'});
				break;
			case TAB_PROJECTS:
				MochiKit.Style.setStyle('edit-mode', {'display':'none'});
				MochiKit.Style.setStyle('grey-out', {'display':'none'});
		    		projpage.show();
				break;
			case TAB_LOGIN:
				MochiKit.Style.setStyle('edit-mode', {'display':'none'});
				projpage.hide();
				MochiKit.Style.setStyle('grey-out', {'display':'block'});
				break;
			case TAB_EDIT:
				MochiKit.Style.setStyle('edit-mode', {'display':'block'});
				projpage.hide();
				MochiKit.Style.setStyle('grey-out', {'display':'none'});
				break;
		}
}

function startLogin(username, password) {
	var d = MochiKit.Async.loadJSONDoc("./verifylogin", {"usr" : username, "pwd" : password});

	var gotMetadata = function (meta) {
	    if (meta.login == 1) {
		  tabChange(1);
	    } else {
		  MochiKit.DOM.getElement("login-feedback").innerHTML = "Incorrect Username / Password";
	    }
	};

	var failMetadata = function (meta) {
		  MochiKit.DOM.getElement("login-feedback").innerHTML = "Could Not Contact Server";		
	};
	d.addCallbacks(gotMetadata, failMetadata);
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

	this.load_info = function() {
		// Return a deferred that fires when the data's ready
		var retd = new Deferred();
		var d = loadJSONDoc("./userinfo");
		
		d.addCallback( bind( this._got_info, this ) );

		// Pass on the failure -- our caller is more qualified to handle it  
		d.addErrback( bind( retd.errback, retd ) );

		this._info_deferred = retd;
		return this._info_deferred;
	}

	this._got_info = function( info ) {
		this.team_names = info["teams"];

		this.teams = [];
		for( var team_num in info["teams"] )
			this.teams.push( parseInt(team_num, 10) );

		this._settings = info["settings"];
		for( var k in this._settings ) {
			logDebug( k + " = " + this._settings[k] );
		}

 		this._info_deferred.callback(null);
	}

	this.get_setting = function(sname) {
		return this._settings[sname];
	}
};

function polled()
{
	/*Polling makes sure we're up to date with others changes.
	  This function run roughly every POLL_TIME ms by setTimeout
	  call in pollAction
	  inputs: None
	  returns: Nothing, but callback created*/
	poll_data["team"] = team;
	var j = MochiKit.Async.loadJSONDoc("./polldata", poll_data );
	j.addCallback(pollAction);
	j.addErrback(function (){
		alert("Error connecting to studentrobotics.org. Please refresh.");
		});
}

function pollAction(result)
{
	/*Data received from polling call. Process it, then set up a
	  timeout to call polled in POLL_TIME ms.
	  input: A dictionary of filenames -> revision numbers
				result[filename]["rev"] = N
	  returns: Nothing, but sets up timeout*/

	//For each file for which there is info available, if that file
	//is open and if the local working revision is less than that
	//saved on the server, then mark that file as changed

	for (var file in result["files"]){
		if(open_files[file])
			if(open_files[file].revision < result["files"][file]["rev"]){
				open_files[file].changed = true;
				var lr = "lr" + file;
				MochiKit.Visual.Highlight(lr, {'startcolor' : '#ffff99'});
			}
	}

	if(result["log"].length != undefined){
		if(result["log"][0]["rev"] > poll_data["logrev"]){
			rows = MochiKit.Base.map(buildLogTableEntry, result["log"]);
			MochiKit.DOM.insertSiblingNodesAfter("fltablehead", rows);
			poll_data["logrev"] = result["log"][0]["rev"];
		}
	}

	//Generate the tab list, in case formatting etc needs changing
	//to mark that a file has conflicts
	generatetablist();
	//Setup the next poll in POLL_TIME ms
	setTimeout( "polled()", POLL_TIME );
}

function TeamSelector() {
	this._onSelected = null;
	this._prompt = null;

	this.load = function(onSelect) {
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
			teambox.push( "Team:" );
			teambox.push( tsel );
		}

		// Span to hold the team name
		var tname = SPAN( { "id" : "teamname" }, null );
		teambox.push( tname );

		replaceChildNodes( $("teaminfo"), teambox );
		this._update_name();

		this._onSelected = onSelect;

		if( this._team_exists(team) )
			this._onSelected();
	}

	this._build_options = function() {
		var olist = [];

		for( t in user.teams ) {
			var props = { "value" : user.teams[t]};
			if( user.teams[t] == team )
				props["selected"] = "selected";
			
			olist.push( OPTION(null, user.teams[t]) );
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
		this._update_name();

		this._onSelected();
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
