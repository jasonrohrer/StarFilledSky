<html>

<head>
<title>Inside a Star-filled Sky</title>
</head>

<body bgcolor="#333333" text="#FFFFFF"
      link="#FD8109" alink="#ffff00" vlink="#FD8109">

<center>



<?php

$screenFiles = glob( "screens_1977/screen*.png" );

foreach( $screenFiles as $f ) {
    echo "<img width=\"640\" height=\"480\" src=\"$f\"><br><br><br><br>\n";
    }
?>



<!-- Site Meter -->
<script type="text/javascript" src="http://s47.sitemeter.com/js/counter.js?site=s47insideastarfilledskymain">
</script>
<noscript>
<a href="http://s47.sitemeter.com/stats.asp?site=s47insideastarfilledskymain" target="_top">
<img src="http://s47.sitemeter.com/meter.asp?site=s47insideastarfilledskymain" alt="Site Meter" border="0"/></a>
</noscript>
<!-- Copyright (c)2009 Site Meter -->


<?php include( "footer.php" ); ?>
