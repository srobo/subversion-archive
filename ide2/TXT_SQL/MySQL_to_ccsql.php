<?php

include 'ccsql.php';

//must be called immendiately after ccsql_open
//generate an array that converts col names to col numbers
function set_name_id_array()
{
	global $debug_info, $cc_db, $name_id_array;

	$result	= ccsql_get_rows($cc_db, "0", 0);
	$name_id_array	= array();

	foreach($result[0] as $key => $val)
		$name_id_array[$val]	= $key;

	return;
}

//runs a MySQL select command on a ccsql db
function ORDER_BY_M2C($a, $b)
{
	global $debug_info, $order_col_key, $order_col_arr, $name_id_array;

	$order_col	= $name_id_array[str_ireplace(" desc", "", $order_col_arr[$order_col_key])];

	$retval	= strcmp($a[$order_col], $b[$order_col]);

	$debug_info	.= "\n<pre>\n\$order_col_key = '$order_col_key',	\$a[$order_col] = '".$a[$order_col]."',	\$b[$order_col] = '".$b[$order_col]."'"
				.",	\$order_col_arr[$order_col_key] = '".$order_col_arr[$order_col_key]."',	\$retval = '$retval'\n</pre>";

	if($retval == 0)	//then determine the order via secondary (etc) means, recursively
	{
		$order_col_key	+= 1;
		if($order_col_arr[$order_col_key] != "")
			$retval	= ORDER_BY_M2C($a, $b);
		else
			$retval	= 0;
		$order_col_key	-= 1;
		return $retval;
	}

	if(stripos($order_col_arr[$order_col_key], " desc") === FALSE)
		$asc_desc	= 1;
	else
		$asc_desc	= -1;

	$debug_info	.="\n\$asc_desc = '$asc_desc'\n<br />";

	return $retval * $asc_desc;
}


//runs a MySQL select command on a ccsql db
function SELECT_M2C($command)
{
	global $debug_info, $cc_db, $order_col_key, $order_col_arr, $name_id_array;
	$from_pos	= stripos($command, "from");
	if($from_pos === FALSE)
	{
		echo "\$from_pos === FALSE\n<br />";
		return FALSE;
	}

	$tmp_db	= $cc_db;
	$select	= substr($command, 7, ($from_pos - 8));

	$order_pos	= stripos($command, "order by");
	$where_pos	= stripos($command, "where");

	if($where_pos === FALSE && $order_pos === FALSE)	//if neither where or order by is present
		$from	= substr($command, ($from_pos + 5));
	elseif($where_pos === FALSE)	//if order by is present, but where isn't
		$from	= substr($command, ($from_pos + 5), ($order_pos - ($from_pos + 6)));
	else	//if where is present - where should always be before order by
		$from	= substr($command, ($from_pos + 5), ($where_pos - ($from_pos + 6)));


	if(!($where_pos === FALSE))	//if where is present
	{
		if($order_pos === FALSE)	//if order by is not present
			$where	= substr($command, ($where_pos + 6));
		else
			$where	= substr($command, ($where_pos + 6), ($order_pos - ($where_pos + 7)));
	}

	if(!($order_pos === FALSE))
		$order	= substr($command, ($order_pos + 9));

	$debug_info	.= "\n<br />\$select = '$select'\n<br />\$from = '$from'\n<br />\$where = '$where'\n<br />\$order = '$order'\n<br />\$name_id_array = '".implode(", ", $name_id_array)."'\n<br />";

	if(!($where_pos === FALSE))	//if where is present
		foreach(explode(" && ", str_ireplace(" and ", " && ", $where)) as $test)
		{
			if(!(strpos($test, "\"") === FALSE && strpos($test, "'") === FALSE))	//if we're matching text use the built in tester
			{
				$tmp_arr	= explode("=", str_replace(" = ", "=", $test));
				$tmp_srch	= str_replace(array("'", "\""), "", $tmp_arr[1]);
				if($tmp_srch == "id")
					$tmp_srch	= "0";
				$tmp_col	= $name_id_array[$tmp_arr[0]];
				$tmp_db	= ccsql_get_rows($tmp_db, $tmp_srch, $tmp_col);
			}
			$debug_info	.= "\n<br />\$test = '$test'\n<br />\$tmp_arr[1] = '".$tmp_arr[1]."'\n<br />\$tmp_arr[0] = '".$tmp_arr[0]."'\n<br />\$tmp_srch = '$tmp_srch'\n<br />\$tmp_col = '$tmp_col'\n<br />";
		}

	if(!($order_pos === FALSE))	//if oder by is present
	{
		$order_col_arr	= explode(", ", $order);
		$order_col_key	= 0;
		usort($tmp_db, "ORDER_BY_M2C");
		$debug_info	.= "\n<br />\$test = '$test'".stripos($test, " desc")."\n<br />\$order_col_arr[$order_col_key] = '".$order_col_arr[$order_col_key]."'\n<br />\$order_col_key = '$order_col_key'\n<br />";
	}

	if($select != "*")	//if not all
	{
		$out_db	= array();
		$allowed_keys	= array();
		foreach(explode(", ", $select) as $test)
		{
			array_push($allowed_keys, $name_id_array[$test]);
		}
		foreach($tmp_db as $row)
		{
			$new_row	= array();
			foreach($row as $key => $element)
			{
				if(in_array($key, $allowed_keys))
					array_push($new_row, $element);
			}
			array_push($out_db, $new_row);
		}
	} else
		$out_db	= $tmp_db;

	return $out_db;
}

//executes a MySQL string as ccsql
function MySQL_to_ccsql($command)
{
	global $debug_info;
	if(!is_string($command))
		return FALSE;

	$command_array	= explode(" ", $command);

	$func	= strtoupper($command_array[0])."_M2C";
	$debug_info	.= $func."\n<br />";

	if(function_exists($func))
	{
		$debug_info	.= "function $func exists\n<br />";
		return $func($command);
	}
	else
		return FALSE;
}
?>