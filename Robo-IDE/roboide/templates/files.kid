<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="/tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>

<script type="text/javascript">
MochiKit.DOM.addLoadEvent(function(){
    connect('savefile','onclick', function (e) {
        e.preventDefault(); //Stop normal link action
        //Need to escape all of this!!!
        var d = MochiKit.Async.loadJSONDoc("/savefile?file=" + cur_path +
            "&amp;rev=" + cur_rev + "&amp;message=" +
            MochiKit.DOM.getElement("message").value + 
            "&amp;code=" + CodePress.getCode());
        d.addCallback(filesaved);
        });
});
function filesaved(result) {
    if(result["success"]){
        alert("Now at revision: " + result["new_revision"]);
        //Need to reload or we have a non-up-to-date revision - guaranteed merge
        //pain
        CodePress.edit(cur_path);
    } else {
        //Oh dear, need to handle a merge
        cur_rev = result["new_revision"];
        MochiKit.DOM.getElement("tmpcode").innerHTML = result["code"];
        CodePress.edit(cur_path, "tmpcode");
        alert("Merge conflict. Please check the merged files then save again.");
    }
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
            <a py:if="tree_node.kind == node_kind.file" href="javascript:CodePress.edit('${tree_node.path}')">${tree_node.name}</a>
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
        <code id="codepress" title="moo" class="cp" style="width: 500px"></code>
        <script src="/static/codepress/codepress.js" type="text/javascript" id="cp-script"
        lang="en-us"></script>
        Commit message: <input id="message" value="Default Save Message"/>
        <a id="savefile" href="#">Save File!</a> 
    </div>

    <div id="tmpcode" style="visibility: hidden;"></div>        
</body>
</html>
