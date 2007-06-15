<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python from pysvn import node_kind ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<script type="text/javascript">MochiKit = {__export__: false};</script>
<script src="./tg_widgets/turbogears/js/MochiKit.js" type="text/javascript"></script>
<script src="./static/codepress/codepress.js" type="text/javascript"></script>
<script src="./static/javascript/gui.js" type="text/javascript"></script>

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
