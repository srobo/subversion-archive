/*
	imagegallery.js for mootools v1.2
	by Devin Ross (http://www.tutorialdog.com) - MIT-style license
*/

window.addEvent('domready', function()
{
	var drop = $('large');
	var dropFx = drop.effect('background-color', {wait: false}); // wait is needed so that to toggle the effect,
	 
	$$('.item').each(function(item)
	{
		item.addEvent('click', function(e)
		{
			drop.removeEvents();
			drop.empty();
			var a = item.clone();
			a.inject(drop);
			dropFx.start('7389AE').chain(dropFx.start.pass('ffffff', dropFx));
			
		});
	 
	});
});