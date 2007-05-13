<?python import sitetemplate ?>
<html xmlns:py="http://purl.org/kid/ns#">

<head>
        <title>CodePress - Real Time Syntax Highlighting Editor written in JavaScript</title>
        <meta name="description" content="CodePress source code editor window" />
        <script type="text/javascript"
        src="/static/codepress/detect.js"></script>
        <script type="text/javascript">
            parent.cur_rev = "${revision}";
            parent.cur_path = "${path}";
        </script>
</head>
<body py:if="browser == 'Gecko'" id='code' py:content="code"></body>
<body py:if="browser == 'MSIE'"><pre id='code' py:content="code"></pre></body>
</html>
