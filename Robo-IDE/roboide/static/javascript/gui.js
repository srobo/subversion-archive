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
            if(open_files[file].revision < result[file]["rev"]){
                open_files[file].changed = true;
                var lr = "lr" + file;
                MochiKit.Visual.Highlight(lr, {'startcolor' : '#ffff99'});
            }

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
    //Grab a file list
    updatefilelist();

    //Create an emptyish tab
    open_files[""] = {"revision" : "0",
                      "name" : "New",
                      "tabdata" : "",
                      "dirty" : false,
                      "editedfilename" : "",
                      "changed" : false,
                      "system" : true};

    //Create a tab for the Log
    open_files["Log"] = {"revision" : "0",
                         "name" : "Log",
                         "tabdata" : "",
                         "dirty" : false,
                         "editedfilename" : "",
                         "changed" : false,
                         "system" : true};

    cur_path = "";
    //Initialise the codepress component. This doesn't happen magically as
    //MochiKit overrides the onload handler for this function
    CodePress.run()
    cpscript.edit("", LANGUAGE);
    //Show the blank tab
    showtab("", true);
    var d = MochiKit.Async.loadJSONDoc("./fulllog");
    d.addCallback(show_fullog);
    //Start polling
    setTimeout( "polled()", POLL_TIME );
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

function buildFileListEntry(node){
    /*Create an entry in an unordered list to display a file.
        inputs: a dictionary describing a file or directory
        returns: a DOM object to show that file or directory*/
   
    if (node.kind == "FILE"){
        var contents = MochiKit.DOM.DIV({"class" : "list_row",
                                         "id" : "lr" + node.path},
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

function Left(str, n){
    /*Get the left n characters of a string
        inputs: str - String of data
                n - characters to return
        returns: string n characters long*/
    if (n <= 0)
        return "";
    else if (n > String(str).length)
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

function get_selected() {
    /*Find out which files are selected.
        inputs: none
        returns: an Array of paths of files*/

    var checkboxes = MochiKit.DOM.getElementsByTagAndClassName("input",
            "file_check");
    var selected = new Array()
    //Iterate through the list of checkboxes, adding the paths of files
    //to the selected array if their checkbox is checked
    MochiKit.Iter.forEach(MochiKit.Iter.iter(checkboxes), function (a) {
            if (a.checked){selected.push(a.name)}});
    return selected;
}

function show_fullog(result){
    /*Show a full log for the repository.
        inputs: log : [{author, date, message, rev,
                        changed_paths : [[action, path], ]}, ]
        returns: None*/

    var tab = MochiKit.DOM.TABLE({"id" : "fltable"},
        MochiKit.DOM.THEAD({"id" : "fltablehead"},
            getRow(["Author", "Date", "Message",
                    "Revision", "Files Changed"])),
        MochiKit.DOM.TBODY(null,
            MochiKit.Base.map(buildLogTableEntry, result["log"])));

    MochiKit.DOM.replaceChildNodes("fulllog", tab);
    var newPos = MochiKit.Style.Coordinates(-1000, 0);
    MochiKit.Style.setElementPosition("box", newPos);
    //MochiKit.Style.showElement("fullog");
}

function getRow(row){
    return MochiKit.DOM.TR(null,
        MochiKit.Base.map(MochiKit.Base.partial(MochiKit.DOM.TD, null), row));
}

function buildLogTableEntry(log){
    /*Create an entry with log information
        inputs: a dictionary
        returns: a DOM object*/

    var files = MochiKit.DOM.TABLE(null,
        MochiKit.Base.map(function (fileinfo) {
            return MochiKit.DOM.TR(null,
                MochiKit.DOM.TD(null, fileinfo[1]),
                MochiKit.DOM.TD(null, fileinfo[0]));
        }, log["changed_paths"]));

    return getRow([log["author"], log["date"], log["message"], log["rev"],
                    files]);
}

function deleteclick() {
    /*Delete a list of files. Empty folders pruned.
        inputs: None
        returns: None, but does fire a MochiKit deferred for deleteDone()
    */
    var files = get_selected();
    if(files.length > 0){
        //Check that none of the selected files are currently open in tabs
        for (var af in files)
            for (var of in open_files)
                if (files[af] == of){
                    alert("Can not delete open files.");
                    return;
                }

        if(!confirm("Are you sure you want to delete:\n" + files.join("\n")))
            return;
        var d = MochiKit.Async.loadJSONDoc("./delete",
                                            {files : files.join(",")})
        d.addCallback(deleteDone);
    } else {
        alert("No files selected.");
    }
}

function deleteDone(result){
    /*Data returned from file deletion routine
    inputs (json): Message - Message to show the user.
    returns: None*/
    alert(result["Message"]);
    updatefilelist();
}

function checkout() {
    /*Checkout a list of files.
        TODO: Security munging. Path character escaping.
        inputs: none
        returns: none, but causes a file download by changing
            document.location to a source that provides an attachment*/
    //files is an array of paths of selected files
    var files = get_selected();
    if(files.length > 0){
        document.location = "./checkout?files=" + files.join(",");
    } else {
        alert("No files selected.");
    }
}

//TABS
function savecurrenttab(){
    /*Save the data in the current tab to its hidden textarea.
    inputs: none
    returns: none */
    var code = cpscript.getCode();

    if(cur_path == "Log")
        return;

    //See if the code has changed compared to that in the textarea
    if(code != open_files[cur_path].tabdata){
        //Marking the tab as dirty makes it prompt to save
        open_files[cur_path].dirty = true;
        //Save the new code
        open_files[cur_path].tabdata = code;
    }
    //Remember the filename that may have been changed
    //TODO: Also store the commit message
    namefield = MochiKit.DOM.getElement("filename");
    open_files[cur_path].editedfilename = namefield.value;
}

function showtab(tabpath, force) {
    /*Show a tab of a particular path.
    inputs: tabpath - the path of the tab to show
            force - If true, then do this even if the tab appears to be the one
            that's currently showing, and don't save the contents of the
            current tab. DEFAULTS TO FALSE
    returns: none*/
    
    //Trick to allow default argument values
    var force = (force == null) ? false : true;

    if(tabpath == "Log"){
        MochiKit.Style.showElement("fulllog");
        cur_path = tabpath;
    } else {
        MochiKit.Style.hideElement("fulllog");

        if((tabpath != cur_path) || force){
            //If the selected tab isn't the current one
            if(!force)
                savecurrenttab();

            //Set the new tab to be the current one
            cur_path = tabpath;

            //Load in the data for the new tab
            cpscript.setCode(open_files[tabpath].tabdata);
            
            //Set the filename edit correctly
            namefield = MochiKit.DOM.getElement("filename");
            MochiKit.DOM.setNodeAttribute(namefield, "value",
                    open_files[tabpath].editedfilename);
            //TODO: Also load commit message
            
            //Update the history log
            getLog(tabpath);
            setStatus( "File: " + cur_path + " Revision: " + open_files[tabpath].revision);
        }
    }

    if(open_files[tabpath].system)
        MochiKit.Style.hideElement("history");
    else
        MochiKit.Style.showElement("history");

    generatetablist();
}

function closetab(tabpath) {
    /*Close a tab, prompting to save data if necessary.
    inputs: tabpath - path of the tab to close
    returns: none */

    //Refuse to close the New tab
    //There isn't a button to do this anyway
    if(tabpath == ""){
        return;
    }

    //If closing the current tab, save its data to its textbox first
    if(tabpath == cur_path){
        savecurrenttab();
    }

    //If the file has been changed, prompt the user that they might want to
    //save
    if(open_files[tabpath].dirty){
        if(!confirm("Changes have been made to this file. Still close?")){
            return;
        }
    }

    //OK to close the tab!
    //Remove it from the list of open files
    delete open_files[tabpath];
    //Show the New tab
    showtab("", true);
}

function generatetablist() {
    /*Generate a list of tabs to choose between.
    inputs: none
    returns: none*/

    //Implemented as a list (heavily styled)
    //Create a new list each time
    var list = MochiKit.DOM.OL({"id" : "tablist"});
    
    //Generate a list of which filenames are open to pass to the polling
    //functions, so they get status updates for all the currently open files
    var filenames = new Array();

    for (var tab in open_files) {
        if(tab != "" && tab != "Log")
            filenames.push(tab);
        var attrs = {"id" : "tab"+tab};
        //Each tab might have several classes associated with it. These are
        //then styled appropriately.
        var classes = new Array();
        if(open_files[tab].system == true)
            classes.push("systemtab");
        if(tab == cur_path) 
            classes.push("selected");
        if(open_files[tab].dirty)
            classes.push("dirty");
        if(open_files[tab].changed)
            classes.push("changed");

        attrs["class"] = classes.join(" ");

        //Create a new list item and add it to the list
        //Links for showing a tab and closing it
        MochiKit.DOM.appendChildNodes(list,
                MochiKit.DOM.LI(attrs,
                    MochiKit.DOM.A({"href" : "#", "onclick" : "javascript:showtab('" + tab + "');", "class" : "top"}, open_files[tab].name,
                        MochiKit.DOM.A({"href" : "#", "onclick" : "javascript:closetab('" + tab + "');","class" : "tabx"}, "X"))));
    }
    //Add the list of currently open paths to the blob of data that is sent as
    //part of a poll request
    poll_data.files = filenames.join(",");
    //Show the list on the page
    MochiKit.DOM.replaceChildNodes("tablistdiv", list);
}

//OPEN AND SAVE FILES
function saveFile(e) {
    /*Save the current tab back to the subversion server.
        inputs: e - A load of data from MochiKit. Not used
        returns: none. Creates a deferred which in turn calls filesaved
    */

    /* TODO: Figure out why this was here.
    if(open_files[cur_path].revision){
        alert("Invalid revision.");
        return;
    }*/

    document.body.style.cursor = "wait";

    //TODO: Check cur_path is valid

    //Put data from the current tab into its open_files entry
    savecurrenttab();
    
    //See if the file has been altered.
    if(open_files[cur_path].dirty || 
            (open_files[cur_path].editedfilename != cur_path)){

        //Disable the button
        MochiKit.DOM.getElement("savefile").disabled = true;

        //TODO:Cope with saving as a new file name!
        //Could have:
        //cur_path = MochiKit.DOM.getElement("filename").value;
        //But need to rename open_files data etc

        //TODO: When commit message in open_files, read it from there

        var keys = ["file", "rev", "message", "code"];
        var values = [open_files[cur_path].editedfilename, //File
                      open_files[cur_path].revision, //rev
                      MochiKit.DOM.getElement("message").value, //message
                      cpscript.getCode()]; //Code
        var content = MochiKit.Base.queryString(keys, values);

        //Using doXHR (New in MochiKit 1.4) to do a post request
        var d = MochiKit.Async.doXHR("./savefile",
                           {"method" : "POST",
                            "mimeType" : "application/x-www-form-urlencoded",
                            "sendContent" : content,
                            "headers" : {'Content-Type' : 
                            'application/x-www-form-urlencoded'}});
        d.addCallback(filesaved);
    } else {
        alert("File not changed, not saving.");
    }
}

function filesaved(result) {
    //As called using doXHR, need to extract JSON data manually
    result = MochiKit.Base.evalJSON(result.responseText);
    document.body.style.cursor = "default";
    //Enable the button
    MochiKit.DOM.getElement("savefile").disabled = false;
    var file = result["file"];

    if(result["reloadfiles"] == "true")
        updatefilelist();

    switch(result["success"]){
        case "True": {
            open_files[file].dirty = false;
            open_files[file].revision = result["new_revision"];
            MochiKit.Visual.Highlight("status_block");
            open_files[cur_path].tabdata = result["code"];
            showtab(file, true);
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
            MochiKit.Visual.Highlight("status_block");
            open_files[cur_path].tabdata = result["code"];
            showtab(file, true);
            alert("Merge conflict. Please check the merged files then save again.");
            break;
        }
    }

    generatetablist();
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
        //Add this info to the list of open files
        open_files[result["path"]] = {"revision" : result["revision"],
                                      "name" : result["name"],
                                      "tabdata" : result["code"],
                                      "dirty" : false,
                                      "editedfilename" : result["path"],
                                      "changed" : false,
                                      "system" : false};

        //Load the current script up
        showtab(result["path"]);
    }
}

//FILE HISTORY DISPLAY AND PICKING
function getLog(file) {
    if(file == "")
        return;
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
    MochiKit.DOM.getElement("status_block").innerHTML = str;

}
