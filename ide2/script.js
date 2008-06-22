function showHide(that)
{
	Target	= document.getElementById(that).style.display;

	if(Target == "none")
		document.getElementById(that).style.display	= "";
	else
		document.getElementById(that).style.display	= "none";

	return;
}

/* set the focus to a specific input */
function setFocusDelayed(global_valfield)
{
	global_valfield.focus();
}

function setfocus(valfield)
{
	// save valfield in global variable so value retained when routine exits
	global_valfield = valfield;
	setTimeout( 'setFocusDelayed(global_valfield)', 100 );
}

function Validate_On_Contact_Submit(form_name)
{
	var out	= "form_name = "+form_name;
	FORM	= document.forms[form_name];
//	alert(out);

	//	null strings and spaces only strings not allowed in either case
	if(/(^$)|(^\s+$)/.test(FORM.subject.value))
	{
		setfocus(FORM.subject);
		alert("Please enter a subject for your message");
		return false;
	}
	if(/(^$)|(^\s+$)/.test(FORM.message.value))
	{
		setfocus(FORM.message);
		alert("Please enter a message");
		return false;
	}
	FORM.contact_submit.disabled	= "true";	//make sure they don't hit send twice
	return true;
}

//ajax bits follow
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

//sends an ajax request to address 'addr', using search_string, then evalutaes the function 'userfunc', which can be defined when calling this function
function send_ajax(addr, search_string, userfunc)
{
	xmlHttp	= GetXmlHttpObject();

	if(xmlHttp == null)
	{
		alert("Your browser does not support AJAX!");
		window.location	+= (window.location.search == '' ? "?" : "&") + "ajax=0&"+search_string;
	}

	xmlHttp.onreadystatechange	= userfunc;
	xmlHttp.open("GET", addr + "?" + search_string, true);
	xmlHttp.send(null);
}
