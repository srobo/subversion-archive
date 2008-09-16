function Browser() {
	this.currFile = "";
	this.currFolder = "";
	this.fileTree = null;
	this.folderList = new Array();
	this.rightDOM = null;
}

Browser.prototype._receiveTree = function(nodes) {
	this.fileTree = nodes.tree;
}

Browser.prototype.getFileTree = function() {
	var d = loadJSONDoc("./filelist", { team : team,
					    rootpath : project});
	
	var obj = this;
	d.addCallback( bind(this._receiveTree, this));	
	d.addErrback( function() { alert("Error getting the file list") });

}

function processTree(parentDOM, tree) {
	for (var i = 0; i < tree.length; i++) {
		if(tree[i].kind == "FOLDER") {
			var newLeaf = LI(null, "");
			var newBranch = UL(null, "");
			newLeaf.innerHTML = tree[i].name;
			var newParentDOM = appendChildNodes(parentDOM, newLeaf);
			appendChildNodes(newParentDOM, newBranch);
			newParentDOM = newParentDOM.getElementsByTagName("UL")[0]
			processTree(newParentDOM, tree[i].children);
		}
		else {
			var newLeaf = LI(null, "");
			newLeaf.innerHTML = tree[i].name;
			appendChildNodes(parentDOM, newLeaf);
		}
	}
	return parentDOM;
}


	
