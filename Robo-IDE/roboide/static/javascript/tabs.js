//event handler for new file
function openNewTab(){
	var newTabName = 'New'+Math.round((Math.random()*100))+'.py';
	TABLIST.push(new Tab($("tab-list"), {'isPerm' : false, 
										'label' : newTabName, 
										'onclick' : null,
										'fpath' : "",
										'project' : ""}))
}

function switchToEdit() { 
	if( getStyle($("edit-mode"), "display") == "none" )
	{
		projpage.hide();
		setStyle($("edit-mode"), {"display" : "block"});
	}
}

function hideAllTabs() {
	for(var i = 0; i < TABLIST.length; i++) {
		if(TABLIST[i].properties.focus) {
			TABLIST[i].loseFocus();
		}
	}
}

//class defines a single Tab within the Tab Bar
function Tab(Tbar, args){
	this.properties = args;
	this.tabHandle = null;		
	this.properties.focus = true;	
	this.textbox = null;		

	//{'label' : string, 			the text visible in the tab
	// 'fpath' : string,			full filepath + filename 
	// 'project' : project name,		the project to which the file in tab belongs
	// 'focus' : bool}					if true, tab has focus	

	this.gainFocus = function() {
		//make tab content visible (if editable)
		editAreaLoader.show(this.textbox.id);
		//display filepath
		$("tab-filename").innerHTML = this.properties.project+" :: "+this.properties.fpath;
		//connect up tab specific events
		MochiKit.Signal.connect($("close-edit-area"), 'onclick', this, 'close');
		MochiKit.Signal.connect($("check-syntax"), 'onclick', this, 'checkSyntax');
		MochiKit.Signal.connect($("save-file"), 'onclick', this, 'saveTab');		
		//change tab color
		MochiKit.DOM.setElementClass(this.tabHandle.getElementsByTagName("a")[0], "focus")
		this.properties.focus	=  true;
	}

	this.loseFocus = function() {
		//make tab content invisible (if editable)
		editAreaLoader.hide(this.textbox.id);
		//clear tab-specific events		
		disconnectAllTo($("close-edit-area"));
		disconnectAllTo($("check-syntax"));
		disconnectAllTo($("savefile"));
		//change tab color
		MochiKit.DOM.setElementClass(this.tabHandle.getElementsByTagName("a")[0], "nofocus")
		this.properties.focus	=  false;
	}

 	this.hideAllButThis = function() {
		//if already have focus, just switch to edit mode
		if(this.properties.focus) {
			switchToEdit();
			return;
		}
		//find current tab with focus
		var i = 0;
		while(i < TABLIST.length) {
			if( TABLIST[i].properties.focus ) {
			TABLIST[i].loseFocus();
			}
			i++;
		}
		switchToEdit();
		this.gainFocus();
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

	this.checkSyntax = function() {
		status_msg("Checking syntax...", LEVEL_WARN);
	}

	this.saveTab = function(){
		if(this.properties.project == "" || this.properties.fpath == "") {
			//we have a new file
			//call to save file
			this.properties.project = window.prompt("project name:");
			this.properties.fpath = window.prompt("file path:")
		}
		status_msg("file saved:\n"+this.properties.project+"\n"+this.properties.fpath, LEVEL_OK);
	}

	this.close = function() {
		//confirm close
		if(!window.confirm("Confirm Close Tab"))
			{ return; }
		//check file for modification		
		if(1){
			//file has been modified	
			//warn user about close 
			if(!window.confirm("File Modified! Close without save?")) 
			{ return; }		//user cancels close
			else{
				//do save routine
				this.saveTab();
			}
		}
		//close 
		MochiKit.DOM.removeElement(this.tabHandle);			
		editAreaLoader.delete_instance(this.textbox.id);
		MochiKit.DOM.removeElement(this.textbox);
		//clear all events
		disconnectAll($("close-edit-area"));
		disconnectAll($("check-syntax"));
		disconnectAll($("savefile"));
		//remove obsolete DOM objects
		disconnectAll(this.textbox);

	}
		
	this.open = function(Tbar) {	
		this.properties.focus = false;
		var linkHandle = MochiKit.DOM.A("class", "nofocus"); 	
		linkHandle.innerHTML = this.properties.label;
		this.tabHandle = MochiKit.DOM.LI(null, "");
		MochiKit.DOM.appendChildNodes(this.tabHandle, linkHandle);		
		MochiKit.DOM.appendChildNodes(Tbar, this.tabHandle);

		//default action is to gain focus of selected tab
		MochiKit.Signal.connect(this.tabHandle, 'onclick', this, 'hideAllButThis');

		//now open a new edit area (if is editable tab)

		//unique id for codebox div
		eaId = (new Date()).getTime();
		this.textbox = MochiKit.DOM.TEXTAREA({'id' : eaId});
		// add code
		this.textbox.value = "";	//clear text box
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

		return this;
	}
	
	this.open(Tbar);	
	
}


