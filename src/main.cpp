#include <iostream>
#include <cstdlib>
#include <unistd.h>
using namespace std;

#include "../headers/graphics.h"
//#include "../utils/simple_initializer.h" 
#include "../utils/simple_math.h"


int main(int argc, char ** argv){

	init_hyperGL(&argc, argv);
	
	float pos3[] = {0,50,0, 100,50,0, 50,0,0, 50,100,0, 0,0,0, 0,0,100};
	float col3[] = {1,0,0, 0.5,
				    1,0,0, 0.5,
				    0,1,0, 0.5,
				    0,1,0, 0.5,
				    0.0,0.8,1, 0.5,
				    0.0,0.8,1, 0.5
				   };
	
	Shape axis(6, 3, "lines");
	axis.setVertices(pos3);
	axis.setColors(col3);


	unsigned char image[] = {
	  0, 0,255,255, 	0, 255, 0,255,    0,255,255,255,
	  255,0,0,255,    255,255,255,255,    255,255,0,255   
	};

//	Shape f(4, 3, "triangles", "tex"); // (no. of points, components per point (2/3), render method);
//	f.pointSize = 8;
//	f.setVertices(pos4);	
////	f.setColors(col4);
//	f.setElements(tess_ids, 6);
//	f.applyTexture(UVs, image, 3,2);
	
	unsigned char canvaspixels[] = {
	  255,255,255,255, 	255,255,255,255, 
	  255,255,255,255,  255,255,255,255, 
	};
	Frame canvas(0,0, 100, 100, canvaspixels, 2,2);
	canvas.setLayer(-1);
	
	Frame f1(25, 25, 75, 50, image, 3,2);
////	f1.setExtent(-100,100,-100,100);
	f1.setLayer(100);
	f1.resize(50,50,75,75);
//	f1.setLayer(10);
	
	Frame f2(0,0,50,50, image, 3,2);
	f2.setLayer(1);
	f2.resize(25,25,50,50);
	
	while(1){       // infinite loop needed to poll anim_on signal.
		glutMainLoopEvent();
		usleep(20000);
	}
	// launch sim end.
	
	return 0;
}

