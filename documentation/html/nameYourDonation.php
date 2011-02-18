<?php

$donation = "0.00";
if( isset( $_REQUEST[ "donation" ] ) ) {
    $donation = $_REQUEST[ "donation" ];
    }

$donationInCents = floor( 100 * $donation );

if( $donationInCents > 1 ) {
    // fix for FastSpring bug where it adds a penny to all custom prices
    $donationInCents -= 1;
    }
else if( $donationInCents < 1 ) {
    // FastSpring tags must be >= 1
    $donationInCents = 1;
    }



header( "Location:https://sites.fastspring.com/jasonrohrer/instant/starfilledskyticket?tags=name_your_donation=$donationInCents")


?>