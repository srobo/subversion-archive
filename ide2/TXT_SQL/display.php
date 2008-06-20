<?php
include 'MySQL_to_ccsql.php';

//open a database connection
$cc_db = array();
$cc_db = ccsql_open("db.txt");

//print out databse to view its contents (it should have four new rows)
echo '<h3>Db Contents:</h3>';
ccsql_print($cc_db);
?>