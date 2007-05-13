var language = 'generic';
var engine = 'older';
var ua = navigator.userAgent;
var ts = (new Date).getTime(); // timestamp to avoid cache
var lh = location.href;

if(ua.match('MSIE')) engine = 'msie';
else if(ua.match('KHTML')) engine = 'khtml'; 
else if(ua.match('Opera')) engine = 'opera'; 
else if(ua.match('Gecko')) engine = 'gecko';

if(lh.match('language=')) language = lh.replace(/.*language=(.*?)(&.*)?$/,'$1');

document.write('<link type="text/css" href="./static/codepress/codepress.css?ts='+ts+'" rel="stylesheet" />;');
document.write('<link type="text/css" href="./static/codepress/languages/'+language+'.css?ts='+ts+'" rel="stylesheet" id="cp-lang-style" />');
document.write('<script type="text/javascript" src="./static/codepress/engines/'+engine+'.js?ts='+ts+'"></script>');
document.write('<script type="text/javascript" src="./static/codepress/languages/'+language+'.js?ts='+ts+'"></script>');
