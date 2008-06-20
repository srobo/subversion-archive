<?php
//STRIPOS - Not added until	PHP5

if(!function_exists("stripos"))
{
	function stripos($str, $needle)
	{
		return strpos(strtolower($str), strtolower($needle));
	}
}

//Always call ccsql_open before attempting to use any other functions,
//ccsql_open returns a pseudo-handle which is actually an array containing a copy of the whole database
//$filename must be a full or relative filename(including extension) to a txt based file
function ccsql_open($filename)
{
	$contents = file_get_contents($filename);
	$db = array();

	if($contents != FALSE)
	{
		$lines = explode('[#]', $contents);
		foreach($lines as $row)
		{
			array_push($db, explode('|',$row));
		}
		return $db;
	} else
		return FALSE;
}

//ccsql_get_rows returns a 2d array containing all database entries whose element in column number '$column'
//is equal to '$string'
//string must be an integer >0(the first column is number 0)
function ccsql_get_rows($db, $string, $column)
{
	$result = array();
	if(isset($db[0][0]))
	{
		foreach($db as $row)
		{
			if(strcmp($row[$column], $string) == 0)
				array_push($result, $row);
		}
		return $result;
	}
}

//ccsql_add_row will add the extra rows of the database in the array $new_line to the end of the databse
//and then resort the index(first column unique element) such that they are in numerical order and all unique
//returns the new database in a 2d array
//$new_line must be an array with consistent length,(first element in each row may be blank to be auto-filled)
function ccsql_add_row($db, $new_line)
{
	if(!isset($db[0][0]))
	{
		$db = array();
		array_push($db, $new_line);
	} else {
		$id = 0;
		for($id=0; $id < count($db); $id++)
		{
			$db[$id][0] = $id;
		}

		$row_id = count($db);
		$new_line[0] = $row_id;
		array_push($db, $new_line);

		$id = 0;
		for($id=0; $id < count($db); $id++)
		{
			$db[$id][0] = $id;
		}
	}
	return $db;
}

//ccsql_update_row finds the row with id = $row_id and replaces the whole row of data with that held in $new_line
function ccsql_update_row($db, $new_line, $row_id){

	$col_count = 0;

	for($col_count = 0; $col_count <= count($db[$row_id]); $col_count++)
	{
		if($new_line[$col_count] != "")
			$db[$row_id][$col_count] = $new_line[$col_count];
	}
	return $db;

}

//ccsql_remove_rows copies the entire database into a new 2d array ommiting rows with id's in the $id_list array
//the new database in array form is returned
function ccsql_remove_rows($db, $id_list)
{
	$new_db = array();
	foreach($db as $row)
	{
		if(!in_array($row[0], $id_list))	//do not add to new array!
			array_push($new_db, $row);
	}
	$id = 0;
	foreach($new_db as $row)
	{
		$row[0] = $id;
		$id++;
	}
	return $new_db;
}

//ccsql_print does not alter the array but instead outputs the HTML necessary to represent the
//databse in tabular form. alter the css definition for .admin_table as required
function ccsql_print($db)
{
	echo "<br /><table class='admin_table'>";
	if(!isset($db[0][0]))
		echo "<tr><td colspan=7>Database Empty</td></tr>";
	else
	{
		foreach($db as $row)
		{
			echo "<tr>";
			foreach($row as $element)
			{
				echo "<td>".substr($element,0,30)."</td>";
			}
			echo "</tr>";
		}
	}
	echo "</table>";
}

//ccsql_save converts the databse from its easy to handle 2d array format into the necessary strings of
//text so that it can be saved in a txt file. all the necessary column and row separators are added
function ccsql_save($db, $filename)
{
	//reset index column
	$id = 0;
	for($id=0; $id < count($db); $id++)
	{
		$db[$id][0] = $id;
	}

	//create contents of db
	$contents = "";
	$count = 0;

	if($db[0][0] == "0")
	{
		for($count = 0; $count < count($db)-1; $count ++)
		{
			$subcount = 1;
			$contents .= $db[$count][0];
			for($subcount = 1; $subcount < count($db[$count]); $subcount++)
			{
				$contents .= "|".$db[$count][$subcount];
			}
		$contents .= '[#]';
		}

		//print last line to file(without [#])

		$subcount = 1;
		$contents .= $db[$count][0];
		for($subcount = 1; $subcount < count($db[$count]); $subcount++)
		{
			$contents .= "|".$db[$count][$subcount];
		}
	}

	//open, write and close file
	$handle = fopen($filename, 'w');
	if($handle != FALSE)
	{
		fwrite($handle, $contents);
		fclose($handle);
		return TRUE;
	} else
		return FALSE;
}

//accepts any portion of the database in 2d array form and returns a smaller portion which is no greater than $length long
function ccsql_shorten($db, $length)
{
	$new_db = array();
	if($length > count($db))
		$length = count($db);

	for($count = 0; $count < $length; $count++)
	{
		array_push($new_db, $db[$count]);
	}
	return $new_db;
}

//copies any portion of the database starting with the latest entries
function ccsql_reverse($db)
{
	$new_db = array();
	$count = count($db)-1;
	for($count; $count >= 0; $count--)
	{
		array_push($new_db, $db[$count]);
	}
	return $new_db;
}

?>