cur_path = ""; //TODO: Replace these with cur_tab

open_files = {}; //A dictionary (hash table) of the currently open
//files - one for each tab

//POLLING
POLL_TIME = 2000; //In ms.
poll_data = {}; /*The data to be shipped with the poll.
files : comma seperated list of open files*/

team = 0; /*The current team number*/

project = "";

LEVEL_INFO = 0;
LEVEL_OK = 1;
LEVEL_WARN = 2;
LEVEL_ERROR = 3;

TABLIST = new Array();

// Number that's incremented every time a new status message is displayed
status_num = 0;

var projpage;

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

MochiKit.DOM.addLoadEvent( function() {
	//On page load - this replaces a onload action of the body tag
	//Hook up the save file button
	//MochiKit.Signal.connect('savefile','onclick', saveFile);
	MochiKit.Signal.connect(window, 'onbeforeunload', beforeunload);

	//Create an emptyish tab
	open_files[""] = {"revision" : "0",
					  "name" : "New",
					  "tabdata" : "",
					  "dirty" : false,
					  "editedfilename" : "",
					  "changed" : false,
					  "system" : true};

	//Create a tab for the Log
	open_files["Log"] = {"revision" : "0",
						 "name" : "Log",
						 "tabdata" : "",
						 "dirty" : false,
						 "editedfilename" : "",
						 "changed" : false,
						 "system" : true};

	cur_path = "";
	projpage = new ProjPage();


});

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

