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
margin = 10;

// Deduced from other dimensions
globalLengthX = (buttonsNumberX-1)*switchSpacingX+switchLengthX+2*margin;
globalLengthY = (buttonsNumberX-1)*buttonsRankShiftY+(buttonsNumberY-1)*switchSpacingY+switchLengthY+2*margin;
buttonPinRadius = buttonPinDiameter/2;
pegRadius = pegDiameter/2;

// 2nd layer (switch squares)
difference() {
    square([globalLengthX,globalLengthY]);
    union() {
        translate([margin,margin])
        for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
            switchX = i*switchSpacingX;
            switchY = j*switchSpacingY+i*buttonsRankShiftY;
            translate([switchX,switchY,0])
            square([switchLengthX,switchLengthY]);  
        }
    }
}