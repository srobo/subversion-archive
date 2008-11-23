<?php
if(!function_exists('print_switchboard'))
{
	function print_switchboard($links)
	{
		echo "<div id=\"switchboard\">\n	<ul>";
		$i	= 1;
		$count_s_l	= count($links);
		foreach($links as $row)
		{
			$image	= "/sites/all/themes/robogrid/images/".$row['title'].".png";
			
			//if(!file_exists($image))
			//	$image	= "/sites/all/themes/robogrid/images/UnknownFile.png";

			echo "\n		<li".($i < $count_s_l ? "" : " class=\"last\"")."><a title=\"".$row['attributes']['title']."\" href=\"".$row['href']."\">"
			."<img src=\"$image\" title=\"".$row['attributes']['title']."\" alt=\"".$row['title']." page image\" /><p>"
			.$row['title']."</p></a></li>";
			$i++;
		}
		echo "	\n</ul>\n</div><!-- end switchboard -->\n";
	}
}
if(!isset($secondary_links))
	$secondary_links	= menu_secondary_links();

if( isset($secondary_links) )
{
//	print theme('links', $secondary_links, array('class' => 'links', 'id' => 'subnavlist')); for debug

	print_switchboard($secondary_links);
} else {
	echo "Switchboard error: secondary_links not set: '$secondary_links'";
}
?>
