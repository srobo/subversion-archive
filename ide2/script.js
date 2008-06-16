function showHide(that)
{
	Target	= document.getElementById(that).style.display;

	if(Target == "none")
		document.getElementById(that).style.display	= "";
	else
		document.getElementById(that).style.display	= "none";

	return;
}

function Validate_On_Contact_Submit (form_name)
{
	var out	= "form_name = "+form_name;
	FORM	= document.forms[form_name];
//	alert(out);

	if(/(^$)|(^\s+$)/.test(FORM.subject.value))
	{
		setfocus(FORM.subject);
		alert ( "Please enter a subject for your message" );
		return false;
	}
	//	null strings and spaces only strings not allowed
	if(/(^$)|(^\s+$)/.test(FORM.message.value))
	{
		setfocus(FORM.message);
		alert ( "Please enter a message" );
		return false;
	}
	FORM.contact_submit.disabled	= "true";	//make sure they don't hit send twice

	return true;
}