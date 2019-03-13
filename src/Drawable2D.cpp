#include "Drawable2D.h"

Drawable2D::Drawable2D(){
	position=glm::vec2(0, 0);
	scale=glm::vec2(1, 1);
	thickness=2;
	rotation=0.0;
	vertices.push_back(glm::vec2(0, 0));
	vertices.push_back(glm::vec2(800, 600));
	color.push_back(glm::vec4(1, 1, 1, 1));
	color.push_back(glm::vec4(1, 1, 1, 1));
	mode=LINES;
}
Drawable2D::Drawable2D(std::vector<glm::vec2> v)
	: vertices(v){
	position=glm::vec2(0, 0);
	scale=glm::vec2(1, 1);
	thickness=1;
	rotation=0.0;
	color.push_back(glm::vec4(1, 1, 1, 1));
	color.push_back(glm::vec4(1, 1, 1, 1));
	mode=LINES;
}


void Drawable2D::draw(GLuint shader){
	//FACTOR IN ROTATION
	//optimize?


//	std::cout<<"HERE";
	glUseProgram(shader);

//	GLint colorID=glGetUniformLocation(shader, "colorIn");
//	glUniform3fv(colorID, 1, &color[0]);
	std::vector<glm::vec2> tris;
	std::vector<glm::vec4> tricols;
	if(mode==LINES){
//		std::cout<<"hereline";
//		std::cout<<"verts: "<<vertices.size()<<" colors: "<<color.size()<<std::endl;
		tris.push_back(vertices.at(0));
		tricols.push_back(color.at(0));
		for(int x=1;x<vertices.size()-1;x++){
			tris.push_back(vertices.at(x));
			tricols.push_back(color.at(x));
			tris.push_back(vertices.at(x));
			tricols.push_back(color.at(x));
		}
		tris.push_back(vertices.at(vertices.size()-1));
		tricols.push_back(color.at(color.size()-1));
//		std::cout<<"herelineend";
	}
	else if(mode==FILL){
//		std::cout<<"herefill";
		for(int x=0;x<vertices.size()-1;x++){
			tris.push_back(vertices.at(x));
			tricols.push_back(color.at(x));
			tris.push_back(glm::vec2(vertices.at(x).x, 0));
			tricols.push_back(color.at(x));
			tris.push_back(vertices.at(x+1));
			tricols.push_back(color.at(x+1));

			tris.push_back(glm::vec2(vertices.at(x).x, 0));
			tricols.push_back(color.at(x));
			tris.push_back(glm::vec2(vertices.at(x+1).x, 0));
			tricols.push_back(color.at(x+1));
			tris.push_back(vertices.at(x+1));
			tricols.push_back(color.at(x+1));
		}
	}
	else if(mode==POINTS){
//		std::cout<<"herepoint";
		for(int x=0;x<vertices.size();x++){
//			std::cout<<v.x<<"   "<<v.y<<std::endl;
			tris.push_back(vertices.at(x)+glm::vec2(10, -10));
			tris.push_back(vertices.at(x)+glm::vec2(-10, -10));
			tris.push_back(vertices.at(x)+glm::vec2(0, 10));

			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x));
		}
//		std::cout<<tris.size()<<std::endl;
	}
	else if(mode==SPECIAL){
		for(int x=0;x<vertices.size()-1;x++){
			tris.push_back(vertices.at(x));
			tris.push_back(vertices.at(x+1));
			tris.push_back(vertices.at(x)+glm::vec2(0, 10));

			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x+1));
			tricols.push_back(color.at(x));

			tris.push_back(vertices.at(x+1));
			tris.push_back(vertices.at(x)+glm::vec2(0, 10));
			tris.push_back(vertices.at(x+1)+glm::vec2(0, 10));

			tricols.push_back(color.at(x+1));
			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x+1));
		}
	}
	else{tris=vertices;tricols=color;}
//	std::cout<<"hErE";


	std::vector<glm::vec2> oldverts=tris, newverts;
	glm::vec2 temp;
	for(auto&v:oldverts){
		temp=glm::vec2(atan2(v.y-position.y, v.x-position.x)+rotation, sqrt(pow(v.x-position.x, 2)+pow(v.y-position.y, 2)));
		newverts.push_back(glm::vec2(temp.y*cos(temp.x), temp.y*sin(temp.x)));
	}
	tris=newverts;

	for(auto& v:tris){v*=scale;v+=position;}

	GLuint vb;
	glGenBuffers(1, &vb);
	vertexbuffer=vb;
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, tris.size()*sizeof(glm::vec2), &tris[0], GL_STATIC_DRAW);
	GLuint cb;
	glGenBuffers(1, &cb);
	colorbuffer=cb;
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, tricols.size()*sizeof(glm::vec4), &tricols[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	if(mode==LINES){glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(tris.size()));}
	if(mode==TRIANGLES||mode==FILL||mode==POINTS||mode==SPECIAL){glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(tris.size()));}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

std::vector<glm::vec2> Drawable2D::wrapPolar(std::vector<glm::vec2> oldverts, int rate){
	std::vector<glm::vec2> newverts;
	//polar in form (theta, r)
	float theta;
	for(int x=0;x<oldverts.size();x++){
		theta=oldverts.at(x).x*6.28/rate;
//		theta=oldverts.at(x).x*6.28/rate-1.57;
		newverts.push_back(glm::vec2(oldverts.at(x).y*cos(theta), oldverts.at(x).y*sin(theta)));
	}
	return newverts;
}

void Drawable2D::fourierTest(){
	//changes here should be accounted for in findPeaks()
	std::vector<glm::vec2> oldverts=vertices, polarverts, newverts;
	float avg;
	for(int y=20;y<2000;y+=10){
		//tries frequencies from 20 Hz to 2 kHz
		//one frequency->one vertex
		avg=0.0;
		polarverts=wrapPolar(oldverts, 2.90189/(0.0000328642*y));
//		polarverts=wrapPolar(oldverts, y);
		for(int x=0;x<polarverts.size();x++){//lowering resolution has little to no effect on efficiency
			avg+=polarverts.at(x).x;
			avg+=polarverts.at(x).y;
		}
		avg/=polarverts.size();
//		avg*=sqrt(y*10)/50;
		newverts.push_back(glm::vec2(y, abs(avg)*10));
		color.push_back(glm::vec4(0, float(y)/1440, 1-float(y)/1440, 1));
//		color.push_back(glm::vec4(1, 1, 1, 1));
//		color.push_back(glm::vec4(1, 1, 1, y/1440));
//		color.push_back(glm::vec3(avg/10, avg/10, 1));
//		color.push_back(glm::vec3(1, 1, 1));
	}
//	std::cout<<"old: "<<vertices.size()<<" new: "<<newverts.size()<<std::endl;
	vertices=newverts;
	this->setScale(glm::vec2(0.4, 0.5));
	//5000->0.165
}

std::vector<glm::vec2> Drawable2D::findPeaks(){
	std::vector<glm::vec2> result;
	float max;
	//really 20.60
	for(float x=20;x<2000;x*=pow(2, 1.0/12.0)){
//		std::cout<<"   "<<x<<std::endl;
//		std::cout<<x<<std::endl;
//		max=0.0;
//		for(int y=0;y<x;y++){
////			if(vertices.at(y).y>max){max=vertices.at(y).y;}
//		}
//		std::cout<<(x-20)/5<<std::endl;
		for(float y=x;y<x*pow(2, 1.0/12.0);y++){
//			std::cout<<y<<std::endl;
			if(y<vertices.size()&&vertices.at(y).y>200.0){result.push_back(vertices.at(y)*glm::vec2(0.8, 1));}
//			if(y<vertices.size()){std::cout<<vertices.at(y).y<<std::endl;}
		}
//		if((int)(x-20)/10.0<vertices.size()){
////			std::cout<<vertices.at((int)(x-20)/10).y<<std::endl;
//			std::cout<<(int)(x-20)/10<<std::endl;
//			if((int)(x-20)/10>vertices.size()&&vertices.at((int)(x-20)/10).y>10.0){
//				result.push_back(vertices.at((int)(x-20)/10));
//				std::cout<<result.at(result.size()-1).y<<std::endl;
//			}
//		}
//		if(max>1.0){std::cout<<max<<std::endl;}
//		std::cout<<max<<std::endl;
	}
//	for(auto&v:result){
//		std::cout<<v.x<<"   "<<v.y<<std::endl;
//	}
//	std::cout<<result.size()<<std::endl;
	return result;
}

void Drawable2D::setRotation(float r){
	rotation=r;
//	std::vector<glm::vec2> oldverts=vertices, newverts;
//	//polar in form (theta, r)
//	glm::vec2 temp;
//	for(auto&v:oldverts){
//		temp=glm::vec2(atan2(v.x, v.y), sqrt(v.x*v.x+v.y*v.y));
//		newverts.push_back(glm::vec2(temp.y*cos(temp.x), temp.y*sin(temp.x)));
//	}
//	vertices=newverts;
}

Drawable2D::~Drawable2D(){
	glDeleteBuffers(1, &vertexbuffer);
}