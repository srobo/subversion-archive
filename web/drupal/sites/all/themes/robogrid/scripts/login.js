window.addEvent('domready', function() {
	//var mylogingif = new Asset.image('images/srobo-login.gif', {id: 'login-img', title: 'ALoginGif'});

	var morph = new Fx.Morph('navigate');
	var morph2 = new Fx.Morph('top-spacer');
	
	var navigate = $('navigate');
	var top = $('top-spacer');

	var scene2 = function(){
			morph.start.delay(2000, this, {
				width: '0px',
				opacity: 0});
			};

	var scene1 = function(){ 
			morph.start({
				width: '200px',
				display: 'block',
				opacity: 1}).chain(scene2);
			top.set('html', '<img src=\'images/srobo-login.gif\' /> Logging you in...');
			(function(){top.set('html', 'You are now logged in')}).delay(2000, this);
			};

	$('trigger').addEvent('click', scene1);
	$('login').addEvent('submit', scene1);
});

