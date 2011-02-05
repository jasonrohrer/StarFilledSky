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
</center>
<b>Update</b> (February 4, 2011):  Beta testing is in progress.  The game is now polished and working well.  Just a bit more tweaking and balancing before release.  Here are some fresh screen shots (click to enlarge):

<center>
<br>
<br>


<a href="screen00011.png"><img width="427" height="240" border=0 src="screen00011_thumb.png"></a><br><br><br><br>

<a href="screen00014.png"><img width="427" height="240" border=0 src="screen00014_thumb.png"></a><br><br><br><br>


<?php

$screenFiles = glob( "progressionScreens/thumbs/screen*.png" );

foreach( $screenFiles as $f ) {

    $bigFile = preg_replace( "#/thumbs#", "", $f );
    
    echo "<a href=\"$bigFile\"><img width=\"427\" height=\"240\" border=0 src=\"$f\"></a><br><br><br><br>\n";
    }
?>


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
