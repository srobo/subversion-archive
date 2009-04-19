<?
include("pass.php");

$db = @mysql_connect( $mysql_details["host"],
		      $mysql_details["user"],
		      $mysql_details["password"] );

if( !$db ) die("Couldn't connect to mysql");

if( !mysql_select_db("comp", $db) ) die ("Couldn't switch to use comp database");

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
  	  "teams" : [ <?=implode(", ", $match["teams"])?> ],
	  "matchType" : <?=$match["matchType"]?> }
<?			
}
?>
] 
}
<?
}

$matches = array();

$q = "SELECT * FROM matches WHERE time > " . time() . " ORDER BY time ASC LIMIT 6 ;";
$res = mysql_query( $q, $db );
if( !$res ) die( "Couldn't list matches" );

while( $match = mysql_fetch_assoc($res) ) {
  $m = array( "number" => $match["number"],
	      "time" => date( "H:i", $match["time"] ),
	      "teams" => array( $match["red"], $match["green"], $match["blue"], $match["yellow"] ),
		  "matchType" => $match["matchType"] );
  array_push( $matches, $m );  
 }


outputMatches($matches);


