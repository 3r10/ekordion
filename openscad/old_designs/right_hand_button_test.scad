// Testing inner cross dimensions
// 3 x 3 with various length & width
$fn=600;
for (i = [0:2],j = [0:2]) {
    pinDiameter = 5;
    pinHeight = 5;
    // Datasheet gives :
    // length=4 and width=1.3
    crossLength = 4+0.1*i;
    crossWidth = 1.3+0.1*j;
    crossDepth = 3.6;

    pinRadius = pinDiameter/2;
    translate([i*6,j*6,0])
    difference() {
        cylinder(pinHeight,pinRadius,pinRadius);
        union() {
            translate([-crossLength/2,-crossWidth/2,pinHeight-crossDepth]) 
            cube([crossLength,crossWidth,crossDepth*2]);
            translate([-crossWidth/2,-crossLength/2,pinHeight-crossDepth]) 
            cube([crossWidth,crossLength,crossDepth*2]);
        }
    }
}
// Linking the 9 tests
for (i = [0:2]) {
    translate([6*i-0.5,0,0])
    cube([1,12,1]);
    translate([0,6*i-0.5,0])
    cube([12,1,1]);
}
// Origin indicator
rotate(135,[0,0,1])
translate([-0.5,0,0])
cube([1,3.5,1]);
