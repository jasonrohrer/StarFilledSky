<?php

// no caching
header('Cache-Control: no-store, no-cache, must-revalidate');
header('Cache-Control: post-check=0, pre-check=0', FALSE);
header('Pragma: no-cache');

error_reporting( E_ALL );


$pathToRoot = "";

include( "header.php" );



?>


<font size=6>Reviews</font><br>


<table border=0 width="100%" cellpadding=5><tr><td bgcolor="#222222">
<br>
<a href="http://daxgamer.com/2011/02/star-filled-sky-review/">DaxGamer</a> -- "...easily Rohrer's best and most accessible game to date."

<br><br>

<a href="http://www.dealspwn.com/inside-a-star-filled-sky-review/">Dealspwn UK</a> -- "A remarkable achievement. 9 out of 10."

<br><br>

</td></tr></table>
<br>

<?php include( "footer.php" ); ?>
