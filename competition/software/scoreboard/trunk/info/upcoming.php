<?
function outputMatches($matches)
{
?>
{
"matches" : [ 
<?
$first = true;
foreach( $matches as $match ) {
	if( !$first )
		echo ",";
	else
		$first = false;

?>
	{ "number" : <?=$match["number"]?>,
          "time" : "<?=$match["time"]?>",
  	  "teams" : [ <?=implode(", ", $match["teams"])?> ] }
<?			
}
?>
] 
}
<?
}

$matches = array();

array_push( $matches, array( "number" => 1,
			     "time" => "10:30",
			     "teams" => array(1,2,3,4) ) );

array_push( $matches, array( "number" => 2,
			     "time" => "10:40",
			     "teams" => array(3,4,5,6) ) );

array_push( $matches, array( "number" => 3,
			     "time" => "10:50",
			     "teams" => array(4,5,6,7) ) );

array_push( $matches, array( "number" => 4,
			     "time" => "11:00",
			     "teams" => array(5,6,7,8) ) );

array_push( $matches, array( "number" => 5,
			     "time" => "11:10",
			     "teams" => array(6,7,8,9) ) );

array_push( $matches, array( "number" => 6,
			     "time" => "11:20",
			     "teams" => array(7,8,9,10) ) );

outputMatches($matches);


