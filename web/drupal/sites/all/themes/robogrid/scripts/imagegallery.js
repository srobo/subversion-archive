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
	$$('.item').each(function(item)
	{
		first = item.clone();
	});
	first.inject(drop);
});
