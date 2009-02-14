// File save dialog
// Arguments:
//  - cback: Callback for when things happen.
//  - options: Dictionary of settings:
// 		- "type" 	'isFile' 	- renders file browser with file name box
// 				'isDir' 	- renders file browser with folder name box
// 				'isCommit' 	- renders file browser without file view, just commit box

function Browser(cback, options) {
	// Public functions:
	//  - clickSaveFile(bool):	event handler for when save is clicked, if bool is true, ignore lack of commit msg
	//  - clickCancelSave(): cancel and close browser
	//  - dirSelected(): event handler for when a directory is selected  in the left hand pane
	//  - display(): show all browser css
	//  - hide():hide all browser css

	// Private functions:
	//  - _init:	Constructor.  Displays filebrowser, grabs file tree and connects up events.
	//  - _receiveTree: AJAX success callback
	//  - _errorReceiveTree: AJAX fail callback
	//  - _getFileTree: Grab file tree from server
	//  - _processTree: Recursive function to turn filelist into DOM

	this.newDirectory = "";
	this.newFname = "";
	this.commitMsg = "";

	this._DEFAULT_MSG = "Commit message";
	this._DEFAULT_FNAME = "new.py";

	this.fileList = new Array();
	this.type = options.type;

	this.fileTree = null;
	this.callback = cback;
	this._init();
}

Browser.prototype._init = function() {

	$("new-commit-msg").value = this._DEFAULT_MSG;
	$("new-file-name").value = this._DEFAULT_FNAME;

	//make visible
	this.display();

	//get file listings - if not just commit message
	if(this.type != 'isCommit') {
		this._getFileTree(team, "");
	}

	//clear previous events
	disconnectAll($("save-new-file"));
	disconnectAll($("cancel-new-file"));
	//set up event handlers
    disconnectAll("save-new-file");
    disconnectAll("cancel-new-file");
	connect("save-new-file", 'onclick', bind(this.clickSaveFile, this, false));
	connect("cancel-new-file", 'onclick', bind(this.clickCancelSave, this));
	connect("new-commit-msg","onfocus", bind(this._msg_focus, this));
	connect("new-file-name","onfocus", bind(this._fname_focus, this));

	if(this.type == "isCommit")
		$("new-commit-msg").focus();
}

Browser.prototype._receiveTree = function(nodes) {
	this.fileTree = nodes.tree;
	//default to root directory
	this.newDirectory = this.fileTree[0].path;
	replaceChildNodes($("left-pane-list"), null);
	this._processTree($("left-pane-list"), this.fileTree, "");
}

Browser.prototype._errorReceiveTree = function() {
	$("browser-status").innerHTML = "ERROR :: Unable to retrieve file list";
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
	this.newFname = $("new-file-name").value;

	var fnameErrFlag = (findValue(this.fileList, this.newFname) > -1);
	var commitErrFlag = ( ((this.commitMsg == "Commit message") || (this.commitMsg == "")) && !override);

    if(fnameErrFlag && (this.type=='isFile')) {
		$("browser-status").innerHTML = "\""+this.newFname+"\" already exists!";
		return;
    }

    if(commitErrFlag) {
		$("browser-status").innerHTML = "No commit message added - click to ignore";
		connect($("browser-status"), 'onclick', bind(this.clickSaveFile, this, true));
		return;
    }

    disconnectAll("browser-status");

    switch(this.type) {
        case 'isFile' :
            this.callback(this.newDirectory+"/"+this.newFname, this.commitMsg);
            break;
        case 'isDir' :
            this.callback(this.newDirectory+"/"+this.newFname, this.commitMsg);
            break;
        case 'isCommit' :
            this.callback(this.commitMsg);
            break;
    }
    this.hide();
}

//cancel save operation
Browser.prototype.clickCancelSave = function() {
	status_hide();
	this.commitMsg = "";
	this.newFilePath = "";
	this.hide();
}

//Recursive function to convert filetree into valid DOM tree
Browser.prototype._processTree = function(parentDOM, tree, pathSoFar) {
	for (var i = 0; i < tree.length; i++) {
		if(tree[i].kind == "FOLDER") {
			//create entry in folder list
			var newPathSoFar = pathSoFar+"/"+tree[i].name;

			//get just files contained within this directory
			var filesWithin = "";
			for (var j = 0; j < tree[i].children.length; j++) {
					if(tree[i].children[j].kind == "FILE") {
						filesWithin = filesWithin+"/"+tree[i].children[j].name;
					}
			}

			var newLeaf = LI(null, tree[i].name+"/");

			connect(newLeaf, 'onclick', bind(this.dirSelected, this, newPathSoFar, filesWithin));

			//create new list for child folder
			var newBranch = LI(null, "");
			appendChildNodes(newBranch, UL(null, ""));
			appendChildNodes(parentDOM, newLeaf);

			//recursive call, with newly created branch
			appendChildNodes(parentDOM, this._processTree(newBranch.childNodes[1], tree[i].children, newPathSoFar));
		}
	}
	return parentDOM;
}

Browser.prototype.dirSelected = function(directory, files) {
	logDebug("Folder selected :"+directory);
	//update selected directory
	this.newDirectory = directory;
	$("selected-dir").innerHTML = "Save Directory: "+directory;

	//populate left hand list
	this.fileList = files.split("/");
	var li = null;
	//empty file list
	replaceChildNodes($("right-pane-list"));
	//populate file list
	for(var k = 1; k < this.fileList.length; k++) {
		li = LI(null, "");
		li.innerHTML = this.fileList[k];
		appendChildNodes($("right-pane-list"), li);
	}
}

Browser.prototype.display = function() {
	showElement($("file-browser"));
	showElement($("grey-out"));

	switch(this.type) {
		case 'isFile':
			$("browser-status").innerHTML = "Please Select a save directory & new file name";
			$("selected-dir").innerHTML = "File Save As:";
			showElement("right-pane");
			showElement("left-pane");
			showElement("new-file-name");
			break;
		case 'isDir' :
			$("browser-status").innerHTML = "Please Select a save directory & new directory name";
			$("selected-dir").innerHTML = "New Directory:";
			showElement("right-pane");
			showElement("left-pane");
			showElement("new-file-name");
			break;
		case 'isCommit' :
			$("browser-status").innerHTML = "Please add a commit message before saving";
			$("selected-dir").innerHTML = "Commit Message:";
			hideElement("right-pane");
			hideElement("left-pane");
			hideElement("new-file-name");
			break;
	}
}
Browser.prototype.hide = function() {
	logDebug("Hiding File Browser");
	disconnectAll($("save-new-file"));
	disconnectAll($("cancel-new-file"));
	disconnectAll($("browser-status"));
	hideElement($("file-browser"));
	hideElement($("grey-out"));
	replaceChildNodes($("left-pane-list"));
	replaceChildNodes($("right-pane-list"));
}

Browser.prototype._msg_focus = function() {
	var t = $("new-commit-msg");

	if( t.value == this._DEFAULT_MSG )
		// Select all
		t.select();
}

Browser.prototype._fname_focus = function() {
	var t = $("new-file-name");

	if( t.value == this._DEFAULT_FNAME )
		// Select all
		t.select();
}
