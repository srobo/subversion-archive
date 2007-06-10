<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="./tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>
<script src="./static/codepress/codepress.js" type="text/javascript"></script>


<script type="text/javascript">
LANGUAGE = "generic"; // The default language for the code editor
cur_path = ""; //TODO: Replace these with cur_tab

open_files = {}; //A dictionary (hash table) of the currently open
//files - one for each tab

//POLLING
POLL_TIME = 2000; //In ms.
poll_data = {"files" : ""}; /*The data to be shipped with the poll.
files : comma seperated list of open files*/
                            
function polled()
{
    /*Polling makes sure we're up to date with others changes.
      This function run roughly every POLL_TIME ms by setTimeout
      call in pollAction
      inputs: None
      returns: Nothing, but callback created*/

    var j = MochiKit.Async.loadJSONDoc("./polldata", poll_data );
    j.addCallback(pollAction);
}

function pollAction(result)
{
    /*Data received from polling call. Process it, then set up a
      timeout to call polled in POLL_TIME ms.
      input: A dictionary of filenames -> revision numbers
                result[filename]["rev"] = N
      returns: Nothing, but sets up timeout*/

    //For each file for which there is info available, if that file
    //is open and if the local working revision is less than that
    //saved on the server, then mark that file as changed
    for (var file in result)
        if(open_files[file])
            if(open_files[file].revision &lt; result[file]["rev"])
                open_files[file].changed = true;

    //Generate the tab list, in case formatting etc needs changing
    //to mark that a file has conflicts
    generatetablist();
    //Setup the next poll in POLL_TIME ms
    setTimeout( "polled()", POLL_TIME );
}

MochiKit.DOM.addLoadEvent( function() {
    //On page load - this replaces a onload action of the body tag
    //Hook up the save file button
    MochiKit.Signal.connect('savefile','onclick', saveFile);
    //Start polling
    //setTimeout( "polled()", POLL_TIME );
    //Grab a file list
    updatefilelist();

    //Create an empty tab
    var newta = MochiKit.DOM.TEXTAREA({"id" : "td"},
        "Type here to create a new file.");

    open_files[""] = {"revision" : "0",
                      "name" : "New",
                      "tabdata" : newta,
                      "dirty" : false,
                      "editedfilename" : "",
                      "changed" : false};
    cur_path = "";
    //Show the New tab
    showtab("", true);
});

function updatefilelist() {
    /*Called to update the file list
        inputs: None
        returns: None, but adds a callback to gotFileList
*/
    var d = MochiKit.Async.loadJSONDoc("./filelist");
    d.addCallback(gotFileList);
}

function gotFileList(nodes){
    /*List of files to display returned. Build up a DOM model of
      the file list for the file tree, then connect signals to the
      directory checkboxes
        inputs: A dictionary with keys as dirnames and values as
        lists of files (or directories) in that dir. Subdirs are in
        turn dictionaries.
        returns: Nothing*/

    MochiKit.DOM.replaceChildNodes("filelist", buildFileList(nodes["children"]));

    //Get all the checkboxes next to directory names
    var checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            "dir_check");
    
    //Go through each of the checkboxes, hooking up a signal.
    //click_dir gets passed an object containing all sorts of info
    //about the click
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes),
            function (a) {
                MochiKit.Signal.connect(a, "onclick", click_dir);
            });
}

function buildFileList(nodes){
    /*Builds up a DOM of a tree of files.
        inputs: A tree of directories and files
        returns: A DOM tree*/
    //Create an unlinked list and fill it with entries
    return MochiKit.DOM.UL({"class" : "links"},
            MochiKit.Base.map(buildFileListEntry, nodes));
}

function Left(str, n){
    /*Get the left n characters of a string
        inputs: str - String of data
                n - characters to return
        returns: string n characters long*/
    if (n &lt;= 0)
        return "";
    else if (n &gt; String(str).length)
        return str;
    else
        return String(str).substring(0,n);
}

function click_dir(data){
    /*Run when a directory checkbox in the file list is clicked on
      Makes selected status of children same as parent
      inputs: data, a lump on info about the event, see MochiKit docs
      Returns: none*/
    var par = data["src"](); //Get the clicked on checkbox
    //Get all checkboxes on the page
    var checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            null);
    
    //For each checkbox on the page, check to see if it is a child of
    //the parent checkbox. If so, set its selected status the same as
    //the parent
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes),
            function (a) {
                //If it's the root checkbox
                //Set all dir_check and file_check class checkboxes
                //to same as root checkbox

                //If it's a different checkbox, only do children
                if (par.name == ""){
                    if(MochiKit.DOM.hasElementClass(a, "file_check") ||
                        MochiKit.DOM.hasElementClass(a, "dir_check"))
                            a.checked = par.checked;
                } else {
                    if(Left(a.name, par.name.length+1) == par.name + "/")
                    a.checked = par.checked;
                }
            });
}

function buildFileListEntry(node){
    /*Create an entry in an unordered list to display a file.
        inputs: a dictionary describing a file or directory
        returns: a DOM object to show that file or directory*/
   
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
        //As for a file, but without the anchor and with a sub list!
        //The sublist is a UL element created by buildFileList
        //mmm... recursion...
        var contents = new Array(MochiKit.DOM.DIV({"class" : "list_row"},
                MochiKit.DOM.SPAN({"class" : "list_box"},
                    MochiKit.DOM.INPUT({"class" : "dir_check",
                                        "type" : "checkbox",
                                        "name" : node.path})),
                MochiKit.DOM.SPAN({"class" : "list_label"}, node.name)), buildFileList(node.children));

    }
    return MochiKit.DOM.LI({"class" : "list_row"}, contents);
}

function get_selected() {
    /*Find out which files are selected.
        inputs: none
        returns: an Array of paths of files*/

    var checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            "file_check");
    var selected = new Array();

    //Iterate through the list of checkboxes, adding the paths of files
    //to the selected array if their checkbox is checked
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes), function (a) {
            if (a.checked){selected.push(a.name)}});
    return selected;
}

function checkout() {
    /*Checkout a list of files.
        TODO: Security munging. Path character escaping.
        inputs: none
        returns: none, but causes a file download by changing
            document.location to a source that provides an attachment*/
    //files is an array of paths of selected files
    var files = get_selected();
    if(files.length &gt; 0){
        document.location = "./checkout?files=" + files.join(",");
    } else {
        alert("No files selected.");
    }
}

//TABS
function savecurrenttab(){
    //Save the data in the current tab to its hidden textarea
    var code = cpscript.getCode();
    alert(code);
    if(code != open_files[cur_path].tabdata.innerHTML){
        open_files[cur_path].dirty = true;
        open_files[cur_path].tabdata.innerHTML = code;
    }
    namefield = MochiKit.DOM.getElement("filename");
    open_files[cur_path].editedfilename = namefield.value;
}

function showtab(tabpath, force) {
    var force = (force == null) ? false : true;
    if((tabpath != cur_path) || force){
        //If the selected tab isn't the current one
        if(!force)
            savecurrenttab();

        //Note details of the now current tab
        cur_path = tabpath;

        //Load in the data for the other tab
        MochiKit.DOM.replaceChildNodes("tabdatacontainer", open_files[tabpath].tabdata);
        cpscript.edit("td"+tabpath, LANGUAGE);
        
        //Set the filename edit correctly
        namefield = MochiKit.DOM.getElement("filename");
        MochiKit.DOM.setNodeAttribute(namefield, "value",
                open_files[tabpath].editedfilename);
        
        
        getLog(tabpath);
        setStatus( "File: " + cur_path + " Revision: " + open_files[tabpath].revision);
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
    //Remove it from the list of open files
    delete open_files[tabpath];
    showtab("", true);
}

function generatetablist() {
    var list = MochiKit.DOM.OL({"id" : "tablist"});
    var filenames = new Array();
    for (var tab in open_files) {
        //TODO: Check cur_path always valid here
        filenames.push(tab);
        var attrs = {"id" : "tab"+tab};
        var classes = new Array();
        if(tab == "")
            classes.push("newtab");
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
                        MochiKit.DOM.A({"href" : "#", "onclick" : "javascript:closetab('" + tab + "');","class" : "tabx"}, "X"))));
    }
    poll_data.files = filenames.join(",");
    MochiKit.DOM.replaceChildNodes("tablistdiv", list);
}

//OPEN AND SAVE FILES
function saveFile(e)
{
    if(open_files[tabpath].revision){
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
            "&amp;rev=" + open_files[tabpath].revision + "&amp;message=" +
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
<title py:content="rev">Robotics IDE</title>
</head>
<body>
    <div id="sidebar">
        <h2>Files</h2>
        <button id="checkout" onclick="checkout()">Checkout Selected</button>
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
    <div id="tabdatacontainer" style="visibility: hidden;"></div>
</body>
</html>
