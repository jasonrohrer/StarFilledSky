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
an infinite, recursive, tactical shooter for one player by
<a href="http://hcsoftware.sf.net/jason-rohrer">Jason Rohrer</a><br>
<br>
Coming soon for Windows, Mac, and GNU/Linux.
<br>
<br>
<b>Update:</b>  Beta testing is progressing.  The game is now polished and working well.  Just a bit more tweaking and balancing before release.  Here are some screen shots:

<!--
<?php

$mapFiles = glob( "maps/map*.png" );

foreach( $mapFiles as $f ) {
    echo "<img width=\"512\" height=\"512\" src=\"$f\"><br>\n";
    }
?>


<br>

[<a href="enter.zip">enter.zip</a>]
-->
</center>

<?php include( "footer.php" ); ?>
