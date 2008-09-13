//class defines a single Tab within the Tab Bar
function Tab(Tbar, args){
	this.properties = args;

	//{'label' : string, 		the text visible in the tab
	// 'onClick' : function object, 		the function called when tab is clicked
	// 'isPerm' : bool}		if true, tab cannot be removed - i.e. Project tab must always be visible

	this.draw = function(Tbar) {		
		var myLink = MochiKit.DOM.A({"href" : "#"});
		myLink.innerHTML = this.properties.label;
		var myList = MochiKit.DOM.LI(null, "");
		MochiKit.DOM.appendChildNodes(myList, myLink);		
		MochiKit.DOM.appendChildNodes(Tbar, myList);
		MochiKit.Signal.connect(myList, 'onclick', this.properties.onClick);
		return myList;
	}
	
	this.handle = this.draw(Tbar);

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
	
	this.closeAll = function() {
		for (i = 0; i < this.TabList.length; i++) {
			if(!this.TabList[i].properties.isPerm)
				this.TabList[i].close();	
		}	
		this.init();
	}
}();
