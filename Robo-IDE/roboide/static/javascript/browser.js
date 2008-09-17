function Browser() {
	this.currFile = "";
	this.currFolder = "";
	this.fileTree = null;
	this.folderList = new Array();
	this.rightDOM = null;
	this.getFileTree();
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

Browser.prototype.getFileTree = function() {
	var d = loadJSONDoc("./filelist", { team : 1,
					    rootpath : ""});

	d.addCallback( bind(this._receiveTree, this));	
	d.addErrback( function() { alert("Error getting the file list"); });

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
			newLeaf.innerHTML = tree[i].name;
			//create new list for child folder
			var newBranch = UL(null, "");
			var newParentDOM = appendChildNodes(parentDOM, newLeaf);
			appendChildNodes(newParentDOM, newBranch);
			newParentDOM = newParentDOM.getElementsByTagName("UL")[0];
			//recursive call, with newly created branch
			processTree(newParentDOM, tree[i].children, newPathSoFar);
		}
	}
	return parentDOM;
}

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

function enlarge_commit_msg() {
	if($("new-commit-msg").innerHTML == "Commit message") { $("new-commit-msg").innerHTML = "";}
	Morph($("right-pane"), {"style": {"height" : "100px" }});
	Morph($("left-pane"), {"style": {"height" : "100px" }});
	Morph($("new-commit-msg"), {"style": {"height" : "150px" }});
}	

function shrink_commit_msg() {
	Morph($("right-pane"), {"style": {"height" : "200px" }});
	Morph($("left-pane"), {"style": {"height" : "200px" }});
	Morph($("new-commit-msg"), {"style": {"height" : "50px" }});
}	

function click_save_file(override) {
	var commitMsg = $("new-commit-msg").innerHTML;
	if( ((commitMsg == "Commit message") || (commitMsg == "")) && !override)
	{
		status_button("No commit message added", LEVEL_WARN, "ignore", function() {click_save_file(true)});
	}
	else {
		status_msg("File '"+$("new-file-name").getAttribute("value").toString()+"' saved successfully", LEVEL_OK); 
	}
}
