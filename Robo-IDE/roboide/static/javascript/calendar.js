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
    
    this.init();
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
    
    //highlight today's date
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
        this.logs = nodes.history;
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
					    file : this.proj, 
					    mnth : this.date.getMonth(),
					    yr : this.date.getFullYear()});

	d.addCallback( bind(this._receiveDates, this));	
	d.addErrback( bind(this._errorReceiveDates, this));    
}

//create a new array with one log entry per date (the last one from that day of that month)
Calendar.prototype.processDates = function() {
    var temp = new Array();
    //log array is ordered newest to oldest, so stop after the first
    
    //find out what is the last day in the month for which there are log entries
    var day = this.extract(this.logs[0].date).getDate();
    temp.push(this.logs[0]); 
    day--;   

    //now count down through the month
    for(var z=1; z < this.logs.length; z++) { 
        var now = this.extract(this.logs[z].date).getDate();   
        if(now > day) {
            //ignore (same day)
        }
        else {
            //moved to a new day, add to the new array
            temp.push(this.logs[z]); 
            day = now-1;             
        }
    }
    
    this.logdays = new Array();
    update(this.logdays, temp);
}

//use logdays to bring to life the cells on the Calendar which relate to log entries
Calendar.prototype.updateCal = function() {
    for(var i=0; i < this.logdays.length; i++) {
        var date = this.extract(this.logdays[i].date).getDate();
        setNodeAttribute($("cal"+date), "class", "td-log");
        setNodeAttribute($("cal"+date), "rev", this.logdays[i].rev);        
        setNodeAttribute($("cal"+date), "onclick", "alert('TODO: switch to revision: x')");
        setNodeAttribute($("cal"+date), "title", this.logdays[i].message);
    }
}

Calendar.prototype.changeMonth = function(dir) {
    this.date.setMonth(this.date.getMonth() + dir);
    this.init();
}
