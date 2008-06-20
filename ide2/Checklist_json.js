var xmlHttp;
var DEST;
var LIST;

function stateChanged()
{
	if(xmlHttp.readyState == 4)
		eval(xmlHttp.responseText);
}

function GetXmlHttpObject()
{
	var xmlHttp	= null;
	try
	{
		// Firefox, Opera 8.0+, Safari
		xmlHttp=new XMLHttpRequest();
	}
	catch(e)
	{
		// Internet Explorer
		try
		{
			xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
		}
		catch(e)
		{
			xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
		}
	}
	return xmlHttp;
}

function get_checklist(search)
{
	xmlHttp	= GetXmlHttpObject();
	DEST	= folder_id;

	if(xmlHttp == null)
	{
		alert("Your browser does not support AJAX!");
		window.location	+= (window.location.search == '' ? "?" : "&") + "ajax=0&search="+search;
	}

	xmlHttp.onreadystatechange	= stateChanged;
	xmlHttp.open("GET", "get_checklist.php?search=" + search, true);
	xmlHttp.send(null);
}
