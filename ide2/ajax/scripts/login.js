window.addEvent('domready', function() {
	var morph = new Fx.Morph('navigate');
	var morph2 = new Fx.Morph('top-spacer');
	
	var navigate = $('navigate');
	var top = $('top-spacer');

	var scene2 = function(){
			morph.start.delay(2000, this, {
				width: '0px'});
			};
	var scene3 = function(){
			morph.start({
				display: 'none'});
			};

	var scene1 = function(){ 
			morph.start({
				width: '200px',
				display: 'block'}).chain(scene2).chain(scene3);
			};

	$('trigger').addEvent('click', scene1);
});

