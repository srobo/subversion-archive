function calendar(month, year, logs) {
    this.month = month;
    this.year = year;
    this.logs = logs;
    this.date;
    this.init();
}
    // 13:16:20 23/09/2008
    
var MONTHS = new Array("January", "February", "March", "April", "May", "June", "July", "August","September", "October", "November", "December");

calendar.prototype.init = function() {
    var myDate = new Date();
    myDate.setMonth(this.month);
    myDate.setYear(this.year);
    myDate.setDate(1);
    
    $("cal-header").innerHTML = MONTHS[myDate.getMonth()];    
    var td = 0;
    var tr = 0;
    var day = 1;
    //blankrows
    for(tr=0; tr < 6; tr++) {
        replaceChildNodes("cal-row-"+tr);
    }
    
    //row 1 
    tr = 0;
    for(td= 0; td < 7; td++) {
        if(td < myDate.getDay()) {
            appendChildNodes($("cal-row-"+tr), TD({'class':'null'}, ""));
        }
        else {
            appendChildNodes($("cal-row-"+tr), TD(null, day));
            day++;  
        }
    }   
    for(tr=1; tr < 6; tr++) {
        while(td < (7*(tr+1))){
            if(day <= this.dinm() ) {
                appendChildNodes($("cal-row-"+tr), TD(null, day));
                day++;
            }
            else {
                //appendChildNodes($("cal-row-"+tr), TD(null, " "));  
            }
            td++; 
        }    
    }    
}

calendar.prototype.extract = function(datetime) {

    var time = datetime.split(" ")[0];
    var date = datetime.split(" ")[1];
    var month = parseInt(date.split("/")[1], 10);
    
    this.date = new Date(MONTHS[month]+" "+date.split("/")[0]+", "+ date.split("/")[2]+" "+time);
}

calendar.prototype.dinm = function() {
    return 32 - new Date(this.year, this.month, 32).getDate();
}

