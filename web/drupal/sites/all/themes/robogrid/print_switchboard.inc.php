<?php
function print_switchboard($links)
{
	echo "\n<ul>";
	$i	= 1;
	$count_s_l	= count($links);
	foreach($links as $row)
	{
		$image	= "sites/all/themes/robogrid/images/".$row['title'].".png";
		if(!file_exists($image))
			$image	= "sites/all/themes/robogrid/images/UnknownFile.png";

		echo "\n	<li".($i < $count_s_l ? "" : " class=\"last\"")."><a title=\"".$row['attributes']['title']."\" href=\"".$row['href']."\">"
		."<img src=\"$image\" title=\"".$row['attributes']['title']."\" alt=\"".$row['title']." page image\" /><p>"
		.$row['title']."</p></a></li>";
		$i++;
	}
	echo "\n</ul>";
}
?>