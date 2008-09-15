//event handler for new file
function openNewTab(){
	var newTabName = 'New'+Math.round((Math.random()*100))+'.py';
	TABLIST.push(new Tab($("tab-list"), {'isPerm' : false, 
										'label' : newTabName, 
										'onclick' : null}))
}


//class defines a single Tab within the Tab Bar
function Tab(Tbar, args){
	this.properties = args;
	this.tabHandle = null;
	this.properties.focus = true;
	this.textbox = null;

	//{'label' : string, 				the text visible in the tab
	// 'project' : project name			the project to which the file in tab belongs
	// 'onclick' : function object, 	the function called when tab is clicked
	// 'isPerm' : bool					if true, tab cannot be removed - i.e. Project tab must always be visible
	// 'focus' : bool}					if true, tab is on top	

	this.gainFocus = function() {
		//make tab content visible (if editable)
		if(!this.properties.isPerm) { 
			setStyle($("frame_"+this.textbox.id), {'display' : 'inline'});
			setStyle($("edit-mode"), {'display' : 'block' });	//make sure we are in edit mode
		}
		//make tab content visible (if editable)
	
		//change tab color
		MochiKit.DOM.setElementClass(this.tabHandle.getElementsByTagName("a")[0], "focus")
		this.properties.focus	=  true;
	}

	this.loseFocus = function() {
		//make tab content invisible (if editable)
		if(!this.properties.isPerm) { 
			setStyle($("frame_"+this.textbox.id), {'display' : 'none'});
		}
		MochiKit.DOM.setElementClass(this.tabHandle.getElementsByTagName("a")[0], "nofocus")
		this.properties.focus	=  false;
	}

	this.hideAllButThis = function() {
		for(var i = 0; i< TABLIST.length; i++) {
			if(!TABLIST[i].properties.isPerm){
				if(TABLIST[i].textbox.id != this.textbox.id) {TABLIST[i].loseFocus();}
				else{ TABLIST[i].gainFocus(); }
			}
			else{ TABLIST[i].loseFocus(); }
		}	
	}

	this.flash = function() {
		Highlight(this.tabHandle.getElementsByTagName("a")[0], 
				{ 'startcolor' : '#ffff31',
				  'endcolor' : this.tabHandle.getElementsByTagName("a")[0].style.backgroundColor,
				'afterFinish' : bind("removeStyle", this)
				});
	}
	// Following line necessary to remove inline style attributes added by Visual effects
	this.removeStyle = function() { (this.tabHandle.getElementsByTagName("a")[0]).removeAttribute("style"); }

		
	this.open = function(Tbar) {	
		var linkHandle = MochiKit.DOM.A({"href" : "#", "class" : "focus"}); 	
		linkHandle.innerHTML = this.properties.label;
		this.tabHandle = MochiKit.DOM.LI(null, "");
		MochiKit.DOM.appendChildNodes(this.tabHandle, linkHandle);		
		MochiKit.DOM.appendChildNodes(Tbar, this.tabHandle);
		
		//if onclick event supplied, add to tab
		if(this.properties.onclick) {
			MochiKit.Signal.connect(this.tabHandle, 'onclick', this.properties.onclick);	
		}
		//else add default (editable tab)
		else {
			//default action is to gain focus of selected tab
			MochiKit.Signal.connect(this.tabHandle, 'onclick', this, 'hideAllButThis');
		}
		//now open a new edit area (if is editable tab)
		if(this.properties.isPerm != true) {
			//hide project page and show editarea
			projpage.hide()
			setStyle($("edit-mode"), {'display' : 'block' });
			//unique id for codebox div
			eaId = (new Date()).getTime();
			this.textbox = MochiKit.DOM.TEXTAREA({'id' : eaId});
			// add code
			this.textbox.value = "import * string";
			MochiKit.DOM.appendChildNodes($("edit-mode"), this.textbox);
			//initialize new instance of editArea			
			editAreaLoader.init({
		 		id : eaId,
		 		syntax : "python",
		 		language : "en",
		 		start_highlight : true,
		 		allow_toggle : false,
		 		allow_resize : "no",
				display : 'onload',
		 		replace_tab_by_spaces : 4,
				min_width:600,
				min_height:400
	 		});
			//get focus on the new tab
			this.hideAllButThis();
			this.flash();
		}
		return this;
	}
	
	this.open(Tbar);	

	this.close = function() {
		MochiKit.DOM.removeElement(this.tabHandle);
	}
	
}

