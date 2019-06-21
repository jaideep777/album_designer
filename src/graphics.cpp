#include "../headers/graphics.h"

#include "../utils/simple_io.h"
#include "../utils/simple_math.h"
#include "../utils/simple_histogram.h"

#include <algorithm>
using namespace std;

Renderer * glRenderer = NULL;
int generic_count = 0;

void printError(const char *context)
{
  GLenum error = glGetError();
  if (error != GL_NO_ERROR) {
    fprintf(stderr, "%s: %s\n", context, gluErrorString(error));
  };
}

void printStatus(const char *step, GLuint context, GLuint status)
{
  GLint result = GL_FALSE;
  glGetShaderiv(context, status, &result);
  if (result == GL_FALSE) {
    char buffer[1024];
    if (status == GL_COMPILE_STATUS)
      glGetShaderInfoLog(context, 1024, NULL, buffer);
    else
      glGetProgramInfoLog(context, 1024, NULL, buffer);
    if (buffer[0])
      fprintf(stderr, "%s: %s\n", step, buffer);
  };
}

void loadShader(string filename, GLuint &shader_id, GLenum shader_type){

	ifstream fin(filename.c_str());
	string c((istreambuf_iterator<char>(fin)), istreambuf_iterator<char>());
	const char * glsl_src = c.c_str();

	shader_id = glCreateShader(shader_type);
	glShaderSource(shader_id, 1, &glsl_src, NULL);
	glCompileShader(shader_id);
	printStatus(filename.c_str(), shader_id, GL_COMPILE_STATUS);
	
}


void Shape::createShaders(){
	GLenum ErrorCheckValue = glGetError();

//	cout << "creating shaders from " << vertexShaderFile << endl; 

	loadShader(vertexShaderFile, vertexShader_id, GL_VERTEX_SHADER);	
	loadShader(fragmentShaderFile, fragmentShader_id, GL_FRAGMENT_SHADER);

	program_id = glCreateProgram();
	glAttachShader(program_id, vertexShader_id);
	glAttachShader(program_id, fragmentShader_id);
	glLinkProgram(program_id);
	printStatus("link shader", program_id, GL_LINK_STATUS);

	glDeleteShader(fragmentShader_id);
	glDeleteShader(vertexShader_id);
	
//	ErrorCheckValue = glGetError();
//	if (ErrorCheckValue != GL_NO_ERROR){
//		cout << "ERROR: Could not create the shaders: " << gluErrorString(ErrorCheckValue) << endl;
//	}

//	cout << "DONE." << endl;
}


void Shape::deleteShaders(){
	glUseProgram(0);

	glDetachShader(program_id, vertexShader_id);
	glDetachShader(program_id, fragmentShader_id);

//	glDeleteShader(fragmentShader_id);
//	glDeleteShader(vertexShader_id);

	glDeleteProgram(program_id);
}

void Shape::useProgram(){
	glUseProgram(program_id);
}

void Shape::setRenderVariable(string s, float  f){
	GLuint loc = glGetUniformLocation(program_id, s.c_str());
	glUniform1f(loc, f);
}

void Shape::setRenderVariable(string s, glm::vec2 f){
	GLuint loc = glGetUniformLocation(program_id, s.c_str());
	glUniform2f(loc, f.x, f.y);
}

void Shape::setRenderVariable(string s, glm::vec3 f){
	GLuint loc = glGetUniformLocation(program_id, s.c_str());
	glUniform3f(loc, f.x, f.y, f.z);
}

void Shape::setRenderVariable(string s, glm::vec4 f){
	GLuint loc = glGetUniformLocation(program_id, s.c_str());
	glUniform4f(loc, f.x, f.y, f.z, f.w);
}

void Shape::setShaderVariable(string s, glm::mat4 f){
	GLuint loc = glGetUniformLocation(program_id, s.c_str());
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(f));
}

// ===========================================================
// class Palette 
// ===========================================================

glm::vec4 HSVtoRGB(float h, float s, float v ){
	glm::vec4 C;
	C.a = 1;
	
	if( s == 0 ) {
		// achromatic (grey)
		C.r = C.g = C.b = v;
		return C;
	}

	h = h*360.f/60.f;			// sector 0 to 5
	int i = int(h);
	float f = h - i;			// factorial part of h
	float p = v * ( 1 - s );
	float q = v * ( 1 - s * f );
	float t = v * ( 1 - s * ( 1 - f ) );

	switch( i ) {
		case 0:
			C.r = v;
			C.g = t;
			C.b = p;
			break;
		case 1:
			C.r = q;
			C.g = v;
			C.b = p;
			break;
		case 2:
			C.r = p;
			C.g = v;
			C.b = t;
			break;
		case 3:
			C.r = p;
			C.g = q;
			C.b = v;
			break;
		case 4:
			C.r = t;
			C.g = p;
			C.b = v;
			break;
		default:		// case 5:
			C.r = v;
			C.g = p;
			C.b = q;
			break;
	}

	return C;
}

Palette::Palette(int _n){
	n = _n;
	colors.resize(n);
}

void Palette::create_rainbow(float start, float end){
	for (int i=0; i<n; ++i){
		float h = start + i*(end-start)/n; float s=1, v=1;
		colors[i] = HSVtoRGB(h,s,v);
	}
}

void Palette::create_random(float start, float end){
	for (int i=0; i<n; ++i){
		colors[i].r = start + (end-start)*rand()/RAND_MAX;
		colors[i].g = start + (end-start)*rand()/RAND_MAX;
		colors[i].b = start + (end-start)*rand()/RAND_MAX;
		colors[i].a = 1;
	}
}


void Palette::create_grayscale(float start, float end){
	for (int i=0; i<n; ++i){
		colors[i].r = start + (end-start)*float(i)/(n-1);
		colors[i].g = start + (end-start)*float(i)/(n-1);
		colors[i].b = start + (end-start)*float(i)/(n-1);
		colors[i].a = 1;
	}
}


void Palette::create_ramp(glm::vec4 start, glm::vec4 end){
	for (int i=0; i<n; ++i){
		colors[i].r = start.r + (end.r-start.r)*float(i)/(n-1);
		colors[i].g = start.g + (end.g-start.g)*float(i)/(n-1);
		colors[i].b = start.b + (end.b-start.b)*float(i)/(n-1);
		colors[i].a = 1;
	}
}


void Palette::print(){
	cout << "palette:\n";
	for (int i=0; i<n; ++i){
		glm::vec4 c = colors[i];
		cout << int(c.r*255) << "\t" << int(c.g*255) << "\t" << int(c.b*255) << "\n";
	}
	cout << '\n';
}


// WORKS for 2D and 3D
vector <float> Palette::map_values(float* v, int nval, int stride, float vmin, float vmax){
	vector <float> cols(4*nval);
	
	float min_val = v[0], max_val = v[0];
//	double mean = v[0];
	for (int i=1; i < nval; ++i){
		min_val = min(min_val, v[stride*i]);
		max_val = max(max_val, v[stride*i]);
//		mean += v[stride*i];
	}
//	mean /= nval;
	
	if (vmin != 1e20f) min_val = vmin; 
	if (vmax != 1e20f) max_val = vmax;

	cout << "minmax:" <<  min_val << " " << max_val << endl;
//	cout << "mean:" <<  mean << endl;
//	colMax = max(fabs(colMax), fabs(colMin));
//	cout << "nCol = " << nCol << ", colMax = " << colMax << ", colMin = " << colMin << '\n';
	for (int i=0; i < nval; ++i) {
		glm::vec4 c;
		int colID = (v[stride*i] - min_val)/(max_val-min_val)*(n-1);
//		cout << v[i*stride] << endl;
//		cout << "colid = " << colID << endl;
		if (colID < 0 || colID > n-1){
			cols[4*i+0] = 0;	// if color is out of range, return black
			cols[4*i+1] = 0;
			cols[4*i+2] = 0;
			cols[4*i+3] = 0;
		}
		else{
			cols[4*i+0] = colors[colID].r;	// else return color from palette
			cols[4*i+1] = colors[colID].g;
			cols[4*i+2] = colors[colID].b;
			cols[4*i+3] = colors[colID].a;
		}
	}
	return cols;
}


// ===========================================================
// class Shape
// ===========================================================

Shape::Shape(int nVert, int components_per_vertex, string _type, string shader_name, bool ren){
	
//	cout << "constructing shape..." << endl;
	dim = components_per_vertex;
	
	if (shader_name == "") shader_name = as_string(dim) + "dpt";
	
	vertexShaderFile = "src/shaders/shader_vertex_" + shader_name + ".glsl";
	fragmentShaderFile = "src/shaders/shader_fragment_" + shader_name + ".glsl";
	createShaders();

	type = _type;
	nVertices = nVert;
	model = glm::mat4(1.0f);
	pointSize = 1;
	textured = false;

	// create vertex buffer
	glGenBuffers(1, &vbo);					// create buffer ids and store in array
	glBindBuffer(GL_ARRAY_BUFFER, vbo); 	// Bring buffer into current openGL context
	glBufferData(GL_ARRAY_BUFFER, dim*sizeof(float)*nVertices, NULL, GL_DYNAMIC_DRAW); 

	// create color buffer
	glGenBuffers(1, &cbo);
	glBindBuffer(GL_ARRAY_BUFFER, cbo); 	// Bring buffer into current openGL context
	glBufferData(GL_ARRAY_BUFFER, 4*sizeof(float)*nVertices, NULL, GL_DYNAMIC_DRAW); 
	
	glGenBuffers(1, &ebo);
	glGenBuffers(1, &tbo);
	
	glGenTextures(1, &tex);

	glRenderer->addShape(this);
	
	b_render = ren;
}


Shape::~Shape(){
	
	deleteShaders();
	
//	cout << "destroy " << objName << endl;
//	if (textured){
//	glBindTexture(GL_TEXTURE_2D, tex);
	glDeleteTextures(1, &tex);
//	}
	
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ebo);

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &cbo);

//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &tbo);
	
	glRenderer->removeShape(this);
	
}

void Shape::setVertices(void* data){

	glBindBuffer(GL_ARRAY_BUFFER, vbo); 	// Bring 1st buffer into current openGL context
	glBufferSubData(GL_ARRAY_BUFFER, 0, dim*sizeof(float)*nVertices, data); 
	// remove buffers from curent context. (appropriate buffers will be set bu CUDA resources)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
}


void Shape::setColors(float *colData){
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(float)*nVertices, colData); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Shape::setElements(int * elements, int n){
	// create color buffer
	nElements = n;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); 	// Bring buffer into current openGL context
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*n, elements, GL_DYNAMIC_DRAW); 
	//   ^ Not using glBufferSubData here, assuming elements will be set only once.
}

void Shape::applyTexture(float* uvs, unsigned char* pixels, int width, int height){
	textured = true;

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, nVertices*2*sizeof(float), uvs, GL_DYNAMIC_DRAW);
	//   ^ Not using glBufferSubData here, assuming UVs will be set only once.

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels); // After reading one row of texels, pointer advances to next 4 byte boundary. Therefore ALWAYS use 4byte colour types. 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

}

void Shape::render(){

	if (!b_render) return;
	
	useProgram();
	
	// set the point size to match physical scale
	if (type == "points" ) setRenderVariable("psize", pointSize);
	if (dim == 3) setShaderVariable("model", glRenderer->projection*glRenderer->view*model);
	if (dim == 2) setShaderVariable("model", model);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(glGetAttribLocation(program_id, "in_pos"), dim, GL_FLOAT, GL_FALSE, 0, 0);
//							^ position of variable in shader         ^components, type        ^ stride and offset     
	glBindBuffer(GL_ARRAY_BUFFER, cbo);
	glVertexAttribPointer(glGetAttribLocation(program_id, "in_col"), 4, GL_FLOAT, GL_FALSE, 0, 0);

	if (textured){
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glVertexAttribPointer(glGetAttribLocation(program_id, "in_UV"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindTexture(GL_TEXTURE_2D, tex);
	}
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	if (textured)  
	glEnableVertexAttribArray(2);


	if (type == "triangles") 	glDrawElements(GL_TRIANGLES, nElements, GL_UNSIGNED_INT, (void *)0);
	else if (type == "lines")  	glDrawArrays(GL_LINES, 0, nVertices);
	else if (type == "points") 	glDrawArrays(GL_POINTS, 0, nVertices);
	else 	  					glDrawArrays(GL_POINTS, 0, nVertices);
	
}



vector <float> calcExtent(float* data, int nVertices, int dim){
	glm::dvec3 centroid(0.0, 0.0, 0.0); // use double because large accummulation is expected
	glm::vec3 max(-1e20f, -1e20f, -1e20f);
	glm::vec3 min(1e20f, 1e20f, 1e20f);

	for (int i=0; i<nVertices; ++i){
		centroid += glm::dvec3(data[dim*i], data[dim*i+1], data[dim*i+2]);

		min.x = fmin(min.x, data[dim*i]);
		max.x = fmax(max.x, data[dim*i]);

		min.y = fmin(min.y, data[dim*i+1]);
		max.y = fmax(max.y, data[dim*i+1]);

		min.z = fmin(min.z, data[dim*i+2]);
		max.z = fmax(max.z, data[dim*i+2]);
	}
	centroid /= nVertices;
	float dz = max.z - min.z;
	float dy = max.y - min.y;
	float dx = max.x - min.x;
	float scale = 2/fmax(fmax(dx, dy), dz)*50;

	cout << "centroid: " << centroid.x << " " << centroid.y << " " << centroid.z << endl;
	cout << "scale: " << scale << endl;
	
	vector <float> temp(4*3); // centroid, min, max, scale
	temp[0] = centroid.x;
	temp[1] = centroid.y;
	temp[2] = centroid.z;
	
	temp[3] = min.x;
	temp[4] = min.y;
	temp[5] = min.z;
	
	temp[6] = max.x;
	temp[7] = max.y;
	temp[8] = max.z;
	
	temp[9] = scale;
	temp[10] = scale;
	temp[11] = scale;
	
	return temp; 
}

void Shape::autoExtent(float* data){

	glm::dvec3 centroid(0.0, 0.0, 0.0); // use double because large accummulation is expected
	glm::vec3 max(-1e20f, -1e20f, -1e20f);
	glm::vec3 min(1e20f, 1e20f, 1e20f);

	for (int i=0; i<nVertices; ++i){
		centroid += glm::dvec3(data[dim*i], data[dim*i+1], data[dim*i+2]);

		min.x = fmin(min.x, data[dim*i]);
		max.x = fmax(max.x, data[dim*i]);

		min.y = fmin(min.y, data[dim*i+1]);
		max.y = fmax(max.y, data[dim*i+1]);

		min.z = fmin(min.z, data[dim*i+2]);
		max.z = fmax(max.z, data[dim*i+2]);
	}
	centroid /= nVertices;
	float dz = max.z - min.z;
	float dy = max.y - min.y;
	float dx = max.x - min.x;
	float scale = 2/fmax(fmax(dx, dy), dz)*50;

	cout << "centroid: " << centroid.x << " " << centroid.y << " " << centroid.z << endl;
	cout << "scale: " << scale << endl;

	model = glm::mat4(1.f);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	model = glm::translate(model, -glm::vec3(float(centroid.x), float(centroid.y), float(centroid.z)));
}

void Shape::setExtent(vector <float>& ex){
	model = glm::mat4(1.f);
	model = glm::scale(model, glm::vec3(ex[9], ex[10], ex[11]));
	model = glm::translate(model, -glm::vec3(float(ex[0]), float(ex[1]), float(ex[2])));
}

void Shape::move(float x0ndc, float y0ndc, float xndc, float yndc){
	// must be implemented in the plane parpendicular to camera axis	
}

Shape2D::Shape2D(int nVert, string _type, string shader_name, bool ren) : Shape(nVert, 2, _type, shader_name, ren) {}

void Shape2D::setExtent(float xmin, float xmax, float ymin, float ymax){
	model = glm::ortho(xmin, xmax, ymin, ymax, 0.f, 100.f);
}


Frame::Frame(float _x0, float _y0, float _x1, float _y1, unsigned char* image, int width, int height)
	: Shape(4,3,"triangles", "tex"){

	x0 = _x0; y0 = _y0; x1 = _x1; y1 = _y1;
	layer = 0;
	
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(x0, y0, 0.f));
	model = glm::scale(model, glm::vec3(x1-x0, y1-y0, 1.f));

//	glm::vec4 a = model*glm::vec4(1.f,1.f,0.f,1.f);
//	cout << "Frame Vec:" << a.x << " " << a.y << " " << a.z << " " << a.w << endl;
	
	float verts[] = {
		1, 1, 0,
		0, 1, 0,
		0, 0, 0,
		1, 0, 0
	};	
	
	int tess_ids[] = {0,1,2,2,3,0};

//	float UVs[] = {	// unmirrored UVs
//	   1.0f, 1.0f,
//	   0.0f, 1.0f,
//	   0.0f, 0.0f,
//	   1.0f, 0.0f
//	};
	float UVs[] = {	// mirrorred along y
	   1.0f, 0.0f,
	   0.0f, 0.0f,
	   0.0f, 1.0f,
	   1.0f, 1.0f
	};
	setVertices(verts);	
	setElements(tess_ids, 6);
	applyTexture(UVs, image, width, height);

}

void Frame::setExtent(float xmin, float xmax, float ymin, float ymax){
	model = glm::ortho(xmin, xmax, ymin, ymax, 0.f, 100.f);
}

//void Frame::setPosition(float x0, float y0){
//	model = glm::translate(model, ...);
//}

void Frame::setLayer(int l){
//	float verts[] = {
//		x1, y1, 0.1f*l,
//		x0, y1, 0.1f*l,
//		x0, y0, 0.1f*l,
//		x1, y0, 0.1f*l
//	};
//	setVertices(verts);	
	model = glm::translate(model, glm::vec3(0.f, 0.f, 0.1f*(l-layer)));
	layer = l;
//	glm::vec4 a = model*glm::vec4(1.f,1.f,0.f,1.f);
//	cout << "Frame Vec:" << a.x << " " << a.y << " " << a.z << " " << a.w << endl;

}

void Frame::setSize(float _x0, float _y0, float _x1, float _y1){
	x0 = _x0; y0 = _y0; x1 = _x1; y1 = _y1;
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(x0, y0, 0.f));
	model = glm::scale(model, glm::vec3(x1-x0, y1-y0, 1.f));
	model = glm::translate(model, glm::vec3(0.f, 0.f, 0.1f*layer));

//	glm::vec4 a = model*glm::vec4(1.f,1.f,0.f,1.f);
//	cout << "Resized Frame Vec:" << a.x << " " << a.y << " " << a.z << " " << a.w << endl;
}


float Frame::containsPixel(int x, int y){
	float winw = glutGet(GLUT_WINDOW_WIDTH);
	float winh = glutGet(GLUT_WINDOW_HEIGHT);
	float xndc = 2*x/winw-1;
	float yndc = 1-2*y/winh;
//				cout << "xyndc = " << xndc << " " << yndc << endl;

	glm::vec4 p = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(xndc, yndc, 0.f, 1.f);
//				cout << "world xy = " << p.x << " " << p.y << endl;	
	
	if (p.x > x0 && p.x < x1 && p.y > y0 && p.y < y1) {
		return layer;
	}
	else return -1e20;
}

int Frame::cursorLocation(int x, int y){
	float winw = glutGet(GLUT_WINDOW_WIDTH);
	float winh = glutGet(GLUT_WINDOW_HEIGHT);
	float xndc = 2*x/winw-1;
	float yndc = 1-2*y/winh;
//				cout << "xyndc = " << xndc << " " << yndc << endl;

	glm::vec4 p = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(xndc, yndc, 0.f, 1.f);
//				cout << "world xy = " << p.x << " " << p.y << endl;	

	float d = 0.5;
	float ar = (x1-x0)/(y1-y0);
	if (p.x > (x0+d) && p.x < (x1-d) && p.y > (y0+d*ar) && p.y < (y1-d*ar)) return 1;
	else if (p.x < (x0-d) || p.x > (x1+d) || p.y < (y0-d*ar) || p.y > (y1+d*ar)) return 0;
	else if (p.y > (y0-d*ar) && p.y < (y0+d*ar)) return 21;
	else if (p.x > (x0-d) && p.x < (x0+d)) return 22;
	else if (p.y > (y1-d*ar) && p.y < (y1+d*ar)) return 23;
	else if (p.x > (x1-d) && p.x < (x1+d)) return 24;
	else return 0;
	

}


void Frame::changeCursor(int x, int y){
	
	int cloc = cursorLocation(x,y);
	if (cloc == 1) glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	else if (cloc == 0) glutSetCursor(GLUT_CURSOR_INHERIT);
	else if (cloc == 21 || cloc == 23) glutSetCursor(GLUT_CURSOR_UP_DOWN);
	else if (cloc == 22 || cloc == 24) glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
	else glutSetCursor(GLUT_CURSOR_INHERIT);
}

void Frame::move(float xi, float yi, float xf, float yf){
//	glm::vec3 p = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(xndc, yndc, 0.f, 1.f);
//	glm::vec3 p0 = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(x0ndc, y0ndc, 0.f, 1.f);
//				cout << "world xy = " << p0.x << " " << p0.y << " --> " 
//					 << p.x << " " << p.y  << endl;	

	glm::vec3 dp = glm::vec3(xf,yf,0)-glm::vec3(xi,yi,0);
	model = glm::translate(model, glm::vec3(dp.x/(x1-x0), dp.y/(y1-y0), 0));
	x0 += dp.x; x1 += dp.x;
	y0 += dp.y; y1 += dp.y;
}


void Frame::resize(float xi, float yi, float xf, float yf){
	model = glm::scale(model, glm::vec3((xf-x0)/(xi-x0), (yf-y0)/(yi-y0), 1.f));
	x1 += xf-xi; y1+= yf-yi;
}

// ===========================================================
// class Renderer
// ===========================================================

void Renderer::init(){

	b_renderText = true;
	b_renderLabels = true;
	b_renderRs = true;
	b_renderConsole = false;
	b_renderGrid = false;
	b_renderAxes = false;
	b_renderColorMap = true;

	up_axis = 010;

	window_width = 2048;
	window_height = 1024;
	viewport_aspect_ratio = 2;
	
//	view = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, -100.0f) );
	camera_tx = camera_ty = camera_rx = camera_ry = 0;
	camera_s = 1;

	view = glm::lookAt(glm::vec3(0.0f, 0.0f, 100.0f), 
					   glm::vec3(0.0f, 0.0f, 0.0f), 
					   glm::vec3(0.0f, 1.0f, 0.0f));

 	//projection = glm::perspective(glm::radians(90.0f), float(window_width) / window_height, 0.1f, 1000.0f);
	projection = glm::ortho(-10.0f, 110.0f, -10.0f, 110.0f, -10.f, 110.0f);

	glm::vec4 a = projection*view*glm::vec4(1,0,0,1);
	cout << "Vec:" << a.x << " " << a.y << " " << a.z << " " << a.w << endl;

	int t = 50; //I.getScalar("dispInterval");
	if (t < 0) {
		nSkip = -t;
		updateMode = Step;
		displayInterval = 50;	// in this case, only title will be updated in timer function (at 20 fps)
	}
	else {
		displayInterval = t;
		nSkip = -1;
		updateMode = Time;
	}

	quality = 3; //I.getScalar("graphicsQual");

	// create colour palettes
	int n = 100;
	//palette = createPalette_rainbow(n, 0, 0.75);
	//palette_rand = createPalette_random(n);

}


int Renderer::getDisplayInterval(){
	return displayInterval;
}

int Renderer::addShape(Shape* shp){
	shapes_vec.push_back(shp);
}

int Renderer::removeShape(Shape* shp){
	shapes_vec.erase(find(shapes_vec.begin(), shapes_vec.end(), shp));
}

void Renderer::togglePause(){
	b_paused = !b_paused;
}

void Renderer::toggleConsole(){
	command = "";
	b_renderConsole = !b_renderConsole;
}

void Renderer::toggleText(){
	b_renderText = !b_renderText;
}

void Renderer::toggleGrid(){
	b_renderGrid = !b_renderGrid;
}

void Renderer::toggleAxes(){
	b_renderAxes = !b_renderAxes;
}

void Renderer::receiveConsoleChar(char key){
	switch (key){
		case 27:	// esc
			b_renderConsole = false;
			cout << "Command-line turned off.\n";
		break;
				
		case 13:	// enter
			executeCommand();
		break;
		
		case 8:		// backspace
			if (command.size() != 0) command = command.substr(0, command.size()-1);
		break;
		
		default:
			command += key;
			//cout << "command = " << command << '\n';
		break;
	}
}


int Renderer::executeCommand(){
	vector <string> args = parse(command);
	
	if (args[0] == "exit"){
		b_renderConsole = false;
		cout << "Command-line turned off.\n";
	}

	if (args[0] == "set"){
		if (args[1] == "background" || args[1] == "bg" ){
			if (args.size() >= 5){
				float r = as_float(args[2]), g = as_float(args[3]), b = as_float(args[4]); //, a = as_float(args[5]);
				glClearColor(r,g,b,1);
			}
		}
	}

	else{}
	
	command = "";

}


string Renderer::makeTitle(){
    const unsigned char * ver = glGetString(GL_VERSION);
    stringstream sout; 
    sout << fixed << setprecision(1) //<< psys-> N << " Particles" //<< "GL" << ver 
    								 << ", kcps = " << 100 //psys->kernelCounter.fps
    								 << ", dcps = " << frameCounter.fps;
    								// << ", s = " << psys->igen << "." << psys->istep;
	return sout.str();
}



Shape * Renderer::pick(int x, int y){
	
	Shape * pickedShape = NULL;
	float zmax = -1e20;
	cout << "Contains Pixel: ";
	for (int i=0; i<glRenderer->shapes_vec.size(); ++i){
		float z = shapes_vec[i]->containsPixel(x,y);
		cout << z << " ";
		if (z != -1e20 && z > zmax && z > 0) pickedShape = shapes_vec[i];
		zmax = z;
	}
	cout << endl;
	return pickedShape;
}



// =================================================================================
//
//			OpenGL functions and callbacks
//				Init				
//				Display
//				Timer
//				reshape
//				keypress
//				
//			These functions assume that
//				glRenderer points to a valid renderer R
//				psys points to a valid Particle System P
//				P is connect()-ed to R
//
// =================================================================================


// function to initialize opengl, as well as the default renderer
bool init_hyperGL(int *argc, char **argv){
	cout << "init GL" << endl;
	
	glRenderer = new Renderer;
	glRenderer->init();

	// init
	glutInit(argc, argv);

//	glutInitContextVersion(4, 0);
//	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
//	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitWindowSize(glRenderer->window_width, glRenderer->window_height);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Hello World!");

	glewExperimental = GL_TRUE;
	glewInit();

	// Callbacks
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPress);
	glutSpecialFunc(specialKeyPress);
	glutMouseFunc(mousePress);
	glutMotionFunc(mouseMove);
	glutPassiveMotionFunc(mouseHover);
//	glutIdleFunc(NULL);	// start animation immediately. Otherwise init with NULL	
	//glutTimerFunc(glRenderer->getDisplayInterval(), timerEvent, 0);
//	glutCloseFunc(cleanup);
	
    // default initialization
    glClearColor(0.5, 0.5, 0.5, 0.0);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_DEPTH_TEST);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glLineWidth(2);

	glGenVertexArrays(1, &glRenderer->vao_id);
	glBindVertexArray(glRenderer->vao_id);

//	glViewport(0, 0, glRenderer->window_width, glRenderer->window_height);

    return true;
}

void cleanup_hyperGL(){
	delete glRenderer;
}



// ===================== DISPLAY FUNCTION ====================================//

void display(){
	
	//cout << "render..." << endl;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	render all shapes in list
	for (int i=0; i<glRenderer->shapes_vec.size(); ++i){
		Shape * s = glRenderer->shapes_vec[i];
		s->render();
	}

	glRenderer->frameCounter.increment();	// calculate display rate

//	glutPostRedisplay();
	glutSwapBuffers();


}

// ============================ CALLBACKS ====================================//

//void timerEvent(int value){
//	
////	glRenderer->psys->animate();

//    glutSetWindowTitle(glRenderer->makeTitle().c_str());

//	if (glRenderer->updateMode == Time) glutPostRedisplay();
//	glutTimerFunc(glRenderer->getDisplayInterval(), timerEvent, 0);
//}


void reshape(int w, int h){
	
	int w0 = glRenderer->window_width;
	int h0 = glRenderer->window_height;
	float a = glRenderer->viewport_aspect_ratio;

	int w1 = min(w, int(h*a));
	int h1 = min(h, int(w1/a));
	w1 = h1*a;
		
	int x = min(w,h); 	
    // viewport
    glViewport(fabs(w-w1)/2, fabs(h-h1)/2, w1, h1);
//	glRenderer->tailLen = glRenderer->tailLen_def * glRenderer->xmax*float(glRenderer->window_height)/float(x);	
}


void specialKeyPress(int key, int x, int y){
	if (key == GLUT_KEY_UP){	// up arrow
		++generic_count;
		cout << "counter: " << generic_count << endl;
	}
	else if (key == GLUT_KEY_DOWN){	// down arrow
		--generic_count;
		cout << "counter: " << generic_count << endl;
	}
}

void keyPress(unsigned char key, int x, int y){
	if (!glRenderer->b_renderConsole){		
		
		if (key == 32){
			glRenderer->togglePause();
		}
		
		else if (key == 27){
			cout << "\n\n~~~ Simulation ABORTED! ~~~\n\n";
			exit(0);
		}	

		else if (key == 'x'){
			glRenderer->toggleConsole();
			cout << "Command-line turned on.\n";
		}
		else{
		}

	}
	else{	// console is on. keys will be sent to command buffer.
		glRenderer->receiveConsoleChar(key);
	}
		
	glutPostRedisplay();

}

Shape * selectionBox = NULL;
Shape * selectedShape = NULL;

bool lMousePressed, rMousePressed, mMousePressed;
float mouse_x0=0, mouse_y0=0;
string mousetransform = "";

void mousePress(int button, int state, int x, int y){
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN){
				lMousePressed = 1;
				mouse_x0 = x;
				mouse_y0 = y;
				
				if (selectionBox != NULL){
					delete selectionBox;
					selectionBox = NULL;
				}

				// When shape is selected, scale cursor can outside the shape, and cause it to get deselected upon clicking. Therefore, select new shape only if current selection is null or pointer is outside of currently selected shape
				if (selectedShape == NULL  || selectedShape->cursorLocation(x,y) == 0)
					selectedShape = glRenderer->pick(x, y);
					
//				cout << "xy = " << x << " " << y << endl;
				// FIXME implement bounding box in Shape itself. update bbox in setVertices. For other computations, apply model matrix to bbox
				if (selectedShape != NULL){
					float x0=((Frame*)selectedShape)->x0, 
						  y0=((Frame*)selectedShape)->y0, 
						  x1=((Frame*)selectedShape)->x1,
						  y1=((Frame*)selectedShape)->y1;
//					cout << "selected shape bounds: " << x0 << " " << y0 << " " << x1 << " " << y1 << endl;
					float pos3[] = {x0,y0,100, x1,y0,100, x1,y0,100, x1,y1,100, x1,y1,100, x0,y1,100, x0,y1,100, x0,y0,100};
					float rr=0,gg=0.3,bb=0.3,aa=1;
					float col3[] = {rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa,
									rr,gg,bb, aa
								   };
					
					selectionBox = new Shape(8, 3, "lines");
					selectionBox->setVertices(pos3);
					selectionBox->setColors(col3);
					
					selectedShape->changeCursor(x,y);
					
					int a = selectedShape->cursorLocation(x,y);
					if (a == 0) mousetransform = "";
					else if (a == 1) mousetransform = "t";
					else if (a > 20) mousetransform = "s";
					
				}
			}
			else{
				lMousePressed = 0;
				mousetransform = "";
//				if (selectedShape != NULL) selectedShape->changeCursor(x,y);

//				if (selectionBox != NULL){
//					delete selectionBox;
//					selectionBox = NULL;
//				}
			} 
		break;

		case GLUT_MIDDLE_BUTTON:
			if (state == GLUT_DOWN){
				mMousePressed = 1;
				mouse_x0 = x;
				mouse_y0 = y;
			}
			else{
				mMousePressed = 0;
			} 
		break;

		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN){
				rMousePressed = 1;
				mouse_x0 = x;
				mouse_y0 = y;
			}
			else{
				rMousePressed = 0;
			} 
			break;

		default:
		break;
	}
	glutPostRedisplay();

}

void mouseHover(int x, int y){
	if (selectedShape != NULL){
		selectedShape->changeCursor(x,y);
	}
}

void mouseMove(int x, int y){
	float winh = glutGet(GLUT_WINDOW_HEIGHT);
	float winw = glutGet(GLUT_WINDOW_WIDTH);
	
//	cout << "transform: " << mousetransform << endl;
	
	int cursorLoc = 0;
	if (selectedShape != NULL){
		cursorLoc = selectedShape->cursorLocation(x,y);
		selectedShape->changeCursor(x,y);
	}
	
	if (lMousePressed == 1){
		// get initial and final mouse position in world coordinates
		float xndc = 2*x/winw-1;
		float yndc = 1-2*y/winh;

		float x0ndc = 2*mouse_x0/winw-1;
		float y0ndc = 1-2*mouse_y0/winh;
	
		glm::vec3 p = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(xndc, yndc, 0.f, 1.f);
		glm::vec3 p0 = glm::inverse(glRenderer->projection * glRenderer->view)*glm::vec4(x0ndc, y0ndc, 0.f, 1.f);
		
		// if any shape is selected, move the shape and the selection box
		if (selectedShape != NULL ){
			glm::vec3 dp = p-p0;

			if (mousetransform == "t") selectedShape->move(p0.x, p0.y, p.x, p.y);
			else if (mousetransform == "s") selectedShape->resize(p0.x, p0.y, p.x, p.y);
			selectionBox->model = glm::translate(selectionBox->model, dp);
		}
//		glRenderer->camera_rx += 0.2*(y - mouse_y0);
//		glRenderer->camera_ry += 0.2*(x - mouse_x0);
	}
	if (rMousePressed == 1){
//		float r = (y - mouse_y0)/h;
//		glRenderer->camera_s *= 1+r;
	}
	if (mMousePressed == 1){
//		glRenderer->camera_ty -= (h/2.5)*(y - mouse_y0)/h;	// -= because y is measured from top
//		glRenderer->camera_tx += (w/2.5)*(x - mouse_x0)/w;
	}
	mouse_y0 = y;
	mouse_x0 = x;
	
	glutPostRedisplay();

}



