{
"scores" : [
<?
$scores = array( 0 => 10, 1 => 3, 40 => 3, 80 => 8 );

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
