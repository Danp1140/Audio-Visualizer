#ifndef AUDIO_VISUALIZER_VIEWPORT_H
#define AUDIO_VISUALIZER_VIEWPORT_H

#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Drawable2D.h"

class Viewport{
private:
	GLFWwindow*window;
	std::vector<Drawable2D*> flats;

public:
	Viewport(GLFWwindow*w);

	static GLuint loadShaders(const char* vertex_shader_filepath, const char* fragment_shader_filepath);
	void drawFlats(GLuint shader);


	void addFlat(Drawable2D*d){flats.push_back(d);}

	Drawable2D*getFlat(int index){return flats.at(index);}

	std::vector<Drawable2D*> getFlats(){return flats;}
};

#endif //AUDIO_VISUALIZER_VIEWPORT_H
