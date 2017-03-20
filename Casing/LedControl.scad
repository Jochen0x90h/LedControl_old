depth = 26;

coverZ = 6;
coverFit = 0.5;

// pcb
pcbWidth = 68;
pcbHeight = 34;
pcbThickness = 1.6;
pcbX = 0;
pcbY = -35;
pcbZ = depth-1-20;

// mounting positions of pcb
mountWidth = pcbWidth;
mountHeight = pcbHeight-6;
mountX1 = pcbX-mountWidth/2;
mountX2 = pcbX+mountWidth/2;
mountY1 = pcbY+3;
mountY2 = pcbY+3+mountHeight;

// potis
potiX = 22;
potiY = -22;
potiR = 19.5;
potiGap = 0.5;

// display
displayX = 0;
displayY = 20;
panelWidth = 42.04 + 0.5;//34.5 + 0.5;
panelHeight = 27.22 + 0.5;//23 + 0.5;
lowerPanelThickness = 1;
upperPanelThickness = 2;
screenWidth = 37;//30.42;
screenHeight = 19.5;//15.7;
screenOffset = 2; // distance between upper panel border and upper screen border
panelX1 = displayX-panelWidth/2;
panelX2 = displayX+panelWidth/2;
panelY2 = displayY+screenHeight/2+screenOffset; // upper border of panel
panelY1 = panelY2-panelHeight; // lower border of panel

module cuboid(x, y, z, w, h, d) {
	translate([x-w/2, y-h/2, z])
		cube([w, h, d]);
}

module frustum(x, y, z, w1, h1, w2, h2, d) {
	// https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Primitive_Solids#polyhedron	
	points = [
		// lower square
		[x-w1/2,  y-h1/2, z],  // 0
		[x+w1/2,  y-h1/2, z],  // 1
		[x+w1/2,  y+h1/2, z],  // 2
		[x-w1/2,  y+h1/2, z],  // 3
		// upper square
		[x-w2/2,  y-h2/2, z+d],  // 4
		[x+w2/2,  y-h2/2, z+d],  // 5
		[x+w2/2,  y+h2/2, z+d],  // 6
		[x-w2/2,  y+h2/2, z+d]]; // 7
	faces = [
		[0,1,2,3],  // bottom
		[4,5,1,0],  // front
		[7,6,5,4],  // top
		[5,6,2,1],  // right
		[6,7,3,2],  // back
		[7,4,0,3]]; // left  
	polyhedron(points, faces);
}

module potiWheel(x, y) {
	difference() {
		union() {
			// wheel
			translate([x, y, depth-3])
				cylinder(r=potiR, h=3);
			
			// shaft
			translate([x, y, depth-9])
				cylinder(r=5, h=9);
		}
	
		// hole for poti axis
		difference() {
			translate([x, y, depth-12])
				cylinder(r=3, h=11);
			translate([x+1, y-3, depth-7])
				cube([6, 6, 6]);
		}
	}
}

module potiBase(x, y) {
	// base: 2mm base, 1mm air, 3mm wheel
	translate([x, y, depth-6])
		cylinder(r=potiR+potiGap+2, h=6);

	// axis: 3mm poti axis, 2mm wheel axis, 1mm air, 2mm base
	translate([x, y, depth-12])
		cylinder(r=8, h=12);
}

module potiCutout(x, y) {
	// cutout for wheel
	translate([x, y, depth-4])
		cylinder(r=potiR+potiGap, h=5);	

	// hole for wheel axis: 3mm poti axis, 2mm wheel axis, 1mm air
	translate([x, y, depth-11])
		cylinder(r=6, h=11);
	
	// hole for poti axis: 3mm poti axis, 0.5mm poti metal case
	translate([x, y, depth-13])
		cylinder(r1=3.8, r2=3.2, h=3);
}

module upperDisplayHolder(x, y, length) {
	// pillar
	translate([x-3, y+1, depth-1-length])
		cube([6, 2, length]);
	
	// connection from pillar to "spring" clip
	translate([x-3, y-1, depth-1-length-2])
		cube([6, 4, 2]);

	// "spring" clip
	translate([x-3, y-1, depth-1-length])
		cube([6, 1, length-upperPanelThickness]);
}

module lowerDisplayHolder(x, y) {
	// pillar (not needed if panel is not thicker than its cutout)
	translate([x-3, y-2, depth-1-lowerPanelThickness])
		cube([6, 2, lowerPanelThickness]);
	
	// overhanging clip
	translate([x-3, y-2, depth-1-lowerPanelThickness-2])
		cube([6, 3.5, 2]);
}

/*
module pcbHolder(x, y) {
	// pillar
	difference() {
		cuboid(x=x, y=y, z=pcbZ, w=4, h=6, d=depth-1-pcbZ);

		// hole 2.5mm diameter, 12mm deep
		translate([x, y, pcbX-1])
			cylinder(r=1.25, h=12+1);
	}
}
*/
module pcbHolder(x, y, o) {
	// pillar
	cuboid(x=x, y=y, z=pcbZ, w=2, h=6, d=depth-1-pcbZ);
	
	cuboid(x=x+o*0.5, y=y, z=pcbZ-3, w=1, h=6, d=4);
	
	frustum(x=x, y=y, z=pcbZ-3, w1=0.01, h1=6, w2=2, h2=6,
			d=3-pcbThickness);
}

module longHoleH(r, h, l) {
	translate([-l/2, 0, 0])
		cylinder(r=r, h=h);
	translate([l/2, 0, 0])
		cylinder(r=r, h=h);	
	translate([-l/2, -r, 0])
		cube([l, r*2, h]);	
}

module longHoleV(r, h, l) {
	translate([0, -l/2, 0])
		cylinder(r=r, h=h);
	translate([0, l/2, 0])
		cylinder(r=r, h=h);	
	translate([-r, -l/2, 0])
		cube([r*2, l, h]);	
}

module snap(y, l) {
	translate([-l/2, y, coverZ+2])		
		rotate([0, 90, 0])
			rotate([0, 0, 45])
				cuboid(x=0, y=0, z=0, w=1.3, h=1.3, d=l);	
}

module base() {
color([0, 0, 1]) {
difference() {
	union() {
		difference() {
			// base with slanted walls
			union() {
				cuboid(x=0, y=0, z=0, w=76-coverFit, h=76-coverFit, d=coverZ+3);
				frustum(x=0, y=0, z=coverZ+3, w1=76-coverFit, h1=76-coverFit,
						w2=75, h2=75, d=1);
			}
			cuboid(x=0, y=0, z=2, w=72, h=72, d=coverZ+4);
		
			// snap lock
			snap(-(76)/2, 44);
			snap((76)/2, 44);			
		}
		
		// reinforcement for screw holes
		cuboid(x=0, y=10, z=0, w=74, h=2, d=8);
	}
	
	// cable hole
	translate([0, 5, -1])
		longHoleH(r=4, h=7, l=8);
	
	// screw holes
	translate([-12.5, 5, -1])
		longHoleV(r=2, h=7, l=1);
	translate([12.5, 5, -1])
		longHoleV(r=2, h=7, l=1);
	
	translate([-30, 5, -1])
		longHoleV(r=2, h=7, l=1);
	translate([30, 5, -1])
		longHoleV(r=2, h=7, l=1);
	
	// poti cutouts
	translate([-potiX-9, potiY-9, 1])
		cube([18, 18, 2]);
	translate([potiX-9, potiY-9, 1])
		cube([18, 18, 2]);
		
	// pcb screw cutouts
	translate([mountX1, mountY1, 1])
		cylinder(r=3, h=2);
	translate([mountX2, mountY1, 1])
		cylinder(r=3, h=2);
	translate([mountX1, mountY2, 1])
		cylinder(r=3, h=2);
	translate([mountX2, mountY2, 1])
		cylinder(r=3, h=2);
}

// poti supports
translate([-potiX, potiY, 0])
	cylinder(r=3, h=depth-1-20-pcbThickness);
translate([potiX, potiY, 0])
	cylinder(r=3, h=depth-1-20-pcbThickness);

} // color
}

module cover() {
color([1, 0, 0]) {
intersection() {
	difference() {
		union() {
			// case with 2mm wall thickness
			difference() {
				cuboid(x=0, y=0, z=coverZ, w=80, h=80, d=depth-coverZ);
				cuboid(x=0, y=0, z=coverZ-1, w=76, h=76, d=depth-coverZ-1);
			}
			
			// poti bases
			potiBase(x=-potiX, y=potiY);
			potiBase(x=potiX, y=potiY);
			
			// pcb holders
			pcbHolder(x=mountX1, y=mountY1, o=-1);
			pcbHolder(x=mountX2, y=mountY1, o=1);
			pcbHolder(x=mountX1, y=mountY2, o=-1);
			pcbHolder(x=mountX2, y=mountY2, o=1);
		}
		
		// poti cutouts for wheel and axis
		potiCutout(x=-potiX, y=potiY);
		potiCutout(x=potiX, y=potiY);		

		// screen (activa area) cutout
		frustum(x=displayX, y=displayY, z=depth-3,
				w1=screenWidth-4, h1=screenHeight-4,
				w2=screenWidth+4, h2=screenHeight+4, d=4);

		// panel cutout
		cuboid(x=displayX, y=panelY2-panelHeight/2, z=depth-3,
				w=panelWidth, h=panelHeight, d=2);
		cuboid(x=displayX, y=panelY1, z=depth-3,
				w=16, h=8, d=2);
	}

	// cut away overhanging poti bases
	translate([-40, -40, 0])
		cube([80, 80, depth]);
}

// display holders
lowerDisplayHolder(x=panelX1+4, y=panelY1);
lowerDisplayHolder(x=panelX2-4, y=panelY1);
upperDisplayHolder(x=displayX, y=panelY2, length=10);

// snap lock
snap(-(76)/2, 40);
snap((76)/2, 40);

// blockers
cuboid(x=37.5, y=37.5, z=coverZ+4, w=3, h=3, d=2);
cuboid(x=-37.5, y=37.5, z=coverZ+4, w=3, h=3, d=2);
cuboid(x=37.5, y=-37.5, z=coverZ+4, w=3, h=3, d=2);
cuboid(x=-37.5, y=-37.5, z=coverZ+4, w=3, h=3, d=2);

// cable holder
cuboid(x=-30, y=30, z=depth/2, w=16, h=2, d=4);
cuboid(x=-23, y=34, z=depth/2, w=2, h=10, d=4);

} // color
}

module pcb() {
	color([0, 0.6, 0, 0.3])
	translate([-pcbWidth/2, pcbY, pcbZ-pcbThickness])
		cube([pcbWidth, pcbHeight, pcbThickness]);	
}

module poti(x, y) {
	color([0.5, 0.5, 0.5]) {
		cuboid(x=x, y=y, z=pcbZ, w=13, h=13, d=5.5);
		cuboid(x=x, y=y, z=pcbZ, w=16, h=7, d=5.5);
		translate([x, y, pcbZ]) {
			cylinder(r=3.5, h=10);
			cylinder(r=3, h=20);
		}
	}
}

//base();
cover();
//potiWheel(x=-potiX, y=potiY);
potiWheel(x=potiX, y=potiY);
//pcb();

// poti
poti(-potiX, potiY);
poti(potiX, potiY);

// wago
cuboid(x=-29, y=pcbY+pcbHeight-13/2, z=pcbZ, w=7.7, h=13, d=9.1);
cuboid(x=-21, y=pcbY+pcbHeight-13/2, z=pcbZ, w=7.7, h=13, d=9.1);
