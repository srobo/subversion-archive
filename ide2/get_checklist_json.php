<?php
include 'task_list.inc.php';

echo "LIST = {";

//echo mysql_num_rows($result);
//while($row = mysql_fetch_array($result, MYSQL_ASSOC))
foreach($task_list as $row)
{
	echo "\n	".$row['id']." = {";
	foreach($row as $key => $property)
	{
		if(is_string($property))
			$property	= "\"$property\"";
		echo "\n		$key: $property,";
	}
	echo "\n	},";
}
echo "\n}";
?>