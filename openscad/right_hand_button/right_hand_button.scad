
$fn=200;
angle = 11; // 11
buttonDiameter = 14.5;
buttonHeight1 = 3; // scaled semi-sphere
buttonHeight2 = 2; // cylindric part
pinLength = 6.5;
pinWidth  = 5;
pinDepth = 4;
crossLength = 4;
crossWidth = 1.3;
crossDepth = 3.6;

buttonRadius = buttonDiameter/2;
union() {
	rotate(angle,[-1,0,0])
	translate ([0,0,buttonHeight2])
	union() {
		// top part (tilted)
		scale([1,1,buttonHeight1/buttonRadius])
		difference() {
			sphere(buttonRadius);
			translate([0,0,-buttonRadius]) cube([buttonDiameter,buttonDiameter,buttonDiameter],center=true);
		}
		translate([0,0,-buttonHeight2])
		cylinder(buttonHeight2,buttonRadius,buttonRadius);
	}
	translate([0,0,-pinDepth])
	// interface with CHerry MX switch
	difference() {
		translate([-pinLength/2,-pinWidth/2,0]) 
		cube([pinLength,pinWidth,pinDepth+pinWidth*tan(angle)]);
		union() {
			translate([-crossLength/2,-crossWidth/2,-crossDepth]) 
			cube([crossLength,crossWidth,crossDepth*2]);
			translate([-crossWidth/2,-crossLength/2,-crossDepth]) 
			cube([crossWidth,crossLength,crossDepth*2]);
		}
	}
}
