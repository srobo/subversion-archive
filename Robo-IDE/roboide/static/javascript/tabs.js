//event handler for new file
function openNewTab(){
	var newTabName = 'New'+Math.round((Math.random()*100))+'.py';
	tab_manager.addTab({'isPerm' : true, 
						'label' : newTabName, 
						'onClick' : MochiKit.Base.bind("changeTab", tab_manager, newTabName)});
}

//class defines a single Tab within the Tab Bar
function Tab(Tbar, args){
	this.properties = args;
	this.tabHandle;
	this.linkHandle

	//{'label' : string, 				the text visible in the tab
	// 'onClick' : function object, 	the function called when tab is clicked
	// 'isPerm' : bool					if true, tab cannot be removed - i.e. Project tab must always be visible
	// 'focus' : bool}					if true, tab is on top
	
	this.properties.focus = true;
		
	this.draw = function(Tbar) {	
		if(	this.properties.focus ) {
			this.linkHandle = MochiKit.DOM.A({"href" : "#", "class" : "focus"}); 
		}
		else{
			this.linkHandle = MochiKit.DOM.A({"href" : "#", "class" : "nofocus"});
		}			
		this.linkHandle.innerHTML = this.properties.label;
		var myList = MochiKit.DOM.LI(null, "");
		MochiKit.DOM.appendChildNodes(myList, this.linkHandle);		
		MochiKit.DOM.appendChildNodes(Tbar, myList);
		MochiKit.Signal.connect(myList, 'onclick', this.properties.onClick);	

		return myList;
	}
	
	this.handle = this.draw(Tbar);
	
	this.hasFocus = function(opt) {
		if(opt){		
			MochiKit.DOM.setElementClass(this.linkHandle, "focus")
			this.properties.focus	=  true;
		}
		else{
			MochiKit.DOM.setElementClass(this.linkHandle, "nofocus")
			this.properties.focus	=  false;		
			}

	}

	this.close = function() {
		MochiKit.DOM.removeElement(this.handle);
	}
}

//Tab manager instigates creation  and deletion of tabs
var tab_manager = new function () {
	this.TabList = new Array();

	this.init = function() {
		this.TabList = new Array();
	}
	
	this.addTab = function(args) {
		this.TabList[this.TabList.length] = new Tab(MochiKit.DOM.getElement("tab-list"), args);
		this.changeTab(args.label);
	}


	this.closeTab = function(tname) {
		for (i = 0; i < this.TabList.length; i++) {
			if((this.TabList[i].properties.label == tname) && !(this.TabList[i].properties.isPerm)) {
				this.TabList[i].close();
				this.TabList.splice(i, 1);
				break;
			}
		}
	}
	
	this.changeTab = function(tab){
		if(tab == "Projects") { 
			MochiKit.Style.setStyle('edit-mode', {'display':'none'});
			projpage.show();}
		else{
			MochiKit.Style.setStyle('edit-mode', {'display':'block'});
			projpage.hide();}
		for (i = 0; i < this.TabList.length; i++) {						
			if(this.TabList[i].properties.label == tab) {
				this.TabList[i].hasFocus(true);
			}
			else{
				this.TabList[i].hasFocus(false);
			}
		}
	}

	this.closeAll = function() {
		for (i = 0; i < this.TabList.length; i++) {
			if(!this.TabList[i].properties.isPerm)
				this.TabList[i].close();	
		}	
		this.init();
	}

}();
