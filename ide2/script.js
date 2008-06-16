function showHide(that)
{
	Target	= document.getElementById(that).style.display;

	if(Target == "none")
		document.getElementById(that).style.display	= "";
	else
		document.getElementById(that).style.display	= "none";

	return;
}