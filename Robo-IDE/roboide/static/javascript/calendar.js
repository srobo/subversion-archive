function Calendar(project) {

    //Default project is the root dir.
    if(project != null && project != undefined)
        this.proj = project;
    else
        this.proj = "/";     
    
    //holds the selected month & year          
    this.date = new Date();
    this.date.setDate(1);
    
    this.logs = new Array();       //will hold complete month of log entries
    this.logdays = new Array();    //will hold max of one log entry (the last) for each day in month
    
    //event signals
    this._signals = new Array();
    
    //when project changes we need to refresh the calendar
    connect(projpage._selector, "onchange", bind( this.change_proj, this));
}
    
var MONTHS = new Array("January", "February", "March", 
                        "April", "May", "June", "July", 
                        "August","September", "October", 
                        "November", "December");


Calendar.prototype.init = function() {
    this.logs = new Array();
    this.logdays = new Array();
    //do html 
    this.drawCal();
    //try ajax 
    this.getDates();
    //setup events
    
    map( disconnect, this._signals );
	this._signals = [];
    
    this._signals.push( connect("cal-prev-month", 
                                'onclick', 
                                 bind(this.changeMonth, this, -1) ) ); 
    this._signals.push( connect("cal-next-month", 
                                'onclick', 
                                 bind(this.changeMonth, this, +1) ) );     
}

Calendar.prototype.drawCal = function() {

    //Set month header
    $("cal-header").innerHTML = MONTHS[this.date.getMonth()]+" "+this.date.getFullYear();    
    
    //reset row, cell and day variables
    var td = 0;
    var tr = 0;
    var day = 1;
    
    //clear all cells from calendar
    for(tr=0; tr < 6; tr++) {
        replaceChildNodes("cal-row-"+tr);
    }
    
    //insert grey cells so we start on the correct day of the week 
    tr = 0;
    for(td= 0; td < 7; td++) {
        if(td < this.date.getDay()) {
            appendChildNodes($("cal-row-"+tr), TD({'class':'null'}, ""));
        }
        else {
            appendChildNodes($("cal-row-"+tr), TD({'id' : 'cal'+day}, day));
            day++;  
        }
    }   
    
    //now generate the rest of the cells in rows of 7 cells
    for(tr=1; tr < 6; tr++) {
        while(td < (7*(tr+1))){
            if(day <= this.dinm() ) {
                appendChildNodes($("cal-row-"+tr), TD({'id' : 'cal'+day}, day));
                day++;
            }
            else {
                //appendChildNodes($("cal-row-"+tr), TD(null, " "));  
            }
            td++; 
        }    
    } 
    
    //highlight today's date if we are showing current month and current year
    if( ( this.date.getMonth() == (new Date()).getMonth() ) && (this.date.getFullYear() == (new Date()).getFullYear() ) )
        setStyle("cal"+(new Date()).getDate(), {"border" : "1px solid #000000", "font-weight" : "bold"});       
}

//convert date string in log array into jscript date
Calendar.prototype.extract = function(datetime) {
    var parts = datetime.split("/");
    return new Date(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]);
}

//return the number of days in the week
Calendar.prototype.dinm = function() {
    return 32 - new Date(this.date.getFullYear(), this.date.getMonth(), 32).getDate();
}

//ajax handler for receiving logs from server
Calendar.prototype._receiveDates = function(nodes) {

    if(nodes.history.length > 0) {
        //convert string representation of date to javascript date object      
        this.logs = map(function(x) {
                var parts = x.date.split("/");
                var jsdate = new Date(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]);
                return { "date" : jsdate, 
                                    "message" : x.message, 
                                    "rev" : x.rev, 
                                    "author" : x.author};
        }, nodes.history);
        

        this.processDates();
        this.updateCal();        
    }
    else {
        this.logs = [];
        this.logdays = [];
        return;
    }
}

//ajax handler for failed requests
Calendar.prototype._errorReceiveDates = function() {
    logDebug("Error retrieving calendar dates");
}

//get month of logs messages from server
Calendar.prototype.getDates = function() {
	var d = loadJSONDoc("http://localhost:8080/calendar", { 
	                    team : 1,               //TODO Change this
					    file : "/"+this.proj, 
					    mnth : this.date.getMonth(),
					    yr : this.date.getFullYear()});

	d.addCallback( bind(this._receiveDates, this));	
	d.addErrback( bind(this._errorReceiveDates, this));    
}

//create a new array with one log entry per date (the last one from that day of that month)
Calendar.prototype.processDates = function() {  
    
    //blank array
    this.logdays = new Array();    
    
    //get array of days with corresponding log entries
    for(var z=0; z < this.logs.length; z++) { 
        var now = this.logs[z].date.getDate();   
        if(findValue(this.logdays, now) < 0) {
            this.logdays.push(now);
        }
    }   
}

//use logdays to bring to life the cells on the Calendar which relate to log entries
Calendar.prototype.updateCal = function() {
    for(var i=0; i < this.logdays.length; i++) {        
        setNodeAttribute($("cal"+this.logdays[i]), "class", "td-log");      
        connect($("cal"+this.logdays[i]), 
                    'onclick', 
                    bind(this.change_day, this, this.logdays[i]) );
        setNodeAttribute($("cal"+this.logdays[i]), "title", "Click to see revisions for this day");
    }
}

Calendar.prototype.changeMonth = function(dir) {
    this.date.setMonth(this.date.getMonth() + dir);
    replaceChildNodes("cal-revs", OPTION({"value" : -1}, "Select a date"));    
    this.init();
}

Calendar.prototype.change_day = function(target) {
    //alert use to select a revision
    replaceChildNodes("cal-revs", OPTION({"value" : -1}, "Select a revision"));
    
    //get logs for target date
    for(var i = 0; i < this.logs.length; i++) {
        if(this.logs[i].date.getDate() == target) {
            appendChildNodes("cal-revs", OPTION({ "value" : this.logs[i].rev}, 
                                            this.logs[i].author+": "+
                                            this.logs[i].message.slice(0, 20)+" ("+
                                            this.logs[i].date.getHours()+":"+
                                            this.logs[i].date.getMinutes()+")")); 
        }
    }
    
    disconnectAll("cal-revs");
    connect("cal-revs", 'onchange', bind(this._load_new_rev, this) );  
    
    status_msg("Now Select a project revision", LEVEL_OK);            
}

Calendar.prototype._load_new_rev = function() {
    var target = $("cal-revs").value;
       if(target >= 0) {
        projpage.flist.change_rev(target);
        status_msg("Now showing project at revision: "+target, LEVEL_OK);
    }   
}

Calendar.prototype.change_proj = function(project) {
    this.proj = project;
    this.date = new Date();
    this.date.setDate(1);
    this.logs = new Array();
    this.logdays = new Array();
    
    this.init();
}