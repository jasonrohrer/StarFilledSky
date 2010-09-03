/*
 * Modification History
 *
 * 2008-September-11  Jason Rohrer
 * Created.  Copied from Cultivation.
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#include "minorGems/graphics/Color.h"




#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/random/CustomRandomSource.h"


#include "minorGems/util/log/AppLog.h"


#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"

#include "seaBottom.h"
#include "seaRocks.h"
#include "seaLighting.h"
#include "shipLight.h"
#include "drawUtils.h"
#include "digitalDisplay.h"
#include "hoseReel.h"
#include "depth.h"
#include "lighting.h"
#include "battery.h"
#include "bubbles.h"
#include "airSupply.h"
#include "bubbleEmitters.h"

// globals


// used for picking a "slice" from various noise functions
double globalRandomSeed = 0;





















// position of view in world
double viewCenterX = -100;
double viewCenterY = 0;

// world with of one view
double viewWidth = 10;

// fraction of viewWidth visible vertically (aspect ratio)
double viewHeightFraction;


double velocityX = 0;
double velocityY = 0;
double dragX = 0;
double accelY = 0;
double dragY = 0;

double thrustX = 0;

double thrustValue = 16;

char attached = true;

// barge is heavier, thrust and drag don't affect it as much
double bargeMassFactor = 50;

double shipMassFactor = 10;


CustomRandomSource randSource;



double blackOutProgress = 0.0;
double blackOutRate = 0.004;




void initFrameDrawer( int inWidth, int inHeight ) {
    // start far to the left, and walk right to find where rocks stick
    // up above water line
    
    double rockGridStep = getRockGridStep();
    
    viewCenterX = -50 * rockGridStep;
    
    char openWater = true;
    
    while( openWater ) {
        double newX = viewCenterX + rockGridStep;
        
        doublePair start = { newX - rockGridStep, -rockGridStep };
        doublePair end = { newX + rockGridStep, rockGridStep };
        
        

        SimpleVector<doublePair> centers = getRockCentersInRegion( 
            start, end );
        
        if( centers.size() > 0 ) {
            openWater = false;
            }
        else {
            // keep moving
            viewCenterX += rockGridStep;
            }
        }
    

    setViewCenterPosition( viewCenterX, viewCenterY );
    setViewSize( viewWidth );

    viewHeightFraction = inHeight / (double)inWidth;
    

    initDrawUtils();
    initDigitalDisplay();
    }

void freeFrameDrawer() {
    freeDigitalDisplay();
    freeDrawUtils();
    }


int numRadii = 100;


char lightingOn = true;




void drawFrame() {


    // draw stuff



    /*
    glClearColor( mBackgroundColor->r,
                  mBackgroundColor->g,
                  mBackgroundColor->b,
                  mBackgroundColor->a );
    */
    
    // screen-wide lighting at this depth
    lightColor light = getSeaLighting( getDepth( viewCenterY ) );
    
    lightColor shipLight = getShipLightColor();
    


    double viewRadius = viewWidth / 2;
	

    
    double startScreenX = viewCenterX - viewRadius;
    double endScreenX = viewCenterX + viewRadius;
    double startScreenY = viewCenterY - viewRadius;
    double endScreenY = viewCenterY + viewRadius;
    
    // water background
    setDrawColor( 0.12 * light.r, 0.66 * light.g, 0.93 * light.b, 1.0 );
    
    drawRect( startScreenX, startScreenY, endScreenX, endScreenY );
    
    
    if( endScreenY > 0 ) {
        
        // sky visible
        setDrawColor( 0.75, 0.75, 0.75, 1.0 );
        
        drawRect( startScreenX, 0, endScreenX, endScreenY );
        }
    

    


    // now draw rocks
    
    // extra off sides of screen to prevent pop-in
    double rockGridStep = getRockGridStep();
    doublePair start = 
        { viewCenterX - viewRadius - 2 * rockGridStep, 
          viewCenterY - viewRadius - 2 * rockGridStep };
    
    doublePair end =
        { viewCenterX + viewRadius + 2 * rockGridStep, 
          viewCenterY + viewRadius + 2 * rockGridStep };
    
    doublePair craftPos = { viewCenterX, viewCenterY };
    
    drawRocks( start, end, numRadii, craftPos );


    // yellow hose
    if( getHoseLength() > 0 ) {
        
        setDrawColor( 1 * light.r, 1 * light.g, 0 * light.b, 1.0 );

        double endHoseY = endScreenY;
        
        if( endScreenY > 0 ) {
            // sky visible, stop hose there
            endHoseY = 0;
            }
        
        double startHoseY = viewCenterY;
        if( ! attached ) {
            startHoseY = - getHoseLength();
            }
        

        int numHoseSegments = 5;
        
        

        //void drawQuads( int inNumQuads, double inVertices[], float inVertexColors[] );
        double *hoseVerts = new double[ numHoseSegments * 8 ];
        float *hoseColors = new float[ numHoseSegments * 16 ];
        
        double segLength = ( endHoseY - startHoseY ) / numHoseSegments;
        
        double hoseRadius = 0.02;
        

        for( int s=0; s<numHoseSegments; s++ ) {
            
            double segStart = s * segLength + startHoseY;
            double segEnd = segStart + segLength;
        
            int v = s * 8;
            int c = s * 16;
            
            hoseVerts[ v++ ] = viewCenterX - hoseRadius;
            hoseVerts[ v++ ] = segEnd;
            hoseVerts[ v++ ] = viewCenterX + hoseRadius;
            hoseVerts[ v++ ] = segEnd;

            hoseVerts[ v++ ] = viewCenterX + hoseRadius;
            hoseVerts[ v++ ] = segStart;
            hoseVerts[ v++ ] = viewCenterX - hoseRadius;
            hoseVerts[ v++ ] = segStart;
    

            doublePair hoseVert = { viewCenterX - hoseRadius, segEnd };

            lightColor hoseLight = getNetLighting( hoseVert,
                                                   light,
                                                   shipLight );

            // yellow
            hoseColors[ c++ ] = hoseLight.r;
            hoseColors[ c++ ] = hoseLight.g;
            hoseColors[ c++ ] = 0;
            hoseColors[ c++ ] = 1;


            doublePair hoseVert2 = { viewCenterX + hoseRadius, segEnd };

            hoseLight = getNetLighting( hoseVert2,
                                        light,
                                        shipLight );

            hoseColors[ c++ ] = hoseLight.r;
            hoseColors[ c++ ] = hoseLight.g;
            hoseColors[ c++ ] = 0;
            hoseColors[ c++ ] = 1;


            doublePair hoseVert3 = { viewCenterX + hoseRadius, segStart };

            hoseLight = getNetLighting( hoseVert3,
                                        light, 
                                        shipLight );
            
            hoseColors[ c++ ] = hoseLight.r;
            hoseColors[ c++ ] = hoseLight.g;
            hoseColors[ c++ ] = 0;
            hoseColors[ c++ ] = 1;


            doublePair hoseVert4 = { viewCenterX - hoseRadius, segStart };

            hoseLight = getNetLighting( hoseVert4,
                                        light,
                                        shipLight );


            hoseColors[ c++ ] = hoseLight.r;
            hoseColors[ c++ ] = hoseLight.g;
            hoseColors[ c++ ] = 0;
            hoseColors[ c++ ] = 1;
            }
        


        drawQuads( numHoseSegments, hoseVerts, hoseColors );
        delete [] hoseVerts;
        delete [] hoseColors;
        
        /*
        drawRect( viewCenterX - 0.02, endHoseY, 
                  viewCenterX + 0.02, startHoseY );
        */
        }
    
    drawBubbles( craftPos, viewRadius );

    
    // draw square for ship
    doublePair shipPoint = { viewCenterX - 0.25, viewCenterY - 0.25 };
    
    lightColor lightOnShip = getNetLighting( shipPoint,
                                             light, 
                                             shipLight );
        
    setDrawColor( 0 * lightOnShip.r, 
                  1 * lightOnShip.g, 
                  0 * lightOnShip.b, 1.0 );

    double quadArray[] = { viewCenterX - 0.25, viewCenterY + 0.25,
                           viewCenterX + 0.25, viewCenterY + 0.25,
                           viewCenterX + 0.25, viewCenterY - 0.25,
                           viewCenterX - 0.25, viewCenterY - 0.25 };  
    drawQuads( 1, quadArray );


    
    // draw air fill level
    double airFillLevel = getAirSupplyLevel();
    
    setDrawColor( 0 * lightOnShip.r, 
                  0 * lightOnShip.g, 
                  1 * lightOnShip.b, 0.5 );

    double quadArray2[] = { viewCenterX - 0.2, viewCenterY + 0.2,
                           viewCenterX + 0.2, viewCenterY + 0.2,
                           viewCenterX + 0.2, 
                           viewCenterY + 0.2 - 0.4 * airFillLevel,
                           viewCenterX - 0.2, 
                           viewCenterY + 0.2 - 0.4 * airFillLevel };  
    drawQuads( 1, quadArray2 );
    

    // draw light on ship
    // draw bulb under
    setDrawColor( 0.5 * lightOnShip.r, 
                  0.5 * lightOnShip.g,
                  0.5 * lightOnShip.b,
                  1.0 );
    drawCircle( craftPos, 0.06 );

    if( getShipLightOn() ) {
        
        double bright = getShipLightBrightness();
        
        setDrawColor( shipLight.r, shipLight.g, shipLight.b, bright );
        
        drawCircle( craftPos, 0.06 );

        Color inner( shipLight.r, shipLight.g, shipLight.b, 0.45 * bright );
        Color outer( shipLight.r, shipLight.g, shipLight.b, 0 );
        
        // halo
        drawCircle( craftPos, 0.4, inner, outer );
        }
    

    

    // upper right corner

    int depth = (int)getDepth( viewCenterY );
    
    char *depthString = autoSprintf( "%dm", depth );

    drawString( (unsigned char *)depthString, 
                viewCenterX + viewRadius - 0.2, 
                viewCenterY + viewRadius * viewHeightFraction - 1.25, 
                1, true );
    
    delete [] depthString;
    

    if( ! attached ) {
        // draw battery

        // upper left corner
    
        drawBattery( viewCenterX - viewRadius + 0.2,
                     viewCenterY + viewRadius * viewHeightFraction - 1.25,
                     4 );
        drawAirSupply( viewCenterX - viewRadius + 0.2,
                       viewCenterY + viewRadius * viewHeightFraction - 2.25,
                       4 );


        // black over everything if o2 fraction falls too low
        double o2Fraction = getO2Fraction();
        
        if( o2Fraction < 0.2 ) {
        
            blackOutProgress += blackOutRate;
            if( blackOutProgress > 1 ) {
                blackOutRate = 1;
                }
            }
        else if( blackOutProgress > 0 ) {
            blackOutProgress -= blackOutRate;
            if( blackOutProgress < 0 ) {
                blackOutRate = 0;
                }
            }
        
        if( blackOutProgress > 0 ) {
            setDrawColor( 0, 0, 0, blackOutProgress );
            
            drawSquare( craftPos, viewRadius );
            }
        
        }



    

    






    // for collision detection (move rollback)
    doublePair oldPos = { viewCenterX, viewCenterY };


    // always step reel, so it can roll back up when not attached
    reelStep();


    if( attached ) {
        // attached physics
        
        viewCenterY = - getHoseLength();
        


        double secondDelta = 1.0 / 30;

        viewCenterX += velocityX * secondDelta;


        double trueThrustX = thrustX;
        
        if( thrustX < 0 && velocityX < 0
            ||
            thrustX > 0 && velocityX > 0 ) {
            
            // thrust in same dir as movement
            // becomes less effective as velocity increases
            // (terminal velocity of a propeller regardless of hull drag)
            trueThrustX = thrustX / ( fabs( velocityX ) + 1 );
            }
        

        // barge is heavier
        velocityX += (trueThrustX/bargeMassFactor) * secondDelta;

        if( fabs( (dragX/bargeMassFactor) * secondDelta ) 
            < fabs( velocityX ) ) {
            velocityX += (dragX/bargeMassFactor) * secondDelta;
            }
        else {
            // drag has overcome
            velocityX = 0;
            }


        //apply drag to surface barge
        
        double Cd = 0.82;
        // barge has more area
        double area = 10;
        //double densityWater = 990;
        double densityWater = 1;
    
        dragX = 0.5 * densityWater * velocityX * velocityX * Cd * area;
    
    
        if( velocityX > 0 ) {
            dragX = -dragX;
            }
        }
    else {
        
    
        // do freefall physics
        double secondDelta = 1.0 / 30;
    
        
    

        viewCenterX += velocityX * secondDelta;
        viewCenterY += velocityY * secondDelta;
    


        double batThrustFactor = 1.0;
        
        double batLevel = getBatteryLevel();
        
        if( batLevel < 0.1 ) {
            batThrustFactor = batLevel / 0.1;
            }
        

        double trueThrustX = thrustX * batThrustFactor;
        
        if( trueThrustX < 0 && velocityX < 0
            ||
            trueThrustX > 0 && velocityX > 0 ) {
            
            // thrust in same dir as movement
            // becomes less effective as velocity increases
            // (terminal velocity of a propeller regardless of hull drag)
            trueThrustX = trueThrustX / ( fabs( velocityX ) + 1 );
            }
    

        velocityX += (trueThrustX/shipMassFactor) * secondDelta;

        if( fabs( dragX * secondDelta ) < fabs( velocityX ) ) {
            velocityX += dragX * secondDelta;
            }
        else {
            // drag has overcome
            velocityX = 0;
            }

        velocityY += accelY * secondDelta;

        
        // ignore y drag for now
        if( fabs( dragY * secondDelta ) < fabs( velocityY ) ) {
            velocityY += dragY * secondDelta;
            }
        else {
            // drag has overcome
            velocityY = 0;
            }
        
    
    
        double Cd = 0.82;
        double area = 0.985;
        //double densityWater = 990;
        double densityWater = 1;
    
        dragX = 0.5 * densityWater * velocityX * velocityX * Cd * area;
    
    
        if( velocityX > 0 ) {
            dragX = -dragX;
            }
        //printf( "drag x = %f\n", dragX );


        //printf( "vel, thrust, drag x = %f, %f, %f\n", velocityX, thrustX, dragX );


        //accelX = thrustX + dragX;    
    
        double volume = 0.737 + airFillLevel * 1.263;

        accelY = 9.8 * ( 990 * volume - 1980 ) / ( 1980 + 990 * volume );
    
        dragY = 0.5 * densityWater * velocityY * velocityY * Cd * area;
        }
    


    doublePair newPos = { viewCenterX, viewCenterY };
    
    doublePair newPosXOnly = { viewCenterX, oldPos.y };
    doublePair newPosYOnly = { oldPos.x, viewCenterY };
    
    // collision detection
    char collision = false;
    // get rocks in region around new pos
    SimpleVector<doublePair> nearRocks = 
        getRockCentersInRegion( add( newPos, -2 * rockGridStep ),
                                add( newPos,  2 * rockGridStep ) );
    int numNearRocks = nearRocks.size();
    for( int r=0; r<numNearRocks && !collision; r++ ) {
        
        doublePair p = *( nearRocks.getElement( r ) );

        double shipDist = distance( p, newPos );
        doublePair shipDelta = sub( newPos, p );
        
        double shipAngle = angle( shipDelta );
        
        double radius = getRockRadius( p, getRockRoughness( p ), shipAngle );
        
        if( radius > shipDist ) {
            
            collision = true;
            
            // check if there's still a collision with this rock if
            // we move in x or y only
            if( ! equal( newPos, newPosYOnly ) ) {
                
                if( ! equal( newPos, newPosXOnly ) ) {
                    newPos = newPosXOnly;
                    }
                else {
                    // already checked x only move
                    // now check y
                    newPos = newPosYOnly;
                    }
                
                // back up and recheck this rock
                collision = false;
                r--;
                }
            else {
                // else checked both X- and Y-only moves... true collision
                newPos = oldPos;
                }
            
            if( collision ) {
                /*
                printf( "col at angle %f, (%f,%f)\n", shipAngle,
                        shipDelta.x, shipDelta.y );
                */
                
                setDrawColor( 1, 1, 0, 1 );
                

                drawSquare( p, 0.05 );

                setDrawColor( 1, 0, 0, 1 );
                drawSquare( newPos, 0.05 );
                
                setDrawColor( 0, 0, 1, 1 );
                drawSquare( oldPos, 0.05 );
                

                doublePair rockHit;
                
                rockHit.x = cos( shipAngle ) * radius + p.x;
                rockHit.y = sin( shipAngle ) * radius + p.y;
                
                setDrawColor( 1, 0, 1, 1 );
                drawSquare( rockHit, 0.05 );

                newPos = oldPos;
                }
            }        
        }


    if( attached && newPos.y == oldPos.y ) {
        forceHoseLength( - oldPos.y );
        }
    
    // stick with new pos that was picked during any collision detection
    viewCenterX = newPos.x;
    viewCenterY = newPos.y;


    
    


    setViewCenterPosition( viewCenterX, viewCenterY );


    stepBattery();
    stepBubbles();
    stepAirSupply( craftPos );
    
    stepEmitters( start, end );
    }



void pointerMove( float inX, float inY ) {}
void pointerDown( float inX, float inY ) {}
void pointerDrag( float inX, float inY ) {}
void pointerUp( float inX, float inY ) {}






extern double rockParamA;
extern double rockParamB;
extern int rockParamC;
extern double rockParamD;
extern double rockParamE;
extern double rockParamF;
extern int rockParamG;
extern int rockParamJ;

extern double bottomParamH;
extern int bottomParamI;




void keyDown( unsigned char inASCII ) {
    
    if( inASCII == 'z' || inASCII == 'Z' ) {
        // zoom out
        viewWidth *= 2;
        setViewSize( viewWidth );
        }
    if( inASCII == 'x' || inASCII == 'X' ) {
        // zoom in
        viewWidth /= 2;
        setViewSize( viewWidth );
        }


    char paramAdjust = false;

    if( inASCII == 'a' ) {
        rockParamA -= 0.01;
        if( rockParamA < 0.01 ) {
            rockParamA = 0.01;
            }
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'A' ) {
        rockParamA += 0.01;
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'b' ) {
        rockParamB -= 0.01;
        if( rockParamB < 0.01 ) {
            rockParamB = 0.01;
            }
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'B' ) {
        rockParamB += 0.01;
        clearRockCache();
        paramAdjust = true;
        }

    if( inASCII == 'c' ) {
        rockParamC -= 1;
        if( rockParamC < 1 ) {
            rockParamC = 1;
            }
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'C' ) {
        rockParamC += 1;
        clearRockCache();
        paramAdjust = true;
        }

    if( inASCII == 'd' ) {
        rockParamD -= 0.01;
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'D' ) {
        rockParamD += 0.01;
        clearRockCache();
        paramAdjust = true;
        }



    if( inASCII == 'e' ) {
        rockParamE -= 0.01;
        if( rockParamE < 0.01 ) {
            rockParamE = 0.01;
            }
        clearRadiusCache();
        paramAdjust = true;
        }
    if( inASCII == 'E' ) {
        rockParamE += 0.01;
        clearRadiusCache();
        paramAdjust = true;
        }
    if( inASCII == 'f' ) {
        rockParamF -= 0.15;
        if( rockParamF < 0.01 ) {
            rockParamF = 0.01;
            }
        clearRadiusCache();
        paramAdjust = true;
        }
    if( inASCII == 'F' ) {
        rockParamF += 0.15;
        clearRadiusCache();
        paramAdjust = true;
        }

    if( inASCII == 'g' ) {
        rockParamG -= 1;
        if( rockParamG < 1 ) {
            rockParamG = 1;
            }
        clearRadiusCache();
        paramAdjust = true;
        }
    if( inASCII == 'G' ) {
        rockParamG += 1;
        clearRadiusCache();
        paramAdjust = true;
        }

    if( inASCII == 'j' ) {
        rockParamJ -= 1;
        if( rockParamJ < 1 ) {
            rockParamJ = 1;
            }
        clearRockCache();
        paramAdjust = true;
        }
    if( inASCII == 'J' ) {
        rockParamJ += 1;
        clearRockCache();
        paramAdjust = true;
        }


    if( inASCII == 'h' ) {
        bottomParamH -= 0.01;
        if( bottomParamH < 0 ) {
            bottomParamH = 0;
            }
        clearRockCache();
        clearBottomCache();
        paramAdjust = true;
        }
    if( inASCII == 'H' ) {
        bottomParamH += 0.01;
        
        clearRockCache();
        clearBottomCache();
        paramAdjust = true;
        }


    if( inASCII == 'i' ) {
        bottomParamI -= 1;
        if( bottomParamI < 0 ) {
            bottomParamI = 0;
            }
        clearRockCache();
        clearBottomCache();
        paramAdjust = true;
        }
    if( inASCII == 'I' ) {
        bottomParamI += 1;
        
        clearRockCache();
        clearBottomCache();
        paramAdjust = true;
        }


    if( inASCII == 't' ) {
        numRadii -= 10;
        if( numRadii < 10 ) {
            numRadii = 10;
            }
        clearRadiusCache();
        paramAdjust = true;
        }
    if( inASCII == 'T' ) {
        numRadii += 10;
        clearRadiusCache();
        paramAdjust = true;
        }


    if( inASCII == 's' ) {
        globalRandomSeed -= 0.5;

        clearRockCache();
        clearRadiusCache();
        clearBottomCache();
        paramAdjust = true;
        }
    if( inASCII == 'S' ) {
        globalRandomSeed += 0.5;
        clearRockCache();
        clearRadiusCache();
        clearBottomCache();
        paramAdjust = true;
        }
    
    if( inASCII == '1' ) {
        // jump to test point (424m, 1400 feet, where Beebe still saw light)
        viewCenterY = -424 / 4;
        }
    if( inASCII == '2' ) {
        // jump to test point (424m, 1400 feet, where Beebe still saw light)
        viewCenterY = -848 / 4;
        }
    

    if( inASCII == 'y' || inASCII == 'Y' ) {
        printf( "Depth = %f\n", getDepth( viewCenterY ) );
        }
    if( inASCII == 'p' || inASCII == 'P' ) {
        printf( "Pos = %f, %f, bottom height = %f\n", 
                viewCenterX, viewCenterY, getBottomHeight( viewCenterX ) );
        }
    
    
    if( inASCII == 'l' ) {
        lightingOn = false;
        }
    if( inASCII == 'L' ) {
        lightingOn = true;
        }

    
    if( inASCII == ' ' ) {
        toggleShipLight( ! getShipLightOn() );
        batteryLightOn( getShipLightOn() );
        }
    if( inASCII == 'r' || inASCII == 'R' ) {
        // reel hose all the way back up
        reelOn( -1 );
        attached = false;
        batterySetConnected( false );
        airSupplySetConnected( false );
        }


    if( paramAdjust ) {
        printf( "Rock parameters (seed=%f)(rad=%d)= %f, %f, %d, %d\n",
                globalRandomSeed,
                numRadii, rockParamE, rockParamF, rockParamG, rockParamJ );
        printf( "  Bottom parameter = %f, %d\n", bottomParamH, bottomParamI );
        }
    

    }


void keyUp( unsigned char inASCII ) {}







void specialKeyDown( int inKey ) {


    switch( inKey ) {
        case MG_KEY_UP:
            if( attached ) {
                reelOn( -1 );
                }
            break;
        case MG_KEY_DOWN:
            if( attached ) {
                reelOn( 1 );
                }
            else {
                valveOpen( true );
                }
            break;
        case MG_KEY_RIGHT:
            thrustX = thrustValue;
            batteryMotorOn( true );
            break;
        case MG_KEY_LEFT:
            thrustX = -thrustValue;
            batteryMotorOn( true );
            break;
        }

	}



void specialKeyUp( int inKey ) {
    switch( inKey ) {
        case MG_KEY_UP:
            if( attached ) {
                reelOff();
                }
            break;
        case MG_KEY_DOWN:
            if( attached ) {
                reelOff();
                }
            else {
                valveOpen( false );
                }
            break;
        case MG_KEY_RIGHT:
            if( thrustX == thrustValue ) {
                thrustX = 0;
                batteryMotorOn( false );            
                }
            break;
        case MG_KEY_LEFT:
            if( thrustX == -thrustValue ) {
                thrustX = 0;
                batteryMotorOn( false );
                }
            break;
        }

	} 






    



