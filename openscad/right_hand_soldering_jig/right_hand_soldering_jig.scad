$fn=200;

buttonPinDiameter = 10;

switchNumberX = 3;
switchNumberY = 3;

switchSpacingX = 18.9;
switchSpacingY = 18;
switchLengthX = 13.2;
switchLengthY = 13.2;


// Deduced from other dimensions
switchRankShiftX = switchSpacingX/2;
surroundingLengthX = 2*switchSpacingX-switchLengthX;
surroundingLengthY = 2*switchSpacingY-switchLengthY;

for (i = [0:switchNumberX-1],j = [0:switchNumberY-1]) {
    switchX = i*switchSpacingX+j*switchRankShiftX;
    switchY = j*switchSpacingY;
    translate([switchX,switchY,0])
    difference() {
        square([surroundingLengthX,surroundingLengthY],true);  
        square([switchLengthX,switchLengthY],true);  
    }
}
