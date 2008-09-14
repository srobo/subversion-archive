//event handler for new file
function openNewTab(){
	var newTabName = 'New'+Math.round((Math.random()*100))+'.py';
	TABLIST.push(new Tab($("tab-list"), {'isPerm' : false, 
										'label' : newTabName, 
										'onclick' : null}));
}

//class defines a single Tab within the Tab Bar
function Tab(Tbar, args){
	this.properties = args;
	this.tabHandle = null;
	this.linkHandle = null;
	this.properties.focus = true;

	//{'label' : string, 				the text visible in the tab
	// 'project' : project name			the project to which the file in tab belongs
	// 'onclick' : function object, 	the function called when tab is clicked
	// 'isPerm' : bool					if true, tab cannot be removed - i.e. Project tab must always be visible
	// 'focus' : bool}					if true, tab is on top	
		
	this.open = function(Tbar) {	
		this.linkHandle = MochiKit.DOM.A({"href" : "#", "class" : "focus"}); 	
		this.linkHandle.innerHTML = this.properties.label;
		this.tabHandle = MochiKit.DOM.LI(null, "");
		MochiKit.DOM.appendChildNodes(this.tabHandle, this.linkHandle);		
		MochiKit.DOM.appendChildNodes(Tbar, this.tabHandle);

		if(this.properties.onclick) {
			MochiKit.Signal.connect(this.tabHandle, 'onclick', this.properties.onclick);	
		}
		return this;
	}
	
	this.open(Tbar);	
	
	this.gainFocus = function() {
	
		MochiKit.DOM.setElementClass(this.linkHandle, "focus")
		this.properties.focus	=  true;
	}
	this.loseFocus = function() {
	
		MochiKit.DOM.setElementClass(this.linkHandle, "nofocus")
		this.properties.focus	=  false;
	}

	this.close = function() {
		MochiKit.DOM.removeElement(this.tabHandle);
	}
	
	this.flash = function() {
		var effect = new Highlight(this.linkHandle);
		effect.options.startcolor = '#253571';
		effect.options.endcolor = '#2ee5e1';
		effect.start();
	}
}

