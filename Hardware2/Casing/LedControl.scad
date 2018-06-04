
coverZ = 9;
coverFit = 0.5;
overlap = 2.5;

// potis (Bourns PEC12R-4215F-S0024)
potiX = 22;
potiY = -22;
potiL = 15; // length starting at pcb top according to data sheet
potiB = 6.1; // height of body (depends on switch)
potiLB = 2; // length of shaft bearing according to data sheet
potiF = 5; // shaft cutout according to data sheet
wheelR = 19.5; // radius of wheel
wheelGap = 0.5; // visible gab between box and wheel

// pcb
pcbWidth = 70;
pcbHeight = 35;
pcbThickness = 1.6;

// depth of whole cuboid (cover and base)
depth = 1+potiL+pcbThickness+2+3.5;

// pcb
pcbX = 0; // x center of pcb
pcbX1 = pcbX-pcbWidth/2;
pcbX2 = pcbX+pcbWidth/2;
pcbY1 = -pcbHeight;
pcbY2 = 0;
pcbZ2 = depth-1-potiL; // mounting surface of potis on pcb
pcbZ1 = pcbZ2-pcbThickness;
pcbY = (pcbY1+pcbY2)/2; // y center of pcb

// display
screenX = 0;
screenY = 20.5;
panelWidth = 60.5+0.5;
panelHeight = 37+0.5;
panelThickness = 2.3;
cableWidth = 13;
screenWidth = 57.01;
screenHeight = 29.49;
screenOffset = 1.08+0.25; // distance between upper panel border and upper screen border
panelX1 = screenX-panelWidth/2; // left border of panel
panelX2 = screenX+panelWidth/2; // right border of panel
panelY2 = screenY+screenHeight/2+screenOffset; // upper border of panel
panelY1 = panelY2-panelHeight; // lower border of panel
panelY = (panelY2+panelY1)/2;

// cuboid with center at (x, y, z+d/2)
module box(x, y, z, w, h, d) {
	translate([x-w/2, y-h/2, z])
		cube([w, h, d]);
}

module cuboid(x1, y1, z1, x2, y2, z2) {
	translate([x1, y1, z1])
		cube([x2-x1, y2-y1, z2-z1]);
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

module wheel(x, y) {
	// wheel
	difference() {
		// wheel with 3mm thickness
		translate([x, y, depth-3])
			cylinder(r=wheelR, h=3);

		// cutout for shaft down to 1mm thickness
		box(x=x, y=y, z=depth-4, w=10, h=4.5, d=3);
		box(x=x, y=y, z=depth-4, w=4.5, h=10, d=3);
	}
	
	// shaft holder for 6mm shaft (5mm outer radius)
	difference() {
		intersection() {
			box(x=x, y=y, z=depth-1-potiF, w=8, h=8, d=potiF-1);
			translate([x, y, depth-1-potiF])
				cylinder(r=5, h=potiF-1);
		}

		// subtract rounded hole
		intersection() {
			box(x=x, y=y, z=depth-1-potiF-1, w=5.8, h=5.8, d=potiF+1);
			translate([x, y, depth-1-potiF-1])
				cylinder(r=3.5, h=potiF+1);
		}

		// subtract notch to ease insertion of poti shaft
		//translate([x, y, depth-1-potiF-0.5])
		//	cylinder(r1=3.2, r2=2.9, h=1.5);
	}
	
	// shaft cutaway
	difference() {
		// fill shaft cutaway which is 1.5mm
		union() {
			box(x=x, y=y+2.4, z=depth-1-potiF, w=1, h=2, d=potiF-1);
			box(x=x, y=y+1.9, z=depth-1-potiF, w=2, h=1, d=potiF-1);	
		}

		// subtract slanted corners to ease insertion of poti shaft
		translate([x, y+1.5, depth-1-potiF-0.5])
			rotate([30, 0, 0])
				box(x=0, y=0, z=0, w=4, h=1, d=2);
	}
}

module potiBase(x, y) {
	// base in z-direction: 3mm wheel, 1mm air, 2mm base
	translate([x, y, depth-3-1-2])
		cylinder(r=wheelR+wheelGap+2, h=6);

	h1 = pcbZ2+potiB;
	h3 = depth-1-potiF-3;
	
	// shaft radial: 5mm wheel shaft, air gap, 1mm wall
	translate([x, y, h3])
		cylinder(r=5+wheelGap+1, h=depth-h3);

	// shaft radial: 3.5mm poti shaft, 1mm wall
	translate([x, y, h1])
		cylinder(r=3.5+1, h=depth-h1);

	box(x=x, y=y+12.4/2+1, z=pcbZ2, w=13.4, h=2, d=potiL);
	box(x=x, y=y-12.4/2-1, z=pcbZ2, w=13.4, h=2, d=potiL);
	box(x=x, y=y+12.2/2+1, z=pcbZ2+0.5, w=13.4, h=2, d=1);
	box(x=x, y=y-12.2/2-1, z=pcbZ2+0.5, w=13.4, h=2, d=1);
}

module potiCutout(x, y) {
	// cutout for wheel: 3mm wheel, 1mm air
	translate([x, y, depth-3-1])
		cylinder(r=wheelR+wheelGap, h=5);	

	h1 = pcbZ2;
	h2 = pcbZ2+potiB+potiLB;
	h3 = depth-1-potiF-1;

	// hole for wheel shaft: 5mm wheel shaft, air gap
	translate([x, y, h3])
		cylinder(r=5+wheelGap, h=depth-h3);
	
	// hole for poti shaft: 3mm poti shaft (6mm diameter), air gap
	translate([x, y, h1])
		cylinder(r=3+0.1, h=depth-h1);

	// hole for poti shaft bearing: 3.5mm poti shaft bearing, air gap
	translate([x, y, h1])
		cylinder(r=3.5+0.1, h=h2-h1);
	
	// make insertion easier
	translate([x, y, h1-0.5])
		cylinder(r1=3.9, r2=3.5, h=1.5);
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
	translate([-l/2, y, coverZ+1])		
		rotate([0, 90, 0])
			rotate([0, 0, 45])
				box(x=0, y=0, z=0, w=1.3, h=1.3, d=l);	
}

module base() {
color([0.3, 0.3, 1]) {
	difference() {
		union() {
			difference() {
				// base with slanted walls
				union() {
					box(x=0, y=0, z=0,
							w=76-coverFit, h=76-coverFit, d=coverZ+overlap);
					//frustum(x=0, y=0, z=coverZ+3, w1=76-coverFit, h1=76-coverFit,
							//w2=75, h2=75, d=1);
				}
				
				// subtract inner volume
				box(x=0, y=0, z=2, w=72, h=72, d=coverZ+overlap);
			
				// subtract snap lock
				snap(-(76)/2, 44);
				snap((76)/2, 44);			
			}
			
			// add reinforcement for screw holes
			box(x=0, y=12, z=0, w=74, h=3, d=4);
		}
		
		// subtract cable hole
		translate([0, 6, -1])
			longHoleH(r=4, h=7, l=8);
		
		// subtract mounting screw holes
		translate([-12.5, 6, -1])
			longHoleV(r=1.5, h=7, l=1);
		translate([12.5, 6, -1])
			longHoleV(r=1.5, h=7, l=1);
		translate([-32, 6, -1])
			longHoleV(r=1.5, h=7, l=1);
		translate([32, 6, -1])
			longHoleV(r=1.5, h=7, l=1);
		
		// subtract connector cutouts
		box(x=pcbX1+10, y=pcbY2-7.5, z=0.8, w=20, h=15, d=3);
		box(x=pcbX2-10, y=pcbY2-7.5, z=0.8, w=20, h=15, d=3);
		
		// subtract micro sd slot
		//box(x=0, y=-37, z=pcbZ1-0.5, w=11.5, h=10, d=1);

		// subtract micro usb slot
		box(x=0, y=-37, z=pcbZ1-1, w=6, h=10, d=2);
	}

	// poti supports
	box(x=-potiX, y=potiY, z=0, w=2, h=2, d=pcbZ1);
	box(x=potiX, y=potiY, z=0, w=2, h=2, d=pcbZ1);
} // color
}

module cover() {
color([1, 0, 0]) {
	difference() {
		union() {
			// case with 2mm wall thickness
			difference() {
				box(x=0, y=0, z=coverZ, w=80, h=80, d=depth-coverZ);
				box(x=0, y=0, z=coverZ-1, w=76, h=76, d=depth-coverZ-1);
			}
			
			intersection() {
				// poti bases
				union() {
					potiBase(x=-potiX, y=potiY);
					potiBase(x=potiX, y=potiY);
				}
			
				// cut away poti bases outside of box and at display
				cuboid(x1=-40, y1=-40, z1=0, x2=40, y2=panelY1, z2=depth);
			}
			
			// display holders
			box(x=panelX1, y=panelY, z=depth-1-panelThickness-1,
					w=2, h=panelHeight+2, d=1+panelThickness);
			box(x=panelX2+0.5, y=panelY, z=depth-1-panelThickness-8,
					w=1, h=panelHeight+4, d=8); 
			translate([panelX2, panelY, depth-1-panelThickness-4])
				rotate([0, -7, 0])
					box(x=0, y=0, z=-4, w=1, h=5, d=8); 
			
			// pcb holders
			box(x=pcbX1+9, y=pcbY2-3, z=pcbZ2, w=18, h=2, d=depth-pcbZ2);
			box(x=pcbX2-9, y=pcbY2-3, z=pcbZ2, w=18, h=2, d=depth-pcbZ2);
		}
		
		// poti cutouts for wheel and axis
		potiCutout(x=-potiX, y=potiY);
		potiCutout(x=potiX, y=potiY);		

		// display screen (activa area) window
		frustum(x=screenX, y=screenY, z=depth-3,
				w1=screenWidth-4, h1=screenHeight-4,
				w2=screenWidth+4, h2=screenHeight+4, d=4);

		// display panel (glass carrier) cutout
		cuboid(x1=panelX1, y1=panelY1, z1=depth-1-panelThickness,
				x2=panelX2, y2=panelY2, z2=depth-1);

		// display cable cutout
		box(x=screenX, y=panelY1, z=depth-11,
				w=cableWidth+4, h=6, d=10);
		frustum(x=screenX, y=panelY1, z=depth-11,
				w1=cableWidth, h1=40,
				w2=cableWidth, h2=6, d=10);
	}

	// snap lock between upper and lower case
	snap(-(76)/2, 40);
	snap((76)/2, 40);

	// blockers for base
	box(x=37.5, y=37.5, z=coverZ+overlap, w=3, h=3, d=2);
	box(x=-37.5, y=37.5, z=coverZ+overlap, w=3, h=3, d=2);
	box(x=37.5, y=-35, z=coverZ+overlap, w=3, h=8, d=2);
	box(x=-37.5, y=-35, z=coverZ+overlap, w=3, h=8, d=2);

	// cable holder
	box(x=-30, y=30, z=depth/2, w=16, h=2, d=4);
	box(x=-23, y=34, z=depth/2, w=2, h=10, d=4);
} // color
}

module pcb() {
	color([0, 0.6, 0, 0.3])
	box(x=pcbX, y=pcbY, z=pcbZ1,
			w=pcbWidth, h=pcbHeight, d=pcbThickness);
}

module poti(x, y) {
	color([0.5, 0.5, 0.5]) {
		box(x=x, y=y, z=pcbZ, w=13.4, h=12.4, d=potiB);
		box(x=x, y=y, z=pcbZ, w=15, h=6, d=4);
		translate([x, y, pcbZ]) {
			cylinder(r=3.5, h=potiB+potiLB);
			cylinder(r=3, h=potiL);
		}
	}
}

//base();
cover();
/*
rotate([180, 0, 0]) {
	translate([-17, -14, -32])
		potiWheel(x=0, y=0);
	translate([17, 11, -32])
		potiWheel(x=0, y=0);
}
*/
//wheel(x=-potiX, y=potiY);
//wheel(x=potiX, y=potiY);
pcb();

// poti
//poti(-potiX, potiY);
//poti(potiX, potiY);
