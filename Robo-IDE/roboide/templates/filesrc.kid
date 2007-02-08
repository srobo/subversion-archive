<?python import sitetemplate ?>
<html xmlns:py="http://purl.org/kid/ns#">

<head>
        <title>CodePress - Real Time Syntax Highlighting Editor written in JavaScript</title>
        <meta name="description" content="CodePress source code editor window" />
        <link type="text/css" href="/static/codepress/themes/default/codepress.css?timestamp=${curtime}" rel="stylesheet" />
        <link type="text/css" href="/static/codepress/languages/${language}.css?timestamp=${curtime}>" rel="stylesheet" id="cp-lang-style" />
        <script type="text/javascript" src="/static/codepress/engines/${engine}.js?timestamp=${curtime}>"></script>
        <script type="text/javascript" src="/static/codepress/languages/${language}.js?timestamp=${curtime}>"></script>
        <script type="text/javascript">
            parent.cur_rev = "${revision}";
            parent.cur_path = "${path}";
        </script>
</head>
<body py:if="engine == 'gecko'" id='code' py:replace="code"></body>
<body py:if="engine == 'msie'"><pre id='code' py:replace="code"></pre></body>
</html>
