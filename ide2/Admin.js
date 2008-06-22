/*	script inspired by "True Date Selector"
	 Created by: Lee Hinder, lee.hinder@ntlworld.com

	 Tested with Windows IE 6.0
	 Tested with Linux Opera 7.21, Mozilla 1.3, Konqueror 3.1.0
*/

function daysInFebruary (year)
{
	//	February has 28 days unless the year is divisible by four, and if it is the turn of the century
	//	then the century year must also be divisible by 400 when it has 29 days
	return (((year % 4 == 0) && ( (!(year % 100 == 0)) || (year % 400 == 0))) ? 29 : 28 );
}

//function for returning how many days there are in a month including leap years
function DaysInMonth(WhichMonth, WhichYear)
{
	var DaysInMonth = 31;
	if (WhichMonth == "4" || WhichMonth == "6" || WhichMonth == "9" || WhichMonth == "11")
	DaysInMonth = 30;
	if (WhichMonth == "2")
	DaysInMonth = daysInFebruary( WhichYear );
	return DaysInMonth;
}

//function to change the available days in a months
function ChangeOptionDays(formObj, prefix)
{
	var DaysObject = eval("formObj." + prefix + "day" );
	var MonthObject = eval("formObj." + prefix + "month" );
	var YearObject = eval("formObj." + prefix + "year" );

	if (DaysObject.selectedIndex && DaysObject.options)
	{ //	The DOM2 standard way
		//	alert("The DOM2 standard way" );
		var DaySelIdx = DaysObject.selectedIndex;
		var Month = parseInt(MonthObject.options[MonthObject.selectedIndex].value);
		var Year = parseInt(YearObject.options[YearObject.selectedIndex].value);
	}
	else if (DaysObject.selectedIndex && DaysObject[DaysObject.selectedIndex])
	{ //	The legacy ETBS way
		//	alert("The legacy ETBS way" );
		var DaySelIdx = DaysObject.selectedIndex;
		var Month = parseInt(MonthObject[MonthObject.selectedIndex].value);
		var Year = parseInt(YearObject[YearObject.selectedIndex].value);
	}
	else if (DaysObject.value)
	{ //	Opera 6 stores the selectedIndex in property 'value'.
		//	alert("The Opera 6 way" );
		var DaySelIdx = parseInt(DaysObject.value);
		var Month = parseInt(MonthObject.options[MonthObject.value].value);
		var Year = parseInt(YearObject.options[YearObject.value].value);
	}

	//	alert("Day="+(DaySelIdx+1)+" Month="+Month+" Year="+Year);

	var DaysForThisSelection = DaysInMonth(Month, Year);
	var CurrentDaysInSelection = DaysObject.length;
	if (CurrentDaysInSelection > DaysForThisSelection)
	{
		for (i=0; i<(CurrentDaysInSelection-DaysForThisSelection); i++)
		{
			DaysObject.options[DaysObject.options.length - 1] = null
		}
	}
	if (DaysForThisSelection > CurrentDaysInSelection)
	{
		for (i=0; i<DaysForThisSelection; i++)
		{
			DaysObject.options[i] = new Option(eval(i + 1));
		}
	}
	if (DaysObject.selectedIndex < 0)	DaysObject.selectedIndex = 0;
	if (DaySelIdx >= DaysForThisSelection)
		DaysObject.selectedIndex = DaysForThisSelection-1;
	else
		DaysObject.selectedIndex = DaySelIdx;
}

// Validate the Admin form before submission to check for errors
function Validate_On_Admin_Submit(form_name)
{
	var out	= "form_name = "+form_name;
	FORM	= document.forms[form_name];
//	alert(out);

	//	null strings and spaces only strings not allowed in either case
	if(/(^$)|(^\s+$)/.test(FORM.title.value))
	{
		setfocus(FORM.title);
		alert("Please enter a title for the task");
		return false;
	}
	if(/(^$)|(^\s+$)/.test(FORM.description.value))
	{
		setfocus(FORM.description);
		alert("Please enter a description");
		return false;
	}
	if(/(^$)|(^\s+$)/.test(FORM.design_consider.value))
	{
		setfocus(FORM.design_consider);
		alert("Please enter some design considerations");
		return false;
	}
	if(/(^$)|(^\s+$)/.test(FORM.related_docs_help.value))
	{
		setfocus(FORM.message);
		alert("Please enter some related documents that will help the students");
		return false;
	}
	FORM.contact_submit.disabled	= "true";	//make sure they don't hit send twice
	return true;
}

// use an ajax request to change the selected task, or if no ajax use php
function get_new_task(form_name, id)
{
	var out	= "form_name = "+form_name;
	var FORM	= document.forms[form_name];
//	alert(out);

	if(id == "New")
	{
		FORM.title.value	= "";
		FORM.description.value	= "";
		FORM.design_consider.value	= "";
		FORM.related_docs_help.value	= "";
		return;
	}

	var change_form	= function() {
		if(xmlHttp.readyState == 4)
			eval(xmlHttp.responseText);
		};

	send_ajax("Admin_ajax_helper.php", "edit_id="+id, change_form);

	return;
}