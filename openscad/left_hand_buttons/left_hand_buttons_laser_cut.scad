$fn=200;

buttonPinDiameter = 10;

buttonsNumberX = 4;
buttonsNumberY = 6;
buttonsSpacingX = 11.2;
buttonsSpacingY = 15;
buttonsRankShiftY = 7.5;
// buttonsRankShiftZ = 2.5;
// buttonsBaseHeight = 13; // 10mm + box width

switchSpacingX = 16;
switchSpacingY = 16;
switchLengthX = 13.5;
switchLengthY = 13.5;

globalDeltaX = (switchSpacingX-buttonsSpacingX)*1.5;
globalDeltaY = (switchSpacingY-buttonsSpacingY)*2.5;

pegDiameter = 5.5;
margin = 14;

// Deduced from other dimensions
globalLengthX = (buttonsNumberX-1)*switchSpacingX+switchLengthX+2*margin;
globalLengthY = (buttonsNumberX-1)*buttonsRankShiftY+(buttonsNumberY-1)*switchSpacingY+switchLengthY+2*margin;
buttonPinRadius = buttonPinDiameter/2;
pegRadius = pegDiameter/2;

// 1st layer (switch footprint)
difference() {
    square([globalLengthX,globalLengthY]);
    union() {
        // translate([globalLengthX/2,globalLengthY/2])
        // circle(1);
        translate([margin/2,margin/2])
        circle(pegRadius);
        translate([margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([margin,margin])
        for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
            switchX = i*switchSpacingX+switchLengthX/2;
            switchY = j*switchSpacingY+i*buttonsRankShiftY+switchLengthY/2;
            translate([switchX,switchY,0])
            circle(2);
            translate([switchX-5,switchY+3,0])
            circle(0.5);
            translate([switchX-3,switchY-4,0])
            circle(0.5);
        }
    }
}

translate([100,0])
// 2nd layer (switch squares)
difference() {
    square([globalLengthX,globalLengthY]);
    union() {
        // translate([globalLengthX/2,globalLengthY/2])
        // circle(1);
        translate([margin/2,margin/2])
        circle(pegRadius);
        translate([margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([margin,margin])
        for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
            switchX = i*switchSpacingX;
            switchY = j*switchSpacingY+i*buttonsRankShiftY;
            translate([switchX,switchY,0])
            square([switchLengthX,switchLengthY]);  
        }
    }
}

translate([200,0])
// 3rd layer (button circles)
difference() {
    square([globalLengthX,globalLengthY]);
    union() {
        // translate([globalLengthX/2,globalLengthY/2])
        // circle(1);
        translate([margin/2,margin/2])
        circle(pegRadius);
        translate([margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,margin/2])
        circle(pegRadius);
        translate([globalLengthX-margin/2,globalLengthY-margin/2])
        circle(pegRadius);
        translate([margin+switchLengthX/2,margin+switchLengthY/2])
        for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
            buttonX = i*buttonsSpacingX+globalDeltaX;
            buttonY = j*buttonsSpacingY+i*buttonsRankShiftY+globalDeltaY;
            translate([buttonX,buttonY])
            circle(buttonPinRadius);
        }
    }
}