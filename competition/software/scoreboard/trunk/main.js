var slides = new Array();
var curslide;
var sTimeout;

// Starts everything off
function startShow()
{
	// Grab all the slide divs
	var divs = document.getElementsByTagName("div");

	for (var i=0; i<divs.length; i++) {
		if( divs[i].className == "page" ) {
			var s = { div: divs[i] };
			slides.push(s);
		}
	}

	// Make all slides invisible and switch them to the right class
	for( var i=0; i<slides.length; i++ ) {
		slides[i].div.style.display = "none";
		slides[i].div.className = "realpage";
	}

	loadSlide(0);

	curslide = 0;
	showSlide(curslide);	
	startTimer();

	updateTime();
}

// Switch to new slide
function showSlide( nSlide )
{
	// Hide the old
	for(var i=0; i<slides.length; i++)
		slides[i].div.style.display = "none";

	// Show the new
	slides[nSlide].div.style.display = "";

	// Update the title
	tcell = document.getElementById("titlecell");
	tcell.innerHTML = slideTitle( nSlide );

	// Update the next slide
	var nextSlide = (nSlide + 1) % slides.length;
	loadSlide( nextSlide );
}

// Call the update function for slide number n
function loadSlide(n)
{
	try {
		fname = slides[n].div.id + "Up";
		if( slides[n].div.id != "" )
			eval( fname + "(slides[" + n + "]);" );
	}
	catch(err) {}
}


// Advance onto the next slide
function changeSlide()
{
	curslide = (curslide + 1) % slides.length;

	showSlide(curslide);
	startTimer();
}

// Start the timer to the next slide
function startTimer()
{
	sTimeOut = setTimeout("changeSlide();", 3000);
}

// Update the clock
function updateTime()
{
	cell = document.getElementById("time");
	
	d = new Date();
	hour = d.getHours();
	minute = d.getMinutes();

	if( minute < 10 )
		minute = "0" + minute;
	if( hour < 10 )
		hour = "0" + hour;

	cell.innerHTML = hour + ":" + minute;

	setTimeout("updateTime()", 1000);
}

// Find slide number n's title
function slideTitle(n)
{
	children = slides[n].div.childNodes;
	for( var i=0; i < children.length; i++ ) {
		if( children[i].className == "title" )
			return children[i].innerHTML;
	}
}

// *** Slide update functions ***
function upcoming_matchesUp(slide)
{
	d = MochiKit.Async.doSimpleXMLHttpRequest( "./info/upcoming.php" );
	d.addCallback(upcoming_matches_cb,slide);
	d.addErrback(upcoming_matches_err);
}

function upcoming_matches_cb(slide,res)
{
	var resList = MochiKit.Async.evalJSONRequest(res);

	// Create the rows
	rows = [];
	for (var i in resList.matches) {
		var match = resList.matches[i];
		var r = [];

		r.push( TD({"class":"matchNum"}, match.number) );
		r.push( TD({"class":"matchTime"}, match.time) );
		
		for( var j = 0; j < 4; j++ )
			r.push( TD({"class":"matchTeam"}, match.teams[j]) );
		
		rows.push(TR(null, r));
	}

	var mt = document.getElementById("matchtable");

	var t = TABLE({"id":"matchtable"},
		      THEAD(null,
			    TR( {"class":"header"},
				[ TH( {"id":"matchMatchHeader"}, "Match" ),
				  TH( {"id":"matchTimeHeader"}, "Time" ),
				  TH( {"colspan":4, "id":"matchTeamsHeader"},  "Teams" ) ] ) ),
		      TBODY(null,rows ) );

	MochiKit.DOM.swapDOM(mt, t);
}

function upcoming_matches_err()
{
}

function scoresUp(slide)
{
	d = MochiKit.Async.doSimpleXMLHttpRequest( "./info/scores.php" );
	d.addCallback(scores_cb,slide);
	d.addErrback(scores_err);
}

function scores_cb(slide,res)
{
	var resList = MochiKit.Async.evalJSONRequest(res);

	// Create the rows
	rows = [];
	var position = 1;
	for (var i in resList.scores) {
		var score = resList.scores[i];
		var r = [];

		r.push( TD({"class":"scorePosition"}, position) );
		r.push( TD({"class":"scoreTeam"}, score.team) );
		r.push( TD({"class":"scorePoints"}, score.points) );
		
		rows.push(TR(null,r));

		position++;
	}

	var mt = document.getElementById("scorestable");

	var t = TABLE({"id":"matchtable"},
		      THEAD(null,
			    TR( {"class":"header"},
				[ map(partial(TH,null), ["Position", "Team", "Score"]) ] ) ),
		      TBODY(null, rows) );

	MochiKit.DOM.swapDOM(mt, t);
}

function scores_err()
{
}
