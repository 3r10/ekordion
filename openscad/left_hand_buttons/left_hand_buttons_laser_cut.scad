$fn=200;

globalLengthX = 110;
globalLengthY = 146;

centerDeltaX = 35;
centerDeltaY = 73.75;

buttonPinDiameter = 10;

buttonsNumberX = 4;
buttonsNumberY = 6;
buttonsSpacingX = 11.2;
buttonsSpacingY = 15;
buttonsRankShiftY = 7.5;

mountingHolesRadius = 1.6;

buttonPinRadius = buttonPinDiameter/2;

difference() {
    square([globalLengthX,globalLengthY]);
    union() {
        translate([67+buttonPinRadius,10+buttonPinRadius])
        minkowski() {
            square([33-2*buttonPinRadius,126-2*buttonPinRadius]);
            circle(buttonPinRadius);
        }
        minkowski() {
            polygon([
                [29.2,136-buttonPinRadius],
                [18,124],
                [18,136-buttonPinRadius],
            ]);
            circle(buttonPinRadius);
        }
        minkowski() {
            polygon([
                [35,10+buttonPinRadius],
                [35+11.2,10+buttonPinRadius],
                [35+11.2,22]
            ]);
            circle(buttonPinRadius);
        }
        translate([5,5])
        circle(mountingHolesRadius);
        translate([5,141])
        circle(mountingHolesRadius);
        translate([5,117])
        circle(mountingHolesRadius);
        translate([59,29])
        circle(mountingHolesRadius);
        translate([59,141])
        circle(mountingHolesRadius);
        translate([105,5])
        circle(mountingHolesRadius);
        translate([105,141])
        circle(mountingHolesRadius);
        translate([centerDeltaX,centerDeltaY])
        union() {
            // circle(1);
            for (i = [0:buttonsNumberX-1],j = [0:buttonsNumberY-1]) {
                buttonX = (1.5-i)*buttonsSpacingX;
                buttonY = (2.5-j)*buttonsSpacingY+(1.5-i)*buttonsRankShiftY;
                translate([buttonX,buttonY])
                circle(buttonPinRadius);
            }
        }
    }
}