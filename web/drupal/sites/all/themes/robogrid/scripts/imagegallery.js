/*
	imagegallery.js for mootools v1.2
	by Devin Ross (http://www.tutorialdog.com) - MIT-style license
*/

window.addEvent('domready', function()
{
	var drop = $('large');
	 
	$$('.item').each(function(item)
	{
		item.addEvent('click', function(e)
		{
			drop.removeEvents();
			drop.empty();
			var a = item.clone();
			a.inject(drop);
		});
	 
	});
	
	var first;
	var isFirst = 0;
	$$('.item').each(function(item)
	{
		if(isFirst == 0)
		{
			first = item.clone();		
		}
		isFirst++;
	});
	drop.removeEvents();
	drop.empty();
	first.inject(drop);
});