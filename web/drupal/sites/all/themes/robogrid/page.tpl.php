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

<div>

<div id="outer">
	<div id="banner">
		<!-- <a href="" id="trigger">Click</a> -->
		<?php if ($logo) { ?><img src="<?php print $logo ?>" alt="<?php if($site_name) print $site_name; else print "Student Robotics";	?>" /><?php } ?>
		<?php /* if ($logo) { ?><a href="<?php print $front_page ?>" title="<?php print t('Home') ?>"><img src="<?php print $logo ?>" alt="<?php if($site_name) print $site_name; else print "Student Robotics";	?>" /></a><?php } ?>
		<?php if ($site_slogan) { ?><div class='site-slogan'><?php print $site_slogan ?></div><?php } */?>
	</div>
	<div id="top">
		<div id="navigate">
			<?php // this list will not exist in the future because the menu will be generated from drupal ?>
			<ul>
			<li><a href="http://www.free-css-templates.com" title="home">IDE HOME</a></li>
			<li><a href="#" title="Articles">FORUMS</a></li>
			<li><a href="#" title="Gallery">CHECK LIST</a></li>
			<li><a href="#" title="Affiliates">DOCUMENTATION</a></li>
			<li><a href="#" title="Articles">SIMULATOR</a></li>
			<li><a href="#" title="Abous us">MESSAGES</a></li>
			<li><a href="#" title="Contact">CONTACT</a></li>
			</ul>
		</div>
	<div id="header">
		<?php print $header ?>
		<form id="login">
			<label for="login_user">SR Username</label> <input type="text" value="Username" name="user" id="login_user" class="login" onFocus="this.value='';" />
			<label for="login_pass">SR Password</label> <input type="password" value="Password" name="pass" id="login_pass" class="login" onFocus="this.value='';" />
			<input type="submit" value="Login" name="submit" id="login_sbumit" />
		</form>
	</div>

	<div id="main">
		<div id="content">
			<div id="sidebar-left">
				<div id="menu">
					<?php if (isset($secondary_links)) { ?><?php print theme('links', $secondary_links, array('class' => 'links', 'id' => 'subnavlist')) ?><?php } ?>
					<?php if (isset($primary_links)) { ?><?php print theme('links', $primary_links, array('class' => 'links', 'id' => 'navlist')) ?><?php } ?>
					<?php print $search_box ?>
				</div>
				<?php if ($left) print $left ?>
			</div>
			<?php if ($mission) { ?><div id="mission"><?php print $mission ?></div><?php } ?>
			<?php print $breadcrumb ?>
			<h1 class="title"><?php print $title ?></h1>
			<div class="tabs"><?php print $tabs ?></div>
			<?php if ($show_messages) { print $messages; } ?>
			<?php print $help ?>
			<?php print $content; ?>
			<?php print $feed_icons; ?>
		</div>
		<div id="sidebar-right">
			<?php if ($right) print $right ?>
		</div>
	</div>
	<div id="footer">
		<div class="right">
			Student Robotics - <a href="/rss/">RSS Feed</a>
			<br />
			Design: <a href="http://www.free-css-templates.com">David Herreman</a>
			<?php print $footer_message ?>
			<?php print $footer ?>
		</div>
	</div>

</div>

<?php print $closure ?>
</body>
</html>
