var slides = new Array();
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

	for( var i=0; i<slides.length; i++ ) {
		slides[i].style.display = "none";
		slides[i].className = "realpage";
	}		

	curslide = 0;
	showSlide(curslide);	
	startTimer();
}

// Switch to new slide
function showSlide( nSlide )
{
	// Hide the old
	for(var i=0; i<slides.length; i++)
		slides[i].style.display = "none";

	// Show the new
	slides[nSlide].style.display = "";
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