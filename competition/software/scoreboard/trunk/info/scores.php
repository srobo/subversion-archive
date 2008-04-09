<?
include("pass.php");

$db = @mysql_connect( $mysql_details["host"],
		      $mysql_details["user"],
		      $mysql_details["password"] );

if( !$db ) die("Couldn't connect to mysql");

if( !mysql_select_db("comp", $db) ) die ("Couldn't switch to use comp database");

// Get the list of the teams 
$q = "SELECT number FROM teams WHERE 1;";
$result = @mysql_query($q, $db);
if( !$result )
  die ("Couldn't get list of teams");

// Array of scores!
$scores = array();

while( $team = mysql_fetch_assoc($result) ) {
  
  $q = "SELECT sum(score) FROM scores WHERE teamNumber = " . $team["number"] . ";";
  $sres = @mysql_query($q, $db);
  if( !$sres or mysql_num_rows($sres) != 1 )
    die("Error grabbing team info");
  $sinfo = mysql_fetch_assoc( $sres );

  if ($sinfo["sum(score)"] == NULL)
    $sinfo["sum(score)"] = 0;

  $scores[$team["number"]] = $sinfo["sum(score)"];  
 }

// Sort the scores
asort($scores);
$scores = array_reverse($scores, true);

?>
{
"scores" : [
<?

$first = true;
foreach( $scores as $team => $points ) {
	if( !$first )
		echo ",";
	else
		$first = false;
?>
	{ "team" : <?=$team?>, "points" : <?=$points?> }
<? } ?>
]
}
