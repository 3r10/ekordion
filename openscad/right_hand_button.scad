
$fn=600;
angle = 11; // 11
buttonDiameter = 14.5;
buttonHeight1 = 3;
buttonHeight2 = 1;
pinDiameter = 5;
pinHeight = 4;
crossLength = 4;
crossWidth = 1.3;
crossDepth = 3.6;

buttonRadius = buttonDiameter/2;
pinRadius = pinDiameter/2;

// for (i = [0:11],j = [0:2]) {
	// translate([i*18.9+j*9.45,j*18,0])
	union() {
		rotate(angle,[-1,0,0])
		translate ([0,0,buttonHeight2])
		union() {
			// semi-sphere
			scale([1,1,buttonHeight1/buttonRadius])
			difference() {
				sphere(buttonRadius);
				translate([0,0,-buttonRadius]) cube([buttonDiameter,buttonDiameter,buttonDiameter],center=true);
			}
			//ù
			translate([0,0,-buttonHeight2])
			cylinder(buttonHeight2,buttonRadius,buttonRadius);
		}
		translate([0,0,-pinHeight])
		difference() {
			cylinder(pinHeight+pinRadius*tan(angle),pinRadius,pinRadius);
			union() {
				translate([-crossLength/2,-crossWidth/2,-crossDepth]) 
				cube([crossLength,crossWidth,crossDepth*2]);
				translate([-crossWidth/2,-crossLength/2,-crossDepth]) 
				cube([crossWidth,crossLength,crossDepth*2]);
			}
		}
	}
// }