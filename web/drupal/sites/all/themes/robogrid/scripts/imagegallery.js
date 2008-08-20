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
	
	var first = 0;
	$$('.item').each(function(item)
	{
		if(first == 0)
		{	
			first = item;
		}
	}
	first.inject(drop);
});
