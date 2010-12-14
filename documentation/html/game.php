<?php

// no caching
header('Cache-Control: no-store, no-cache, must-revalidate');
header('Cache-Control: post-check=0, pre-check=0', FALSE);
header('Pragma: no-cache');

error_reporting( E_ALL );


include( "header.php" );



?>

<br>
<center>
<font size=6>Inside a Star-filled Sky</font><br>
a recursive game for one player by
<a href="http://hcsoftware.sf.net/jason-rohrer">Jason Rohrer</a><br>
<br>
<br>
Coming soon as a <i>Name Your Donation</i> download for Windows, Mac, and GNU/Linux.


<?php

$mapFiles = glob( "maps/map*.png" );

foreach( $mapFiles as $f ) {
    echo "<img width=\"512\" height=\"512\" src=\"$f\"><br>\n";
    }
?>


<br>

[<a href="enter.zip">enter.zip</a>]

</center>

<?php include( "footer.php" ); ?>
