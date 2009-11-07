function Admin() {
	//hold the tab object
	this.tab = null;

	//hold signals for the page
	this._signals = new Array();

	//hold status message for the page
	this._prompt = null;
}

/* *****	Initialization code	***** */
Admin.prototype.init = function() {
	if(!this._inited) {
		logDebug("Admin: Initializing");

		/* Initialize a new tab for Admin - Do this only once */
		this.tab = new Tab( "Administration" );
		connect( this.tab, "onfocus", bind( this._onfocus, this ) );
		connect( this.tab, "onblur", bind( this._onblur, this ) );
		connect( this.tab, "onclickclose", bind( this._close, this ) );
		tabbar.add_tab( this.tab );

		/* Initialise indiviual page elements */
		this.ShowTeams();

		/* remember that we are initialised */
		this._inited = true;
	}

	/* now switch to it */
	tabbar.switch_to(this.tab);
}
/* *****	End Initialization Code 	***** */

/* *****	Tab events: onfocus, onblur and close	***** */
Admin.prototype._onfocus = function() {
	setStyle($("admin-page"), {'display':'block'});
}

Admin.prototype._onblur = function() {
	/* Clear any prompts */
	if( this._prompt != null ) {
		this._prompt.close();
		this._prompt = null;
	}
	/* hide Admin page */
	setStyle($("admin-page"), {'display':'none'});
}

Admin.prototype._close = function() {
	/* Disconnect all signals */
	for(var i = 0; i < this._signals.length; i++) {
		disconnect(this._signals[i]);
	}
	this._signals = new Array();

	/* Close tab */
	this._onblur();
	this.tab.close();
	this._inited = false;
}
/* *****	End Tab events	***** */

/* *****	Team editing Code	***** */
Admin.prototype.ShowTeams = function() {
	var i = 0;
	var td_id = TH(null, 'Team ID');
	var td_name = TH({'class':'name'}, 'Team Name');
	var td_button = TH(null);
	var oddeven = i++ % 2 == 0 ? 'even' : 'odd';
	replaceChildNodes('admin-teams-table');
	appendChildNodes('admin-teams-table', TR({'class':oddeven}, td_id, td_name, td_button));

	for( var id in user.team_names ) {
		td_id = TD(null, id);
		td_name = TD({'class':'name'}, user.team_names[id]);
		var button = BUTTON(null, 'Edit');
		this._signals.push(connect(button, 'onclick', bind(this._editTeam, this, {id:id,name:user.team_names[id]})));
		td_button = TD(null, button);
		oddeven = i++ % 2 == 0 ? 'even' : 'odd';
		appendChildNodes('admin-teams-table',
			TR({'class':oddeven,id:'admin-teams-table-'+id}, td_id, td_name, td_button)
		);
	}
}

Admin.prototype._editTeam = function(ref) {
	var row = $('admin-teams-table-'+ref.id);
	var cell = getFirstElementByTagAndClassName('td', 'name', row);
	var button = getFirstElementByTagAndClassName('button', null, row);
	if(button.innerHTML == 'Edit') {
		var input = INPUT({value:cell.innerHTML});
		replaceChildNodes(cell, input);
		button.innerHTML = 'Save';
	} else {	//save it
		var d = loadJSONDoc("./admin/teamname", {
				'id':ref.id,
				'name':cell.firstChild.value
			});

		cell.firstChild.disabled = true;
		button.disabled = true;

		d.addCallback( bind( this._receiveEditTeam, this, ref) );
		d.addErrback( bind( this._errorEditTeam, this, ref) );
	}
}
Admin.prototype._receiveEditTeam = function(ref, nodes) {
	var row = $('admin-teams-table-'+ref.id);
	if(nodes.success) {
		this._prompt = status_msg("Team name updated", LEVEL_OK);
		var cell = getFirstElementByTagAndClassName('td', 'name', row);
		var button = getFirstElementByTagAndClassName('button', null, row);
		cell.innerHTML = nodes.name;
		button.innerHTML = 'Edit';
		button.disabled = false;
		user.team_names[nodes.id] = nodes.name;
		if(nodes.id == team) {
			$('teamname').innerHTML = nodes.name;
		}
	} else {
		this._errorEditTeam(ref, nodes);
	}
}
Admin.prototype._errorEditTeam = function(ref, nodes) {
	this._prompt = status_msg("Failed to update team name", LEVEL_ERROR);
	var row = $('admin-teams-table-'+ref.id);
	var cell = getFirstElementByTagAndClassName('td', 'name', row);
	var button = getFirstElementByTagAndClassName('button', null, row);
	cell.firstChild.disabled = false;
	button.disabled = false;
}
/* *****	End Team editing Code 	***** */

