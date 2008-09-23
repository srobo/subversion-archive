function Log(file, team) {
    //class properties
    this.tab = null;                    //holds reference to tab in tabbar
	this.selectedRevision = -1;         //Selected revision, -1 indicates no revision set
	this.team = team;                   //team number TODO: get this from elsewhere
	this.user = null;                   //holds author name
	this.userList = new Array();        //List of  users attributed to file(s)
	this.history = new Array();         //array of log entries returned by server
	this.file = file;                   //the file/directory for which we are interested
	this.offset = 0;                    //which results page we want to retrieve from the server
	this.overflow = 0;                  //stores  total number of results pages (retrieved from server)

    //do this only once: add a new tab to the tabbar and link it to this log page
    this.tab = new Tab("Log: "+this.file.toString());
    connect(this.tab, 'onfocus', bind(this._onfocus, this));
    connect(this.tab, 'onblur', bind(this._onblur, this));
    tabbar.add_tab(this.tab);
    tabbar.switch_to(this.tab);
    //start initialisation
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
    _onfocus()              event handler for tab click
    _onblur()               event handler for tab looses focus
    close()                 completely close the tab and log page
*/
}


Log.prototype._init = function() {
	logDebug("Initializing Log...");
    //clear data from previous query
	this.history = new Array();
    this.userList = new Array();
    //do new query
	this._retrieveHistory();
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
//processess log data and formats into list. connects up related event handlers, 
//deals with multile results pages
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
	//now populate log list
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
    disconnectAll($("revert"));
    connect($("revert"), 'onclick', bind(this._revert, this, false));

    //connect up the close button on log menu
    disconnectAll($("log-close"));
    connect($("log-close"), 'onclick', bind(this.close, this));
} 
//get older (updown > 0) or newer (updown < 0) results
Log.prototype._nextview = function(updown) {
	this.offset = this.offset+updown;
    //get new results page
	this._init();
}
//called when user applies author filter
Log.prototype._update = function() {
    //find out which author was selected  using select value as key to userList array
	var index = $("svn-users").value;
    //if user clicks 'All' (-1) clear user variable
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
//revert to selected revision. override = true to skip user confirmation
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
//tab gets focus
Log.prototype._onfocus = function() {
    if(getStyle($("log-mode"), "display") != "block") {
        this.display();
    }
    //don't waste time doing query again, just process results in buffer
    this._populateList();
}

//tab loses focus
Log.prototype._onblur = function() {
    this.hide();
}
//tab is closed
Log.prototype.close = function() {   
    this.tab.close(); 
    delete this;    //free memory      
    logDebug("Closing log tab");
}
//shows all log page specific htmll
Log.prototype.display = function() {
	setStyle($("log-mode"), {"display" : "block"});
}
//hides all log page specfic html
Log.prototype.hide = function() {
	setStyle($("log-mode"), {"display" : "none"});	
}

