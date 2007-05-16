<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="./tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>
<script src="./static/codepress/codepress.js" type="text/javascript"></script>


<script type="text/javascript">
POLL_TIME = 2000;
poll_data = {};
remote_modified = false;

MochiKit.DOM.addLoadEvent( function() {
  connect('savefile','onclick', saveFile);
  setTimeout( "polled()", POLL_TIME );

} );

function saveFile(e)
{
        if(cur_rev == 0){
            alert("Invalid revision.");
            return;
        }
        document.body.style.cursor = "wait";
	MochiKit.DOM.getElement("savefile").disabled = true;

        var d = MochiKit.Async.loadJSONDoc("./savefile?file=" + cur_path +
            "&amp;rev=" + cur_rev + "&amp;message=" +
            MochiKit.DOM.getElement("message").value + 
            "&amp;code=" + escape(cpscript.getCode()));
        d.addCallback(filesaved);
}


function polled()
{
    var j = MochiKit.Async.loadJSONDoc("./polldata", poll_data );
    j.addCallback(pollAction);
}

function pollAction(result)
{
    if( result["rev"] &amp;&amp; 
	remote_modified == false &amp;&amp; 
	cur_rev != result["rev"] )
    {
	remote_modified = true
	setStatus( "This file has been edited" )
    }

    setTimeout( "polled()", POLL_TIME );
}

function setStatus(str)
{
    MochiKit.DOM.getElement("status_block").innerHTML = str
}

function loadFile(file, revision) {
    var revision = (revision == null) ? "HEAD" : revision;
    var d = MochiKit.Async.loadJSONDoc("./filesrc", {file : file,
                                                     revision : revision});
    d.addCallback(gotFile);
}

function gotFile(result) {
    cur_rev = result["revision"];
    cur_path = result["path"];
    MochiKit.DOM.getElement("tmpcode").innerHTML = result["code"];
    cpscript.edit('tmpcode','generic');
    getLog(cur_path);
    setStatus( "File: " + cur_path + " Revision: " + cur_rev )

    poll_data.cur_path = cur_path
}

function getLog(file) {
    var d = MochiKit.Async.loadJSONDoc("./gethistory?file="+file);
    d.addCallback(gotLog);
}

function gotLog(result) {
    //file_log
    historyselect = MochiKit.DOM.createDOM("SELECT", {'id':'logselect'},
        MochiKit.Base.map(returnSelect, result["history"]));
    MochiKit.DOM.replaceChildNodes("file_log", historyselect);
}

function loadHistory() {
    box = MochiKit.DOM.getElement("logselect");
    loadFile(cur_path, box.options[box.selectedIndex].value);
}

function returnSelect(data) {
    return MochiKit.DOM.createDOM("OPTION",
            (data["rev"] == cur_rev) ? {"value" : data["rev"],"selected" : "selected"} : {"value" : data["rev"]},
            "Rev: " + data["rev"] + ", Author: " + data["author"] + ", Date: " +
    data["date"]);
}

function filesaved(result) {
    document.body.style.cursor = "default";
    switch(result["success"]){
        case "True": {
            alert("Now at revision: " + result["new_revision"]);
            //Need to reload or we have a non-up-to-date revision - guaranteed merge
            //pain
            loadFile(cur_path);
	    MochiKit.DOM.getElement("savefile").disabled = false;
            break;
        }
        case "Invalid revision": {
            alert("Invalid revision number submitted.");
            break;
        }
        case "Invalid filename": {
            alert("Invalid filename submitted");
            break;
        }
        case "Merge": {
            //Oh dear, need to handle a merge
            cur_rev = result["new_revision"];
            MochiKit.DOM.getElement("tmpcode").innerHTML = result["code"];
            cpscript.edit("tmpcode", "generic");
            getLog(cur_path);
            alert("Merge conflict. Please check the merged files then save again.");
	    MochiKit.DOM.getElement("savefile").disabled = false;
            break;
        }
    }
}

function get_selected() {
    checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            "file_check");
    var selected = new Array();
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes), function (a) {
            if (a.checked){selected.push(a.value)}});
    return selected;
}

file_options = new Array({"id" : "delete",
                 "name" : "Delete Files",
                 "question" : "Are you sure you want to delete:"},
                {"id" : "move",
                 "name" : "Move Files",
                 "question" : "Are you sure you want to move:"});

//Add options to the listbox
function fill_options_select(){
    var file_options_select = MochiKit.DOM.createDOM("SELECT",
            {'id':'file_options_select'},
        MochiKit.Base.map(returnOption, file_options));
    var file_options_span = MochiKit.DOM.SPAN({"id" : "file_options_span"}, file_options_select,
        MochiKit.DOM.BUTTON({"onclick" : "file_cmd()"}, "Go"));

    MochiKit.DOM.replaceChildNodes("file_options", file_options_span);
}

function returnOption(data) {
    return MochiKit.DOM.createDOM("OPTION",
            {"value" : data["id"]}, data["name"])
}

function file_cmd() {
    var files = get_selected();
    if (files.length > 0){
        var box = MochiKit.DOM.getElement("file_options_select");
        var action = file_options[box.selectedIndex];

        if(confirm(action["question"] + "\n" + files.join("\n"))){
            var d = MochiKit.Async.loadJSONDoc("./file_action", {"method" : action["id"], "files" : files});
            d.addCallback(filesDeleted);
        }
    }
}

function filesDeleted(result){
    alert(result["status"]);
}
</script>
<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" py:replace="''"/>
<title>Robotics IDE</title>
</head>
<body>
    <div id="sidebar">
        <h2>Files</h2>
        <div id="file_options"><select></select></div>
        <!-- With kid nesting magic from:
        http://permalink.gmane.org/gmane.comp.python.kid.general/825 -->
        <ul class="links" py:def="display_tree(tree_node)">
            <li class="list_row">
        <div class="list_row" py:if="tree_node.kind == node_kind.file">
<span class="list_box"><input class="file_check" type="checkbox" name="${tree_node.path}"
                    value="${tree_node.path}"></input>
            </span>

            <span class="list_label"><a
                    href="javascript:loadFile('${tree_node.path}')">${tree_node.name}</a></span>
                    </div>
            <span py:if="tree_node.kind != node_kind.file">${tree_node.name}</span>

            <div py:for="node in tree_node.children.values()" py:replace="display_tree(node)" />
            </li>
        </ul>

        <div py:replace="display_tree(tree)">
            <ul>
                <li>SVN file list here</li>
                <ul>
                    <li>With Nesting!</li>
                </ul>
            </ul>
        </div>
    </div>

    <div id="code_block">
        <div id="history"><p>
        <span id="file_log"><select></select></span> <button
            onclick="loadHistory()">Load Revision</button></p>
        </div>
        <div id="status_block">SVN Crazy Belgium STYLE!!!</div>

        <div id="box">
        <textarea id="cpscript" class="codepress javascript"
            style="width:100%;height:425px;"></textarea>
        </div>
        <div id="savebox">
        <p>Commit message: <input id="message" value="Default Save Message"/>
        <button id="savefile">Save File!</button></p></div>
    </div>

    <textarea id="tmpcode" style="visibility: hidden;"></textarea>
</body>
</html>
