ErrorsPage = function () {

	//hold the tab object
	this.tab = null;
	//hold signals for the page
	this.signals = new Array();
	//Array of ErrorFiles
	this.eflist = new Array();
	//status prompt ref
	this._prompt = null;

	//the info we're going to use to load with
	this._info = null;

	this._inited = false;

	this._init = function() {
		if(this._inited)
			return;

		this.tab = new Tab( "Errors" );
		connect( this.tab, "onfocus", bind( this._onfocus, this ) );
		connect( this.tab, "onblur", bind( this._onblur, this ) );

		tabbar.add_tab( this.tab );

		this.signals.push(connect("close-errors-page", "onclick", bind(this._close, this) ));
		this.signals.push(connect("switch-errors-page", "onclick", bind(this._switch, this) ));

		this._inited = true;
	}

	//load a new set of errors
	this.load = function(info, opts) {
		var path = info.path+'/';
		var filelist = new Array();
		this._init();
		log('Loading the ErrorPage');

		//if we've simply got an import or syntax error, not a dud function etc
		logDebug(this._module_name(info.file)+' | '+(info.messages[0] == this._module_name(info.file)+":1: NOT PROCESSED UNABLE TO IMPORT"));
		if(info.messages[0] == this._module_name(info.file)+":1: NOT PROCESSED UNABLE TO IMPORT") {
			//grab the error type
			var type = info.err[0].split(':')[0].slice(2, -5);
			logDebug('type:'+type+'|');

			switch(type) {
				case 'Syntax':
					var file = info.err[0].split('(')[1].split(', line ')[0];
					var line = info.err[0].split(', line ')[1].split(')')[0];
					var code = info.err[1];
					var marker = info.err[2];
					break;
				case 'Import':
					break;
				default:
					return;
			}
			
			if(info.whole) {	//if it was called from a code tab
				code += '';
			}

			logDebug('file: '+file+'| line: '+line+'| code: '+code+'| marker: '+marker+'|');

			if(this.eflist[file] == null)
				this.eflist[file] = new ErrorFile(file);

			this.eflist[file].load_error(type, line, code, marker);

		} else {
			for(var i = 0; i < info.messages.length; i++) {
				var msg = info.messages[i].split(':');
				var file = path+msg[0];

				//if we've not seen this file before, but it has a listing already, reset it
				if(findValue(filelist, file) == -1 && this.eflist[file] != null)
					this.eflist[file].reset();
				else {	//add it to our list, if it's null the initialise it
					filelist.push(file);
					if(this.eflist[file] == null)
						this.eflist[file] = new ErrorFile(file);
					log('file '+file+' has been pushed');
				}

				this.eflist[file].add_warn({ 'line' : msg[1], 'comment' : msg[2] });
			}
		}

		if(opts['switch_to'])
			tabbar.switch_to(this.tab);
	}

	this._module_name = function(n) {
		if(n.substr(-3) == '.py')
			return n.slice(0, -3);
		return n;
	}

	this._onfocus = function() {
		setStyle( $("errors-page"), {"display":"block"} );
	}

	this._onblur = function() {
		setStyle( $("errors-page"), {"display":"none"} );
	}

	this._close = function() {
		for( i in this.eflist ) {
			this.eflist[i].remove();
		}
		this.eflist = new Array();

		this.tab.close();

		for(var i = 0; i < this.signals; i++) {
			disconnect(this.signals[i]);
		}
		if(this._prompt != null)
			this._prompt.close();
		this._inited = false;
	}

	this._switch = function(filename) {
		for(var i=0; i < tabbar.tabs.length; i++)
		{
			if(tabbar.tabs[i].label == "robot.py") {
				tabbar.switch_to(tabbar.tabs[i]);
				return;
			}
		}
		this._prompt = status_msg("Couldn't switch to selected file: "+filename, LEVEL_ERROR);
		return;
	}

	this.show_only = function(file) {
		this.hide_all_files();
		this.eflist[file].show_errs();
	}

	this.hide_all_files = function() {
		for( i in this.eflist ) {
			this.eflist[i].hide_errs();
		}
	}
}

function ErrorFile(name) {
	//object for the errors in a file
	this.errors = new Array();
	//object for the warnings in a file
	this.warns = new Array();
	//the path of this file
	this.label = name;

	//the HTML element for the title
	this._name_elem = null;
	//the HTML element for the warnings
	this._warn_elem = null;
	//the HTML element for the errors
	this._err_elem = null;
	//the HTML element for all messages (errors and warnings)
	this._msgs_elem = null;
	//are the errors shown
	this._msgs_shown = true;

	//hold signals for the page
	this._signals = new Array();

	this._init = function() {
		logDebug('initing file: '+this.label);

		//make the html
		this._name_elem = createDOM('dt', null, this.label);
		this._warn_elem = UL(null, null);
		this._err_elem = UL(null, null);
		this._msgs_elem = createDOM('dd', {'file':this.label}, this._warn_elem, this._err_elem);

		//add the html to the page
		appendChildNodes("errors-listing", this._name_elem);
		appendChildNodes("errors-listing", this._msgs_elem);

		//hook up the signal
		this._signals.push(connect( this._name_elem, 'onclick', bind(this._name_onclick, this) ));
	}

	this.add_warn = function(w) {
		this.warns[w.line] = w.comment;
		appendChildNodes( this._warn_elem, LI({"line" : w.line}, ''+w.line+':'+w.comment) );
		this.show_msgs();
	}

	this.load_error = function(type, line, code, marker) {
		appendChildNodes( this._err_elem, LI(null, type+'\n'+line+': '+code+'\n'+marker) );
		this.show_msgs();
	}

	this.reset = function() {
		log('reseting: '+this.label);
		this.remove();
		this.init();
	}

	this.remove = function() {
		this.clear_msgs();
		removeElement(this._name_elem);
		this._name_elem = null;
	}

	this.clear_warns = function() {
		removeElement(this._warn_elem);
		this._warn_elem = null;
		this.warns = new Array();
	}

	this.clear_errors = function() {
		removeElement(this._err_elem);
		this._err_elem = null;
		this.errors = new Array();
	}

	this.clear_msgs = function() {
		this.clear_errors();
		this.clear_warns();
		removeElement(this._msgs_elem);
		this._msgs_elem = null;
	}

	this._name_onclick = function() {
		if(!this.msgs_shown)
			this.show_msgs();
		else
			this.hide_msgs();
	}

	this.show_msgs = function() {
		if(!this.msgs_shown) {
			setStyle( this._msgs_elem, {"display":''} );
			this.msgs_shown = true;
		}
	}

	this.hide_msgs = function() {
		if(this.msgs_shown) {
			setStyle( this._msgs_elem, {"display":'none'} );
			this.msgs_shown = false;
		}
	}

	this._init();
}