function Browser(team, rootpath, cback) {
	this.newFilePath = "";
	this.commitMsg = "";
	this.fileTree = null;
	this.callback = cback;
	this._init(team, rootpath);
}

Browser.prototype._init = function(team, rootpath) {
	//make visible
	display_file_browser();
	//get file listings
	this._getFileTree(team, rootpath);
	//set up event handlers 
	disconnectAll($("save-new-file"));
	disconnectAll($("cancel-new-file"));
	connect($("save-new-file"), 'onclick', bind(this.clickSaveFile, this, false));
	connect($("cancel-new-file"), 'onclick', bind(this.clickCancelSave, this));
}

Browser.prototype._receiveTree = function(nodes) {
	this.fileTree = nodes.tree;
	//default to root directory
	this.currFolder = this.fileTree[0].path;
	$("selected-dir").innerHTML = this.currFolder;
	//populate left pane
	$("curr-file-path").innerHTML = this.currFolder+"/";
	replaceChildNodes($("left-pane-list"), null)
	processTree($("left-pane-list"), this.fileTree, "");
}

Browser.prototype._errorReceiveTree = function() {
	status_button("Cannot retreive file listing", LEVEL_ERROR, "retry", bind(this._init, this));
}
Browser.prototype._getFileTree = function(tm, rpath) {
	var d = loadJSONDoc("./filelist", { team : tm,
					    rootpath : rpath});

	d.addCallback( bind(this._receiveTree, this));	
	d.addErrback( bind(this._errorReceiveTree, this));

}

//when user clicks save
Browser.prototype.clickSaveFile = function(override) {
	this.commitMsg = $("new-commit-msg").value;
	logDebug(this.commitMsg);
	this.newFilePath = $("selected-dir").innerHTML + $("new-file-name").getAttribute("value").toString();

	if( ((this.commitMsg == "Commit message") || (this.commitMsg == "")) && !override)
	{
		status_button("No commit message added", LEVEL_WARN, "ignore", bind(this.clickSaveFile, this, true));
	}
	else {
		//execute callback function
		this.callback(this.newFilePath, this.commitMsg);
		//remove events
		disconnectAll($("save-new-file"));
		disconnectAll($("cancel-new-file"));
		//hide browser
		hide_file_browser();
	}	
}

//cancel save operation
Browser.prototype.clickCancelSave = function() {
	this.commitMsg = "";
	this.newFilePath = "";
	fade($("file-browser"), {'from' : 1.0, 'to' : 0.0});
}

//Recursive function to conver filetree into valid DOM tree
function processTree(parentDOM, tree, pathSoFar) {
	for (var i = 0; i < tree.length; i++) {
		if(tree[i].kind == "FOLDER") {
			//create entry in folder list
			var newLeaf = LI(null, "");
			var newPathSoFar = pathSoFar+"/"+tree[i].name;
			newLeaf.setAttribute('path', newPathSoFar);
			newLeaf.setAttribute('onClick', "selectFolder(this.getAttribute('path'), this.getAttribute('fileswithin'))");
			//get just files contained within this directory
			var filesWithin = "";
			for (var j = 0; j < tree[i].children.length; j++) {
					if(tree[i].children[j].kind == "FILE") {
						filesWithin = filesWithin+"/"+tree[i].children[j].name;
					}
			}
			newLeaf.setAttribute('fileswithin', filesWithin);
			newLeaf.innerHTML = tree[i].name+"/";
			//create new list for child folder
			var newBranch = LI(null, "");
			appendChildNodes(newBranch, UL(null, ""));
			appendChildNodes(parentDOM, newLeaf);
			//recursive call, with newly created branch
			appendChildNodes(parentDOM, processTree(newBranch.childNodes[1], tree[i].children, newPathSoFar));
		}
	}
	return parentDOM;
}

//when user selects a folder in right hand pane
function selectFolder(path, files) {
	//update selected directory
	$("selected-dir").innerHTML = "Save Directory: "+path;
	var fileList = files.split("/");
	var li = null;
	//empty file list	
	replaceChildNodes($("right-pane-list"));
	//populate file list
	for(var k = 1; k < fileList.length; k++) {
		li = LI(null, "");
		li.innerHTML = fileList[k];
		appendChildNodes($("right-pane-list"), li);
	}
	//update current new file name
	$("curr-file-path").innerHTML = "New Filename: "+path + "/" + $("new-file-name").getAttribute("value").toString();
}

function display_file_browser() {
	showElement($("file-browser"))
	fade($("file-browser"), {'from' : 0.0, 'to' : 1.0});
}
function hide_file_browser() {
	fade($("file-browser"), {'from' : 1.0, 'to' : 0});
	hideElement($("file-browser"))
}
//when commit message box has focus:
function enlarge_commit_msg() {
	//delete deafult text on focus
	if($("new-commit-msg").innerHTML == "Commit message") { $("new-commit-msg").innerHTML = "";}
	Morph($("right-pane"), {"style": {"height" : "100px" }});
	Morph($("left-pane"), {"style": {"height" : "100px" }});
	Morph($("new-commit-msg"), {"style": {"height" : "150px" }});
}	
//when commit message box looses focus:
function shrink_commit_msg() {
	Morph($("right-pane"), {"style": {"height" : "200px" }});
	Morph($("left-pane"), {"style": {"height" : "200px" }});
	Morph($("new-commit-msg"), {"style": {"height" : "50px" }});
}	

