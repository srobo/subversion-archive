// Tab: A single tab.
function Tab(label) {
	this.label = label;

	// Whether this tab can be focussed
	// Setting this to false prevents focus being shifted to the tab,
	// and clicking on it will only trigger the "onclick" signal.
	this.can_focus = true;

	// The list item that we are
	this._li = null;
	// The link
	this._a = null;

	// Whether we have focus
	this._focus = false;

	this._init = function() {
		this._a = A( {"class": "nofocus"}, this.label );
		this._li = LI(null, this._a );

		appendChildNodes($("tab-list"), this._li);

		connect(this._li, 'onclick', bind(this._onclick,this) );
	}

	this._onclick = function(ev) {
		ev.preventDefault();
		ev.stopPropagation();

		signal( this, "onclick", this );
	}

	// Called to tell the tab it has focus
	this.got_focus = function() {
		removeElementClass( this._a, "nofocus" );
		addElementClass( this._a, "focus" );

		if( !this._focus ) {
			logDebug( "tab \"" + this.label + "\" focussed" );
			signal( this, "onfocus", this );
		}

		this._focus = true;
	}

	// Called to tell the tab it no longer has focus
	this.lost_focus = function() {
		addElementClass( this._a, "nofocus" );

		if( this._focus ) {
			logDebug( "tab \"" + this.label + "\" blurred" );
			signal( this, "onblur", this );
		}

		this._focus = false;
	}

	this.close = function() {
		signal( this, "onclose", this );

		disconnectAll(this);
		removeElement( this._li );
	}

	this.flash = function() {
		Highlight( this._a, 
				{ 'startcolor' : '#ffff31',
				  'endcolor' : this._a.style.backgroundColor,
				  'afterFinish' : bind(this._remove_flash_style, this)
				});
	}

	this._remove_flash_style = function() {
		status_msg( "TODO: Remove flash style", LEVEL_WARN );
	}

	this.set_label = function( l ) {
		this.label = l;
		replaceChildNodes(this._a, l);
	}

	this._init();
}


// TabBar: Managers tabs
// Calls show and hide functions as appropriate
function TabBar() {
	this.tabs = [];

	// Current tab with focus
	this._curtab = null;

	// Member functions
	this.add_tab = function( tab ) {
		this.tabs.push( tab );
		connect( tab, "onclick", bind( this._onclick, this ) );
		connect( tab, "onclose", bind( this._onclose, this ) );
	}
	
	this.switch_to = function( tab ) {
		// Don't do anything if the tab can't focus
		if( !tab.can_focus )
			return;

		if( tab != this._curtab ) {
			if( this._curtab != null )
				this._curtab.lost_focus();
			tab.got_focus();

			var fromtab = this._curtab;
			this._curtab = tab;

			// Signal that the tab has been switched
			signal( this, "onswitch", fromtab, tab );
		}
	}

	// Force the tab to be unfocussed and then focussed
	this.force_refresh = function( tab ) {
		// Don't do anything if the tab can't focus
		if( !tab.can_focus )
			return;

		if( tab._curtab == tab )
			tab._curtab = null;

		this.switch_to( tab );
	}

	// Handler for tab onclick events
	this._onclick = function( tab ) {
		this.switch_to( tab );
	}

	// Handler for closing a tab
	this._onclose = function( tab ) {
		var index = 0;

		for( var i in this.tabs ) {
			if( this.tabs[i] == tab ) {
				index = i;
				this.tabs.splice( i, 1 );
				break;
			}
		}

		index--;
		if( index < 0 ) index = 0;
		
		// Try tabs to the left:
		for( var i = index; i >= 0; i-- )
			if( this.tabs[i].can_focus ) {
				index = i;
				break;
			}

		// Try tabs to the right:
		for( var i = index; i < this.tabs.length; i++ ) {
			if( this.tabs[i].can_focus ) {
				index = i;
				break;
			}
		}

		if( this.tabs[i].can_focus ) {
			logDebug( "Switching to tab index " + index );
			this.switch_to( this.tabs[index] );
		}
	}
}
