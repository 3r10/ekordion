$fn=200;

buttonPinDiameter = 9.5;

buttonsNumberX = 4;
buttonsNumberY = 6;
buttonsSpacingX = 11.2;
buttonsSpacingY = 15;
buttonsRankShiftY = 7.5;
buttonsRankShiftZ = 2.5;

buttonsBaseHeight = 13; // 10mm + box width

linkHeight = 4;
plugRadius = 2.5;

switchSpacingX = 16;
switchSpacingY = 16;
switchLengthX = 5;
switchLengthY = 6.5;
switchCrossLength = 4;
switchCrossWidth = 1.3;
switchCrossDepth = 3.6;
switchHeight = 5;





globalDeltaX = (switchSpacingX-buttonsSpacingX)*1.5;
globalDeltaY = (switchSpacingY-buttonsSpacingY)*2.5;

// Deduced from other dimensions
buttonPinRadius = buttonPinDiameter/2;
linkWidth = sqrt(switchLengthX^2+switchLengthY^2);
linkRadius = linkWidth/2;

for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
// for (i = [buttonsNumberX-1],j = [0]) {
    // Buttons
    buttonX = i*buttonsSpacingX+globalDeltaX;
    buttonY = j*buttonsSpacingY+i*buttonsRankShiftY+globalDeltaY;
    buttonZ = switchHeight;
    translate([buttonX,buttonY,buttonZ])
    union() {
        cylinder(plugRadius,0,plugRadius);
        translate([0,0,plugRadius])
        cylinder(linkHeight-plugRadius,plugRadius,plugRadius);
        h = buttonsBaseHeight+i*buttonsRankShiftZ;
        translate([0,0,linkHeight])
        cylinder(h-1,buttonPinRadius,buttonPinRadius);
        translate([0,0,linkHeight+h-1])
        cylinder(1,buttonPinRadius,buttonPinRadius-1);

    }
    // Switches
    switchX = i*switchSpacingX;
    switchY = j*switchSpacingY+i*buttonsRankShiftY;
    deltaX = switchX-buttonX;
    deltaY = switchY-buttonY;
    angle = atan2(deltaX,deltaY)+90;
    distance = sqrt(deltaX^2+deltaY^2);
    translate([switchX,switchY,0])
    difference() {
	    union() {
            translate([-switchLengthX/2,-switchLengthY/2,0])
        	cube([switchLengthX,switchLengthY,switchHeight]);
            rotate(-angle,[0,0,1])
            translate([0,0,switchHeight])
            difference() {
                union() {
                    translate([0,-linkWidth/2,0])
                    cube([distance,linkWidth,linkHeight]);
                    cylinder(linkHeight,linkRadius,linkRadius);
                    translate([distance,0,0])
                    cylinder(linkHeight,linkRadius,linkRadius);

                }
                union() {
                    translate([distance,0,0])
                    cylinder(plugRadius,0,plugRadius);
                    translate([distance,0,plugRadius])
                    cylinder(linkHeight-plugRadius+1,plugRadius,plugRadius);
                }
            }
        }
		union() {
			cube([switchCrossLength,switchCrossWidth,switchCrossDepth*2],true);
			cube([switchCrossWidth,switchCrossLength,switchCrossDepth*2],true);
		}
	}          
}
