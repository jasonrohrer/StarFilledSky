

</td>
</tr>
</table>

</td>
</tr>
</table>
</center>


<br>
<center>

<?php

$nocounter = "0";
if( isset( $_REQUEST[ "nocounter" ] ) ) {
    $nocounter = $_REQUEST[ "nocounter" ];
    }

if( ! $nocounter && ( !isSet( $blockCounter ) || ! $blockCounter ) ) {
?>

<!-- Site Meter -->
<script type="text/javascript" src="http://s47.sitemeter.com/js/counter.js?site=s47insideastarfilledskymain">
</script>
<noscript>
<a href="http://s47.sitemeter.com/stats.asp?site=s47insideastarfilledskymain" target="_top">
<img src="http://s47.sitemeter.com/meter.asp?site=s47insideastarfilledskymain" alt="Site Meter" border="0"/></a>
</noscript>
<!-- Copyright (c)2009 Site Meter -->

<?php
     }
?>

     
</center>


</body>

</html>