window.addEvent('domready', function() {
	var morph = new Fx.Morph('navigate');
	var morph2 = new Fx.Morph('top-spacer');
	
	var navigate = $('navigate');
	var top = $('top-spacer');

	$('trigger').addEvent('click', function() {
		morph.start({
			width: '300px',
			display: 'block'
		}).chain(function(){
			// executes immediately after completion of above effect
			this.start({
				width: '0px'
			});
		});
	});
});

