<?php
// $Id: page.tpl.php,v 1.28 2008/01/24 09:42:52 goba Exp $
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="<?php print $language->language ?>" xml:lang="<?php print $language->language ?>" dir="<?php print $language->dir ?>">

<head>
<title>Student Robotics</title>
<?php print $head ?>
<?php print $styles ?>
<?php print $scripts ?>
</head>
<body>

<div id="outer">

	<div id="banner">
		<!-- <a href="" id="trigger">Click</a> -->
		<?php if ($logo) { ?><img src="<?php print $logo ?>" alt="<?php if($site_name) print $site_name; else print "Student Robotics"; ?>" /><?php } ?>
		<?php /* if ($logo) { ?><a href="<?php print $front_page ?>" title="<?php print t('Home') ?>"><img src="<?php print $logo ?>" alt="<?php if($site_name) print $site_name; else print "Student Robotics";	?>" /></a><?php } ?>
		<?php if ($site_slogan) { ?><div class='site-slogan'><?php print $site_slogan ?></div><?php } */?>
	</div><!-- end banner -->

	<div id="top">
		<?php if(isset($header) && $header) { ?>
		<div id="header">
			<?php print $header; ?>
		</div><!-- end header -->
		<?php } else { ?>
        <div id="navigation-menu">
            <?php if (isset($secondary_links)) { ?><?php print theme('links', $secondary_links, array('class' => 'links', 'id' => 'subnavlist')) ?><?php } ?>
        </div><!-- end navigation-menu -->
		<?php } ?>
	</div><!-- end top -->

	<div id="main">
		<div id="content">
			<div id="sidebar-left">
			    <?php print $search_box ?>
			    <?php if ($left) print $left ?>
			</div><!-- end sidebar-left -->
			<?php if ($mission) { ?><div id="mission"><?php print $mission ?></div><?php } ?>
			<?php print $breadcrumb ?>
			<h1 class="title"><?php print $title ?></h1>
			<div class="tabs"><?php print $tabs ?></div>
			<?php if ($show_messages) { print $messages; } ?>
			<?php print $help ?>
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