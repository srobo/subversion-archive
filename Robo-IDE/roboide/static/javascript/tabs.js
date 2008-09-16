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
function switchToProj() { 
	if( getStyle($("projects-page"), "display") == "none" )
	{
		setStyle($("edit-mode"), {"display" : "none"});
		projpage.show();
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
		//clear previously connected events	
		disconnectAll($("close-edit-area"));
		disconnectAll($("check-syntax"));
		disconnectAll($("save-file"));
		//connect up tab specific events
		connect($("close-edit-area"), 'onclick', this, 'close');
		connect($("check-syntax"), 'onclick', this, 'checkSyntax');
		connect($("save-file"), 'onclick', this, 'saveTab');		
		//change tab color
		setElementClass(this.tabHandle.getElementsByTagName("a")[0], "focus")
		this.properties.focus	=  true;
	}

	this.loseFocus = function() {
		//make tab content invisible (if editable)
		editAreaLoader.hide(this.textbox.id);
		//change tab color
		setElementClass(this.tabHandle.getElementsByTagName("a")[0], "nofocus")
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
			this.properties.fpath = window.prompt("file path:");
		}
		status_msg("file saved:\n"+this.properties.project+"\n"+this.properties.fpath, LEVEL_OK);
	}

	this.close = function() {
		//check file for modification		
		if(1){
			//file has been modified	
			//warn user about close 
			if(window.confirm("File Modified! Do you want to save?")) { 
				this.saveTab();
			}
		}
		//close 
		//remove from global list
		var i = 0;	
		while(i < TABLIST.length) {
			if( TABLIST[i].textbox.id == this.textbox.id ) {
				TABLIST.splice(i, 1);
			}
			i++;
		}
		//garbage collection
		removeElement(this.tabHandle);			
		editAreaLoader.delete_instance(this.textbox.id);
		removeElement(this.textbox);
		//clear all events
		disconnectAll($("close-edit-area"));
		disconnectAll($("check-syntax"));
		disconnectAll($("save-file"));		
		//self destruct 
		delete this;
		//display another tab ... or project menu
		if(TABLIST.length > 0) { TABLIST[0].hideAllButThis(); }
		else { switchToProj(); }
	}
		
	this.open = function(Tbar) {	
		this.properties.focus = false;
		var linkHandle = A("class", "nofocus"); 	
		linkHandle.innerHTML = this.properties.label;
		this.tabHandle = LI(null, "");
		appendChildNodes(this.tabHandle, linkHandle);		
		appendChildNodes(Tbar, this.tabHandle);

		//default action is to gain focus of selected tab
		connect(this.tabHandle, 'onclick', this, 'hideAllButThis');

		//now open a new edit area (if is editable tab)

		//unique id for codebox div
		eaId = (new Date()).getTime();
		this.textbox = TEXTAREA({'id' : eaId});
		// add code
		this.textbox.value = "";	//clear text box
		appendChildNodes($("edit-mode"), this.textbox);
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


