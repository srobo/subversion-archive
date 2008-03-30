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

outputMatches($matches);


