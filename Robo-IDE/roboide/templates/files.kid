<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="./tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>
<script src="./static/codepress/codepress.js" type="text/javascript"></script>


<script type="text/javascript">
LANGUAGE = "generic";
remote_modified = false;
cur_path = "";
cur_rev = 1;

open_files = {};

//POLLING
POLL_TIME = 2000;
poll_data = {"files" : ""};

function polled()
{
    var j = MochiKit.Async.loadJSONDoc("./polldata", poll_data );
    j.addCallback(pollAction);
}

function pollAction(result)
{
    /*Not sure what remote_modified is
    if( result["rev"] &amp;&amp; 
	remote_modified == false &amp;&amp; 
	cur_rev != result["rev"] ) {
        remote_modified = true
        setStatus( "This file has been edited" )
        }*/
    for (var file in result)
        if(open_files[file])
            if(open_files[file].revision &lt; result[file]["rev"])
                open_files[file].changed = true;

    generatetablist();
    setTimeout( "polled()", POLL_TIME );
}

//DOCUMENT LOAD EVENTS
MochiKit.DOM.addLoadEvent( function() {
    connect('savefile','onclick', saveFile);
    setTimeout( "polled()", POLL_TIME );
    fill_options_select();
    updatefilelist();
});

//FILE LIST
function updatefilelist() {
    d = MochiKit.Async.loadJSONDoc("./filelist");
    d.addCallback(gotFileList);
}

function gotFileList(nodes){
    MochiKit.DOM.replaceChildNodes("filelist", buildFileList(nodes["children"]));
}

function buildFileList(nodes){
    return MochiKit.DOM.UL({"class" : "links"},
            MochiKit.Base.map(buildFileListEntry, nodes));
}

function buildFileListEntry(node){
    if (node.kind == "FILE"){
        var contents = MochiKit.DOM.DIV({"class" : "list_row"},
                MochiKit.DOM.SPAN({"class" : "list_box"},
                    MochiKit.DOM.INPUT({"class" : "file_check",
                                        "type" : "checkbox",
                                        "name" : node.path})),
                MochiKit.DOM.SPAN({"class" : "list_label"},
                    MochiKit.DOM.A({"href" : "javascript:loadFile('" + node.path
                        + "')"}, node.name)));
    }else{
        var contents = new Array(MochiKit.DOM.SPAN(null, node.name),
                                 buildFileList(node.children));

    }
    return MochiKit.DOM.LI({"class" : "list_row"}, contents);
}

//FILE ACTIONS

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
            {"value" : data["id"]}, data["name"]);
}

function get_selected() {
    checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            "file_check");
    var selected = new Array();
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes), function (a) {
            if (a.checked){selected.push(a.name)}});
    return selected;
}

function file_cmd() {
    var files = get_selected();
    if (files.length > 0){
        var box = MochiKit.DOM.getElement("file_options_select");
        var action = file_options[box.selectedIndex];

        if(confirm(action["question"] + "\n" + files.join("\n"))){
            var d = MochiKit.Async.loadJSONDoc("./file_action", {"method" : action["id"], "files" : files});
            d.addCallback(filesActioned);
        }
    }
}

function filesActioned(result){
    updatefilelist();
    alert(result["status"]);
}

//TABS
function savecurrenttab(){
    //Save the data in the current tab to its hidden textarea
    if(cur_path) if (open_files[cur_path]){
        //Will only fail above in odd cases (including just opened the page)
        var code = cpscript.getCode();
        if(code != open_files[cur_path].tabdata.innerHTML){
            open_files[cur_path].dirty = true;
            open_files[cur_path].tabdata.innerHTML = code;
        }
        namefield = MochiKit.DOM.getElement("filename");
        open_files[cur_path].editedfilename = namefield.value;
    }
}

function showtab(tabpath, force) {
    var force = (force == null) ? false : true;
    if((tabpath != cur_path) || force){
        //If the selected tab isn't the current one
        if(!force)
            savecurrenttab();
        //Load in the data for the other tab
        cpscript.edit("td"+tabpath, LANGUAGE);
        //Note details of the now current tab
        cur_path = tabpath;
        cur_rev = open_files[tabpath].revision;
        //Set the filename edit correctly
        namefield = MochiKit.DOM.getElement("filename");
        MochiKit.DOM.setNodeAttribute(namefield, "value",
                open_files[tabpath].editedfilename);
        
        
        getLog(tabpath);
        setStatus( "File: " + cur_path + " Revision: " + cur_rev );
        generatetablist();
    }
}

function closetab(tabpath) {
    if(tabpath == cur_path){
        savecurrenttab();
    }
    if(open_files[tabpath].dirty){
        if(!confirm("Changes have been made to this file. Still close?")){
            return;
        }
    }

    //OK to close the tab!
    //Remove the hidden data storage textarea
    MochiKit.DOM.removeElement("td" + tabpath);
    //Remove it from the list of open files
    delete open_files[tabpath];
    //Select a new tab
    for (var tab in open_files){
        showtab(tab);
        return;
    }
    //Nothing to show. Help!
    alert("Need code here when nothing to show!");
}

function generatetablist() {
    var list = MochiKit.DOM.OL({"id" : "tablist"});
    var filenames = new Array();
    for (var tab in open_files) {
        //TODO: Check cur_path always valid here
        filenames.push(tab);
        var attrs = {"id" : "tab"+tab};
        var classes = new Array();
        if(tab == cur_path) 
            classes.push("selected");
        if(open_files[tab].dirty)
            classes.push("dirty");
        if(open_files[tab].changed)
            classes.push("changed");

        attrs["class"] = classes.join(" ");

        MochiKit.DOM.appendChildNodes(list,
                MochiKit.DOM.LI(attrs,
                    MochiKit.DOM.A({"href" : "#", "onclick" : "javascript:showtab('" + tab + "');", "class" : "top"}, open_files[tab].name,
                        MochiKit.DOM.A({"href" : "#", "onclick" : "javascript:closetab('" + tab + "');"}, "X"))));
    }
    poll_data.files = filenames.join(",");
    MochiKit.DOM.replaceChildNodes("tablistdiv", list);
}

//OPEN AND SAVE FILES
function saveFile(e)
{
    if(cur_rev == 0){
        alert("Invalid revision.");
        return;
    }
    document.body.style.cursor = "wait";
    //TODO: Check cur_path is valid
    //TODO: Modify this for multi tab perhaps?
    savecurrenttab();
    if(open_files[cur_path].dirty || 
            (open_files[cur_path].editedfilename != cur_path)){
        MochiKit.DOM.getElement("savefile").disabled = true;

        //TODO:Cope with saving as a new file name!
        //Could have:
        //cur_path = MochiKit.DOM.getElement("filename").value;
        //But need to rename open_files data etc

        var d = MochiKit.Async.loadJSONDoc("./savefile?file=" + 
            open_files[cur_path].editedfilename +
            "&amp;rev=" + cur_rev + "&amp;message=" +
            MochiKit.DOM.getElement("message").value + 
            "&amp;code=" + escape(cpscript.getCode()));
        d.addCallback(filesaved);
    }
}

function filesaved(result) {
    document.body.style.cursor = "default";
    var file = result["file"];

    if(result["reloadfiles"] == "true")
        updatefilelist();

    switch(result["success"]){
        case "True": {
            alert("Now at revision: " + result["new_revision"]);
            open_files[file].dirty = false;
            open_files[file].revision = result["new_revision"];
            open_files[cur_path].tabdata.innerHTML = 
                result["code"];
            showtab(file, true);
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
            open_files[file].revision = result["new_revision"];
            open_files[cur_path].tabdata.innerHTML = result["code"];
            showtab(file, true);
            alert("Merge conflict. Please check the merged files then save again.");
            MochiKit.DOM.getElement("savefile").disabled = false;
            break;
        }
    }
}

function loadFile(file, revision) {
    if(open_files[file]){
        //File already loaded!
        showtab(file);
        return;
    }
    var revision = (revision == null) ? "HEAD" : revision;
    var d = MochiKit.Async.loadJSONDoc("./filesrc", {file : file,
                                                     revision : revision});
    d.addCallback(gotFile);
}

function gotFile(result) {
    //Check to see if the file already open
    //If so, switch to it
    if(open_files[result["path"]]){
        //Close the current tab
        closetab(result["path"]);
    } else {
        //If not, create a textarea to store the code in
        var newta = MochiKit.DOM.TEXTAREA({"id" : "td"+result["path"]},
                result["code"]);
        //Save it in a hidden block
        MochiKit.DOM.appendChildNodes("tabdata", newta);

        //Add this info to the list of open files
        open_files[result["path"]] = {"revision" : result["revision"],
                                      "name" : result["name"],
                                      "tabdata" : newta,
                                      "dirty" : false,
                                      "editedfilename" : result["path"],
                                      "changed" : false};

        //Load the current script up
        showtab(result["path"]);
    }
}

//FILE HISTORY DISPLAY AND PICKING
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

//MISC

function setStatus(str)
{
    MochiKit.DOM.getElement("status_block").innerHTML = str
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
        <div id="filelist"></div>
    </div>
    <div id="code_block">
        <div id="tablistdiv"></div>
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
         File name: <input id="filename" value=""/>
        <button id="savefile">Save File!</button></p></div>
    </div>

    <textarea id="tmpcode" style="visibility: hidden;"></textarea>
    <div id="tabdata" style="visibility: hidden;"></div>
</body>
</html>
