#ifndef AUDIO_VISUALIZER_DRAWABLE2D_H
#define AUDIO_VISUALIZER_DRAWABLE2D_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <complex>

#include <GL/glew.h>
#include <glm/gtx/vector_angle.hpp>

#define LINES 0
#define FILL 1
#define TRIANGLES 2
#define POINTS 3
#define SPECIAL 4

class Drawable2D{
private:
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec4> color;
	glm::vec2 position, scale;
	int thickness, mode;
	float rotation;
	GLuint vertexbuffer, colorbuffer;

public:
	Drawable2D();
	Drawable2D(std::vector<glm::vec2> v);

	void draw(GLuint shader);
	std::vector<glm::vec2> wrapPolar(std::vector<glm::vec2> oldverts, int rate);
	void fourierTest(int width, int step);
	void DFT(float low, float high, float resolution, Drawable2D profile);
	void DFT2(Drawable2D profile, std::complex<float> ws[]);
	void FFT(std::vector<std::vector<std::complex<float>>> matrix, Drawable2D profile);
	std::complex<float> W(float t, float k, float omega);
	void refreshColor(){color=std::vector<glm::vec4>();}
	std::vector<glm::vec2> findPeaks();

	void addVertex(glm::vec2 v){vertices.push_back(v);}
	void eraseVertex(int index){vertices.erase(vertices.begin()+index-1);}
	void addColor(glm::vec4 c){color.push_back(c);}

	void setVertices(std::vector<glm::vec2> v){vertices=v;}
	void setColor(std::vector<glm::vec4> c){color=c;}
	void setColorAt(int index, glm::vec4 c){color.at(index)=c;}
	void eraseColor(int index){color.erase(color.begin()+index-1);}
	void setPosition(glm::vec2 p){position=p;}
	void setScale(glm::vec2 s){scale=s;}
	void setThickness(int t){thickness=t;}
	void setRotation(float r);
	void setTotalColor(glm::vec4 c){for(auto&col:color){col=c;}}
	void setMode(int m){mode=m;}

	std::vector<glm::vec2> getVertices(){return vertices;}
	glm::vec2 getPosition(){return position;}
	glm::vec2 getScale(){return scale;}
	int getThickness(){return thickness;}
	float getRotation(){return rotation;}
	std::vector<glm::vec4> getColor(){return color;}

	virtual ~Drawable2D();
};

#endif //AUDIO_VISUALIZER_DRAWABLE2D_H
