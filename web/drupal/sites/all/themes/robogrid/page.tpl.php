<?php
// $Id: page.tpl.php,v 1.28 2008/01/24 09:42:52 goba Exp $

// $switchboard used as a flag to enable different themeing when visiting the Switchboard page
$switchboard	= ((stripos($header, "login") === FALSE) && !(stripos($title, "switchboard") === FALSE) ? TRUE : FALSE);

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="<?php print $language->language ?>" xml:lang="<?php print $language->language ?>" dir="<?php print $language->dir ?>">

<head>
<title>Student Robotics</title>
<?php print $head ?>
<?php print $styles ?>
<?php print $scripts ?>
<script type="text/javascript">
/* find out what browser they are running, if its too crap then don't do some parts of the js */
function bad_browser()
{
	if (/MSIE (\d+\.\d+);/.test(navigator.userAgent))
	{ //test for MSIE x.x;
		var ieversion=new Number(RegExp.$1) // capture x.x portion and store as a number
		if (ieversion<7)
			return true;
	}
}

function LOGO()
{
	if(bad_browser())
		document.getElementById('banner-logo').src	= document.getElementById('banner-logo').src.replace('.png', '.gif');
}
</script>
</head>
<body onload="LOGO()">

<div id="outer">

	<div id="banner">
		<?php if ($logo) { ?><img id="banner-logo" src="<?php print $logo ?>" alt="<?php if($site_name) print $site_name; else print "Student Robotics"; ?>" /><?php } ?>
	</div><!-- end banner -->

	<div id="top">
		<?php if(isset($header) && $header && !$switchboard) { ?>
		<div id="header">
			<?php print $header; ?>
		</div><!-- end header -->
		<?php } ?>
	</div><!-- end top -->

	<div id="main">
		<div id="content">
		<?php if(!$switchboard) { ?>
			<div id="sidebar-left">
                            <div id="sidebar-left-inner">
			        <?php print $search_box ?>
                                <?php if ($left) {
                                    print $left ?>
				    <div id="sidebar-left-bottom">
				        <div id="sidebar-left-blcorner" class="left"></div>
				        <div id="sidebar-left-brcorner"></div>
				    </div> <!-- sidebar-left-bottom end -->
                                <?php } ?>
			    </div> <!-- sidebar-left-inner end -->
			</div> <!-- sidebar-left end -->
			<?php if ($mission) { ?><div id="mission"><?php print $mission ?></div><?php } ?>
			<?php print $breadcrumb ?>
			<h1 class="title"><?php print $title ?></h1>
			<div class="tabs"><?php print $tabs ?></div>
			<?php if ($show_messages) { print $messages; } ?>
			<?php print $help ?>
		<?php } else { ?>
		<div id="switchboard">
		<?php if (isset($secondary_links))
			{
				include "print_switchboard.inc.php";
				print_switchboard($secondary_links);
			}
			?>
		</div>
		<?php } // end switchboard case ?>
			<?php print $content; ?>
			<?php print $feed_icons; ?>
		</div><!-- end content -->
		<div id="sidebar-right">
			<?php if ($right) print $right ?>
		</div><!-- end sidebar-right -->
	</div><!-- end main -->

	<div id="footer">
		Site contents copyright &copy; <?php if($site_name) print $site_name; else print "Student Robotics"; ?>.
		<br />
		<a href="/rss/" title="Subscribe to the site RSS feed">RSS Feed</a> -
		<a href="http://validator.w3.org/check/referer" title="Valid XHTHML">Valid XHTML</a> -
		<a href="http://jigsaw.w3.org/css-validator/check/referer" title="Valid CSS">Valid CSS</a>
		<?php print $footer_message ?>
		<?php print $footer ?>
	</div><!-- end footer -->

</div><!-- end outer -->

<?php print $closure ?>
</body>
</html>