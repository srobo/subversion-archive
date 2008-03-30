{
"scores" : [
<?
$scores = array( 1 => 10, 2 => 34, 3 => 89, 4 => 8, 5=>9, 6=>10,
		 7 => 39, 8 => 98, 9 => 100, 10 => 3  );

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
