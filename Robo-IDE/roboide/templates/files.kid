<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="./tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>
<script src="./static/codepress/codepress.js" type="text/javascript"></script>

<script type="text/javascript">
MochiKit.DOM.addLoadEvent(function(){
    connect('savefile','onclick', function (e) {
        if(cur_rev == 0){
            alert("Invalid revision.");
            return;
        }
        document.body.style.cursor = "wait";
        e.preventDefault(); //Stop normal link action
        var d = MochiKit.Async.loadJSONDoc("./savefile?file=" + cur_path +
            "&amp;rev=" + cur_rev + "&amp;message=" +
            MochiKit.DOM.getElement("message").value + 
            "&amp;code=" + escape(cpscript.getCode()));
        d.addCallback(filesaved);
        });
});

function loadFile(file) {
    var d = MochiKit.Async.loadJSONDoc("./filesrc", {file : file});
    d.addCallback(gotFile);
    getLog(file);
}

function gotFile(result) {
    cur_rev = result["revision"];
    cur_path = result["path"];
    MochiKit.DOM.getElement("tmpcode").innerHTML = result["code"];
    cpscript.edit('tmpcode','generic');
}

function getLog(file) {
    var d = MochiKit.Async.loadJSONDoc("./gethistory?file="+file);
    d.addCallback(gotLog);
}

function gotLog(result) {
    //file_log
    historyselect = MochiKit.DOM.createDOM("SELECT", null,
        MochiKit.Base.map(returnSelect, result["history"]))
    MochiKit.DOM.replaceChildNodes("file_log", historyselect);
}

function returnSelect(data) {
    return MochiKit.DOM.createDOM("OPTION", {"value" : data["rev"]}, "Rev: "
    + data["rev"] + ", Author: " + data["author"] + ", Date: " +
    data["date"]);
}

function filesaved(result) {
    document.body.style.cursor = "default";
    switch(result["success"]){
        case "True": {
            alert("Now at revision: " + result["new_revision"]);
            //Need to reload or we have a non-up-to-date revision - guaranteed merge
            //pain
            cpscript.edit(cur_path);
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
            alert("Merge conflict. Please check the merged files then save again.");
            break;
        }
    }
    getLog(cur_path);
}
</script>
<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" py:replace="''"/>
<title>Robotics IDE</title>
</head>
<body>
    <div id="status_block">SVN Crazy Belgium STYLE!!!</div>

    <div id="sidebar">
        <h2>Files</h2>
        <!-- With kid nesting magic from:
        http://permalink.gmane.org/gmane.comp.python.kid.general/825 -->
        <ul class="links" py:def="display_tree(tree_node)">
            <li>
            <a py:if="tree_node.kind == node_kind.file"
            href="javascript:loadFile('${tree_node.path}')">${tree_node.name}</a>
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

    <div id="getting_started">
        <div id="file_log"></div>
        <textarea id="cpscript" class="codepress javascript"
            style="width:500px;height:425px;"></textarea>
        <p>Commit message: <input id="message" value="Default Save Message"/>
        <a id="savefile" href="#">Save File!</a></p>
    </div>

    <textarea id="tmpcode" style="visibility: hidden;"></textarea>
</body>
</html>
