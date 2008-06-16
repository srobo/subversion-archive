<?php
/* file contains info in an array that mimics the db output */
include 'task_list.inc.php';
include 'Head.inc.php';

?>
	<div id="main">
<div class="graph">
	<div class="container" style="border: solid grey; border-width: 0 1px 1px;">
		<div class="back"></div>
		<div class="back"></div>
		<div class="back"></div>
		<div class="back"></div>
	</div>
	<div class="container">
<?php
//echo mysql_num_rows($result);
//while($row = mysql_fetch_array($result, MYSQL_ASSOC))
$i	= 0;
foreach($task_list as $row)
{ ?>
		<div class="bar" style="left: <?php echo (6*$i+1); ?>0px; height: <?php echo $teams_completed[$row['id']]*2; ?>0px;"><?php echo $teams_completed[$row['id']]; ?></div>
		<div class="bar_t" style="left: <?php echo (6*$i+1); ?>0px;"><?php echo $row['id']; ?></div>
<?php 
	$i++;
}	//end foreach ?>
	</div><!-- end second container -->
</div><!-- end graph div -->
	</div><!-- end main -->
<?php
include 'Foot.inc.php';
?>