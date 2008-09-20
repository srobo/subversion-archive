function Log(file) {
	this.selectedRevision = -1;
	this.team = 1;
	this.user = null;
	this.userList = new Array();
	this.history = new Array();
	this.file = file;
	this.offset = 0;
	this.overflow = 0;
	this._init();
	
/*
	selectedRevision [int]	holds revision number to revert to
	userList [string[]]		Array of users who have made revisions to the selected  file(s)
	history					the array of dictionaries returned by the server
	file					the file(s) for which the logs are for
    user                    holds the name of the user who's revisions have been selected
    overflow                holds the total number of page of results for  the latest query
    offset                  holds the page offset for the next query

	_init()					Initialize function - clears variables and re-retrieves data from server
	_receiveHistory(rev)	Callback function for loadJSONDoc success
	_errorReceiveHistory()	Callback function for loadJSONDoc fail
	_retrieveHistory()		Instigates call to server to retrieve logs for given file(s) & users
	_populateList()			Takes history and updates list of logs on page, it also adds event handlers to log-menu buttons
	_update()			    Event handler for when a new user is selected
    _revert(bool)           event handler for when user clicks 'revert' - if arg is false, confirmation is  requested from user
    _nextview(int)          event handler for when user clicks on 'older'/'newer' buttons, 
                            if int > 0 an older page of  results is retrieved, if int <0 a later page of results is retrieved
*/
}


Log.prototype._init = function() {
	logDebug("Initializing Log...");
    //clear data from previous query
	this.history = new Array();
    this.userList = new Array();
    //do new query
	this._retrieveHistory();
	display_log();
}

Log.prototype._receiveHistory = function(revisions) {
	logDebug("Log history received ok");
    //extract data from query response
	update(this.history, revisions.history);
    update(this.userList, revisions.authors);
	this.overflow = revisions.overflow;
	status_msg("File History loaded successsfully", LEVEL_OK);	
    //present data
	this._populateList();
}

Log.prototype._errorReceiveHistory = function() {
    //handle failed request
	logDebug("Log history retrieval failed");
	this.history = new Array();
	button_msg("Error retrieving history", LEVEL_WARN, "Retry", bind(this._receiveHistory, this));
}

Log.prototype._retrieveHistory = function() {
	var d = loadJSONDoc("./gethistory", { team : this.team,
					    file : this.file, 
					    user : this.user,
					    offset : this.offset});

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
		$("log-summary").innerHTML = "Displaying "+entries+ " revision(s) between "+this.history[this.history.length-1].date+" & "+this.history[0].date+" Page "+(this.offset+1)+" of "+(this.overflow);
	}
	
	//fill drop down box with authors attributed to file(s)
	//clear list:
	replaceChildNodes($("svn-users"), opt);

	//first item in list is: 'Filter by user'
	var opt = OPTION(null, "");
	opt.innerHTML = "Filter by user";
	opt.value = -1;
	appendChildNodes($("svn-users"), opt);

	//second item in the list is: 'all' meaning, show logs from all users
	var opt = OPTION(null, "");
	opt.innerHTML = "Show all";
	opt.value = -1;
	appendChildNodes($("svn-users"), opt);

	//now add all attributed authors
	for(var i=0; i < this.userList.length; i++) {
		var opt = OPTION(null, "");
		opt.innerHTML = this.userList[i];
		opt.value = i;
		appendChildNodes($("svn-users"), opt);			
	}
    //add event handler for when user applies filter to results 
	disconnectAll($("svn-users"));
	connect($("svn-users"), 'onchange', bind(this._update, this));

	//clear log list
	replaceChildNodes($("log-list"), null);
	//now populate list of query results
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

    //if older results are available, enable the 'older' button	
	if(this.offset < (this.overflow-1)) {
 		disconnectAll($("older"));
		connect($("older"), 'onclick', bind(this._nextview, this, +1));
	}
	else {
		disconnectAll($("older"));
	}
    //if newer results are available, enable the 'newer' button	
	if(this.offset > 0) {
 		disconnectAll($("newer"));
		connect($("newer"), 'onclick', bind(this._nextview, this, -1));		
	}
	else {
		disconnectAll($("newer"));
	}
    //connect up the 'Revert' button to event handler
    connect($("revert"), 'onclick', bind(this._revert, this, false))
} 
Log.prototype._nextview = function(updown) {
    //get older or newer results
	this.offset = this.offset+updown;
	this._init();
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
    //reset offset
    this.offset = 0;
	this._init();
}

Log.prototype._revert = function(override) {
    //find out which radio button is checked
    for(var x=0; x < document.log.log.length; x++) {
        if(document.log.log[x].checked == true) {
            this.selectedRevision = document.log.log[x].value;
            break;
        }    
    }   
    if(this.selectedRevision < 0) {
        //no revision selected
        status_msg("No revision selected !", LEVEL_WARN);
    }
    else if(override){
        //user has confirmed revert, proceed
        status_msg("Reverting to revision: "+this.selectedRevision, LEVEL_OK);
    }
    else {
        //user has not confirmed revert, seek confirmation
        status_button("Are you sure you want to revert selected file(s)?", LEVEL_WARN, "Yes", bind(this._revert, this, true));
    }   

}
function display_log() {
	setStyle($("log-mode"), {"display" : "block"});
}

function hide_log() {
	setStyle($("log-mode"), {"display" : "none"});	
}
