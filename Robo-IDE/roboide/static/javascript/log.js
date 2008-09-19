function Log() {
	this.selectedRevision;
	this.team = 1;
	this.user = null;
	this.userList = new Array("chris", "rob", "pete");
	this.history = new Array();
	this.file = "/candy/fudge/bar.py";
	this._init();
	
/*
	selectedRevision [int]	holds revision number to revert to
	userList [string[]]		Array of user names in current team
	history					the array of dictionaries returned by the server
	file					the file(s) for which the logs are for

	_init()					Initialize function - clears variables and re-retrieves data from server
	_receiveHistory(rev)	Callback function for loadJSONDoc success
	_errorReceiveHistory()	Callback function for loadJSONDoc fail
	_retrieveHistory()		Instigates call to server to retrieve logs for given file(s) & users
	_populateList()			Takes history and updates list of logs on page, it also adds event handlers to log-menu buttons
	_update()			Event handler for when a new user is selected
*/
}


Log.prototype._init = function() {
	disconnectAll($("svn-users"));
	logDebug("Initializing Log...");
	this.history = new Array();
	this._retrieveHistory();
	display_log();
}

Log.prototype._receiveHistory = function(revisions) {
	logDebug("Log history received ok");
	update(this.history, revisions.history);
	status_msg("File History loaded successsfully", LEVEL_OK);	
	this._populateList();
}

Log.prototype._errorReceiveHistory = function() {
	logDebug("Log history retrieval failed");
	this.history = new Array();
	button_msg("Error retrieving history", LEVEL_WARN, "Retry", bind(this._receiveHistory, this));
}

Log.prototype._retrieveHistory = function() {
	var d = loadJSONDoc("./gethistory", { team : this.team,
					    file : this.file, user : this.user});

	d.addCallback( bind(this._receiveHistory, this));	
	d.addErrback( bind(this._errorReceiveHistory, this));
}
Log.prototype._populateList = function() {
	logDebug("Processing log list...");

	//print summary information
	var entries = this.history.length; 
	if(entries <= 0) {
		$("log-summary").innerHTML = "There are no revisions availble for file(s): "+this.path;
	}
	else {
		$("log-summary").innerHTML = "Displaying "+entries+ " revision(s) between "+this.history[this.history.length-1].date+" & "+this.history[0].date;
	}
	
	//TODO: get user list from server

	//Use userList to update list of users in dropdown box
	//clear list:
	replaceChildNodes($("svn-users"), opt);

	//first item in list is: 'Filter by user'always 'all' meaning, show logs from all users
	var opt = OPTION(null, "");
	opt.innerHTML = "Filter by user";
	opt.value = -1;
	appendChildNodes($("svn-users"), opt);

	//second item in the list is: 'all' meaning, show logs from all users
	var opt = OPTION(null, "");
	opt.innerHTML = "Show all";
	opt.value = -1;
	appendChildNodes($("svn-users"), opt);

	//now add all availble users in the team
	for(var i=0; i < this.userList.length; i++) {
		var opt = OPTION(null, "");
		opt.innerHTML = this.userList[i];
		opt.value = i;
		appendChildNodes($("svn-users"), opt);			
	}
	var obj = this;
	connect($("svn-users"), 'onchange', bind(this._update, this));

	//clear log list
	replaceChildNodes($("log-list"), null);
	//
	for(var x=0; x <this.history.length; x++) {
		var logtxt = SPAN("r"+this.history[x].rev+" | "+this.history[x].author+" | "+this.history[x].date);
		var item = LI(null, logtxt);
		var radio = INPUT({'type' : 'radio', 'name' : 'log', 'value' : this.history[x].rev });
		var commitMsg = DIV({'class' : 'commit-msg'}, this.history[x].message);
		appendChildNodes(item, radio);
		appendChildNodes(item, logtxt);	
		appendChildNodes(item, commitMsg);	
		appendChildNodes($("log-list"), item);	
	}	
} 

Log.prototype._update = function() {
	var index = $("svn-users").value;
	if(index > -1) { 
		this.user = this.userList[index];
	}
	else { 
		this.user = null; 
	}
	logDebug("Filter logs by user: "+this.user);
	this._init();
}

function display_log() {
	setStyle($("log-mode"), {"display" : "block"});
}

function hide_log() {
	setStyle($("log-mode"), {"display" : "none"});	
}
