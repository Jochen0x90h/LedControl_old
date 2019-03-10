// config variables

// casing
coverZ1 = 9;
coverFit = 0.5;
coverOverlap = 2.5;

// potis (Bourns PEC12R-4215F-S0024)
potiX = 22;
potiY = -22;
potiL = 15; // length starting at pcb top according to data sheet
potiB = 6.1; // height of body (with switch)
potiLB = 2; // length of shaft bearing according to data sheet
potiF = 5; // shaft cutout according to data sheet
wheelR = 19.5; // radius of wheel
wheelGap = 0.5; // visible gab between box and wheel

// pcb
pcbWidth = 70;
pcbHeight = 35;
pcbThickness = 1.6;

// power connectors
connectorThickness = 4.5;

// usb port
usbX = 24;
usbWidth = 8;
usbThickness = 3;

// display panel
panelWidth = 60.5+0.5;
panelHeight = 37+0.5;
panelThickness = 2.3;
cableWidth = 13;
cableWidth2 = 16;

// screen (active area of display)
screenX = 0;
screenY = 20.5;
screenWidth = 57.01;
screenHeight = 29.49;
screenOffset = 1.08+0.25; // distance between upper panel border and upper screen border

// dependent variables

// depth of whole casing (cover and base)
baseZ2 = coverZ1+coverOverlap;
coverZ2 = 1+potiL+pcbThickness+connectorThickness+1;

// pcb
pcbX = 0; // x center of pcb
pcbX1 = pcbX-pcbWidth/2;
pcbX2 = pcbX+pcbWidth/2;
pcbY1 = -pcbHeight;
pcbY2 = 0;
pcbZ2 = coverZ2-1-potiL; // mounting surface of potis on pcb
pcbZ1 = pcbZ2-pcbThickness;
pcbY = (pcbY1+pcbY2)/2; // y center of pcb

// display panel
panelX1 = screenX-panelWidth/2; // left border of panel
panelX2 = screenX+panelWidth/2; // right border of panel
panelY2 = screenY+screenHeight/2+screenOffset; // upper border of panel
panelY1 = panelY2-panelHeight; // lower border of panel
panelY = (panelY2+panelY1)/2;
panelZ1 = coverZ2-1-panelThickness;
panelZ2 = coverZ2-1;

// screen
screenX1 = screenX-screenWidth/2;
screenX2 = screenX+screenWidth/2;
screenY1 = screenY-screenHeight/2;
screenY2 = screenY+screenHeight/2;


// cuboid with center at (x, y, z+d/2)
module box(x, y, z, w, z1, z2) {
	translate([x-w/2, y-h/2, z1])
		cube([w, h, z2-z1]);
}

module cuboid(x1, y1, z1, x2, y2, z2) {
	translate([x1, y1, z1])
		cube([x2-x1, y2-y1, z2-z1]);
}

module barrel(x, y, r, z1, z2) {
	translate([x, y, z1])
		cylinder(r=r, h=z2-z1);
}

module longHoleX(x, y, r, l, z1, z2) {
	barrel(x=x-l/2, y=y, r=r, z1=z1, z2=z2);
	barrel(x=x+l/2, y=y, r=r, z1=z1, z2=z2);
	box(x=x, y=y, w=l, h=r*2, z1=z1, z2=z2);
}

module longHoleY(x, y, r, l, z1, z2) {
	barrel(x=x, y=y-l/2, r=r, z1=z1, z2=z2);
	barrel(x=x, y=y+l/2, r=r, z1=z1, z2=z2);
	box(x=x, y=y, w=r*2, h=l, z1=z1, z2=z2);
}

module frustum(x, y, w1, h1, w2, h2, z1, z2) {
	// https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Primitive_Solids#polyhedron	
	points = [
		// lower square
		[x-w1/2,  y-h1/2, z1],  // 0
		[x+w1/2,  y-h1/2, z1],  // 1
		[x+w1/2,  y+h1/2, z1],  // 2
		[x-w1/2,  y+h1/2, z1],  // 3
		// upper square
		[x-w2/2,  y-h2/2, z2],  // 4
		[x+w2/2,  y-h2/2, z2],  // 5
		[x+w2/2,  y+h2/2, z2],  // 6
		[x-w2/2,  y+h2/2, z2]]; // 7
	faces = [
		[0,1,2,3],  // bottom
		[4,5,1,0],  // front
		[7,6,5,4],  // top
		[5,6,2,1],  // right
		[6,7,3,2],  // back
		[7,4,0,3]]; // left  
	polyhedron(points, faces);
}

module wheel(select) {
	x = potiX * select;
	y = potiY;

	// wheel
	difference() {
		// wheel with 3mm thickness
		barrel(x=x, y=y, r=wheelR, z1=coverZ2-3, z2=coverZ2);

		// cutout for shaft down to 1mm thickness
		box(x=x, y=y, w=10, h=4.5, z1=1, z2=coverZ2-1);
		box(x=x, y=y, w=4.5, h=10, z1=1, z2=coverZ2-1);
	}
	
	// shaft holder for 6mm shaft
	difference() {
		intersection() {
			// box that has 1mm air gap on sides
			box(x=x, y=y, w=8, h=8, z1=coverZ2-1-potiF, z2=coverZ2-2);

			// make round corners
			barrel(x=x, y=y, r=5, z1=1, z2=coverZ2-1);
		}

		// subtract rounded hole
		intersection() {
			box(x=x, y=y, w=5.8, h=5.8, z1=1, z2=coverZ2-1);
			barrel(x=x, y=y, r=3.5, z1=1, z2=coverZ2-1);
		}
	}
	
	// shaft cutaway
	difference() {
		// fill shaft cutaway which is 1.5mm
		union() {
			box(x=x, y=y+2.4, w=1, h=2, z1=coverZ2-1-potiF, z2=coverZ2-2);
			box(x=x, y=y+1.9, w=2, h=1, z1=coverZ2-1-potiF, z2=coverZ2-2);
		}

		// subtract slanted corners to ease insertion of poti shaft
		translate([x, y+1.5, coverZ2-1-potiF-0.5])
			rotate([30, 0, 0])
				box(x=0, y=0, w=4, h=1, z1=0, z2=2);
	}
}

module wheelBase(x, y) {
	// base in z-direction: 3mm wheel, 1mm air, 2mm base
	barrel(x=x, y=y, r=wheelR+wheelGap+2, z1=coverZ2-3-1-2, z2=coverZ2);

	h1 = pcbZ2+potiB;
	h3 = coverZ2-1-potiF-3;
	
	// shaft radial: 5mm wheel shaft, air gap, 1mm wall
	barrel(x=x, y=y, r=5+wheelGap+1, z1=h3, z2=coverZ2);

	// shaft radial: 3.5mm poti shaft, no air gap, 1mm wall
	barrel(x=x, y=y, r=3.5+1, z1=h1, z2=coverZ2);

	box(x=x, y=y+12.4/2+1, w=13.4, h=2, z1=pcbZ2, z2=coverZ2);
	box(x=x, y=y-12.4/2-1, w=13.4, h=2, z1=pcbZ2, z2=coverZ2);
	//box(x=x, y=y+12.2/2+1, z=pcbZ2+0.5, w=13.4, h=2, d=1);
	//box(x=x, y=y-12.2/2-1, z=pcbZ2+0.5, w=13.4, h=2, d=1);
}

module potiCutout(x, y) {
	// cutout for wheel: 3mm wheel, 1mm air
	barrel(x=x, y=y, r=wheelR+wheelGap, z1=coverZ2-3-1, z2=coverZ2+1);

	h1 = pcbZ2;
	h2 = pcbZ2+potiB+potiLB;
	h3 = coverZ2-1-potiF-1;

	// hole for wheel shaft: 5mm wheel shaft, air gap
	barrel(x=x, y=y, r=5+wheelGap, z1=h3, z2=coverZ2);
	
	// hole for poti shaft: 3mm poti shaft (6mm diameter), tolerance
	barrel(x=x, y=y, r=3+0.1, z1=h1, z2=coverZ2);
	
	// hole for poti shaft bearing: 3.5mm poti shaft bearing, tolerance
	translate([x, y, h1])
		cylinder(r1=4, r2=3.5+0.1, h=h2-h1);
}

module snap(y, l) {
	translate([0, y, coverZ1+1])
		rotate([45, 0, 0])
			cuboid(x1=-l/2, x2=l/2, y1=-0.65, y2=0.65, z1=-0.65, z2=0.65);
}

module base() {
color([0.3, 0.3, 1]) {
	difference() {
		union() {
			difference() {
				// base with slanted walls
				union() {
					box(x=0, y=0, w=76-coverFit, h=76-coverFit,
						z1=0, z2=baseZ2-0.5);
					frustum(x=0, y=0,
						w1=76-coverFit, h1=76-coverFit,
						w2=76-coverFit-0.2, h2=76-coverFit-0.2,
						z1=baseZ2-0.5, z2=baseZ2);
				}
				
				// subtract inner volume
				box(x=0, y=0, w=72, h=72, z1=2, z2=baseZ2+1);
			
				// subtract snap lock
				snap(-76/2, 44);
				snap(76/2, 44);			
			}
			
			// add reinforcement for screw holes
			box(x=0, y=12, w=74, h=3, z1=0, z2=4);
		}
		
		// subtract cable hole
		longHoleX(x=0, y=6, r=4, l=8, z1=-1, z2=6);
		
		// subtract mounting screw holes
		longHoleY(x=-12.5, y=6, r=1.5, l=1, z1=-1, z2=6);
		longHoleY(x=12.5, y=6, r=1.5, l=1, z1=-1, z2=6);
		longHoleY(x=-32, y=6, r=1.5, l=1, z1=-1, z2=6);
		longHoleY(x=32, y=6, r=1.5, l=1, z1=-1, z2=6);
		
		// subtract connector cutouts
		box(x=pcbX1+10, y=pcbY2-7.5, w=20, h=15, z1=0.8, z2=3);
		box(x=pcbX2-10, y=pcbY2-7.5, w=20, h=15, z1=0.8, z2=3);
		
		// subtract micro sd slot
		//box(x=0, y=-37, z=pcbZ1-0.5, w=11.5, h=10, d=1);

		// subtract micro usb slot
		box(x=-usbX, y=-37, w=usbWidth, h=10,
			z1=pcbZ1-usbThickness, z2=pcbZ1);
	}

	// poti supports
	box(x=-potiX+6, y=potiY, w=2, h=2, z1=0, z2=pcbZ1);
	box(x=potiX-6, y=potiY, w=2, h=2, z1=0, z2=pcbZ1);
	box(x=-potiX-10, y=potiY, w=2, h=2, z1=0, z2=pcbZ1);
	box(x=potiX+10, y=potiY, w=2, h=2, z1=0, z2=pcbZ1);
} // color
}

module cover() {
color([1, 0, 0]) {
	difference() {
		union() {
			// case with 2mm wall thickness
			difference() {
				box(x=0, y=0, w=80, h=80, z1=coverZ1, z2=coverZ2);
				box(x=0, y=0, w=76, h=76, z1=coverZ1-1, z2=coverZ2-2);
			}
			
			intersection() {
				// poti bases
				union() {
					wheelBase(x=-potiX, y=potiY);
					wheelBase(x=potiX, y=potiY);
				}
			
				// cut away poti bases outside of box and at display
				cuboid(x1=-40, y1=-40, z1=pcbZ2, x2=40, y2=panelY1, z2=coverZ2);
			}
			
			// display holders
			box(x=panelX1, y=panelY, w=2, h=panelHeight+2,
				z1=panelZ1-1, z2=coverZ2);
			box(x=panelX2+0.5, y=panelY, w=1, h=panelHeight+4, 
				z1=baseZ2, z2=panelZ1); 
			translate([panelX2, panelY, panelZ1-4])
				rotate([0, -7, 0])
					box(x=0, y=0, w=1, h=6, z1=-4, z2=4); 
			
			// pcb holders
			box(x=pcbX1+9, y=pcbY2-3, w=18, h=2, z1=pcbZ2, z2=coverZ2);
			box(x=pcbX2-9, y=pcbY2-3, w=18, h=2, z1=pcbZ2, z2=coverZ2);
		}
		
		// subtract poti cutouts for wheel and axis
		potiCutout(x=-potiX, y=potiY);
		potiCutout(x=potiX, y=potiY);		

		// subtract display screen (activa area) window
		frustum(x=screenX, y=screenY,
				w1=screenWidth-4, h1=screenHeight-4,
				w2=screenWidth+4, h2=screenHeight+4,
				z1=coverZ2-3, z2=coverZ2+1);

		// subtract display panel (glass carrier) cutout
		cuboid(x1=panelX1, y1=panelY1, z1=panelZ1,
				x2=panelX2, y2=panelY2, z2=coverZ2-1);

		// subtract display cable cutout
		box(x=screenX, y=panelY1, w=cableWidth2, h=6,
			z1=panelZ2-10, z2=panelZ2+0.1);
		cuboid(x1=screenX-12, x2=screenX+12, y1=panelY1, y2=screenY1-0.5,
			z1=coverZ2-10, z2=panelZ2+0.1);
		frustum(x=screenX, y=panelY1, 
				w1=cableWidth, h1=45,
				w2=cableWidth, h2=6,
				z1=panelZ2-10, z2=panelZ2-0.5);
	}

	// snap lock between upper and lower case
	snap(-(76)/2, 40);
	snap((76)/2, 40);

	// cable holder
	box(x=-30, y=30, w=16, h=2, z1=baseZ2, z2=baseZ2+4);
	box(x=-23, y=34, w=2, h=10, z1=baseZ2, z2=baseZ2+4);
} // color
}

module pcb() {
	color([0, 0.6, 0, 0.3])
	box(x=pcbX, y=pcbY, w=pcbWidth, h=pcbHeight,
		z1=pcbZ1, z2=pcbZ2);
}

module poti(select) {
	x = potiX * select;
	y = potiY;
	color([0.5, 0.5, 0.5]) {
		box(x=x, y=y, w=13.4, h=12.4, z1=pcbZ2, z2=pcbZ2+potiB);
		box(x=x, y=y, w=15, h=6, z1=pcbZ2, z2=pcbZ2+4);
		barrel(x=x, y=y, r=3.5, z1=pcbZ2, z2=pcbZ2+potiB+potiLB);
		barrel(x=x, y=y, r=3, z1=pcbZ2, z2=pcbZ2+potiL);
	}
}

module usb() {
	color([0.5, 0.5, 0.5]) {
		box(x=-usbX, y=pcbY1+2, w=usbWidth, h=8,
			z1=pcbZ1-usbThickness, z2=pcbZ1);		
	}
}

// casing parts that need to be printed
//base();
cover();
wheel(-1);
wheel(1);


// reference parts
//pcb();
poti(-1);
poti(1);
//usb();
