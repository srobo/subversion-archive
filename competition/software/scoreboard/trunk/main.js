var slides = new Array();
var slideData = new Array();
var curslide;
var sTimeout;

function startShow()
{
	// Grab all the slide divs
	var divs = document.getElementsByTagName("div");

	for (var i=0; i<divs.length; i++) {
		if( divs[i].className == "page" )
			slides.push( divs[i] );
	}

	// Make all slides invisible and switch them to the right class
	for( var i=0; i<slides.length; i++ ) {
		slides[i].style.display = "none";
		slides[i].className = "realpage";
	}		

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
		slides[i].style.display = "none";

	// Show the new
	slides[nSlide].style.display = "";

	// Update the title
	tcell = document.getElementById("titlecell");
	tcell.innerHTML = slideTitle( nSlide );

	// Call all other slide update routines
	for(var i=0; i<slides.length; i++) {
		// Skip the current slide
		if( i==nSlide ) continue;
		
		try {
			fname = slides[i].id + "Up";
			if( slides[i].id != "" )
				eval( fname + "();" );
		} 
		catch(err) {}
	}
}

function changeSlide()
{
	curslide = (curslide + 1) % slides.length;

	showSlide(curslide);
	startTimer();
}

function startTimer()
{
	sTimeOut = setTimeout("changeSlide();", 15000);
}

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
	children = slides[n].childNodes;
	for( var i=0; i < children.length; i++ ) {
		if( children[i].className == "title" )
			return children[i].innerHTML;
	}
}


// *** Slide update functions ***
function upcoming_matchesUp(slide)
{
	
}

function scoresUp(slide)
{

}

