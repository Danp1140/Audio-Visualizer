#include "Drawable2D.h"

Drawable2D::Drawable2D(){
	position=glm::vec2(0, 0);
	scale=glm::vec2(1, 1);
	thickness=2;
	rotation=0.0;
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
	//optimize?

	glUseProgram(shader);

	std::vector<glm::vec2> tris;
	std::vector<glm::vec4> tricols;
	if(mode==LINES&&vertices.size()>0){
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
	}
	else if(mode==FILL){
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
		for(int x=0;x<vertices.size();x++){
			tris.push_back(vertices.at(x)+glm::vec2(10, -10));
			tris.push_back(vertices.at(x)+glm::vec2(-10, -10));
			tris.push_back(vertices.at(x)+glm::vec2(0, 10));

			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x));
		}
	}
	else if(mode==SPECIAL&&vertices.size()>0){
		for(int x=0;x<vertices.size()-1;x++){
			tris.push_back(vertices.at(x));
			tris.push_back(vertices.at(x+1));
			tris.push_back(vertices.at(x)+glm::vec2(0, 20));

			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x+1));
			tricols.push_back(color.at(x));

			tris.push_back(vertices.at(x+1));
			tris.push_back(vertices.at(x)+glm::vec2(0, 20));
			tris.push_back(vertices.at(x+1)+glm::vec2(0, 20));

			tricols.push_back(color.at(x+1));
			tricols.push_back(color.at(x));
			tricols.push_back(color.at(x+1));
		}
	}
	else{tris=vertices;tricols=color;}


	//solve rotation system (drawable 3 seems to rely on it????)
	if(mode==SPECIAL){
		std::vector<glm::vec2> oldverts=tris, newverts;
		glm::vec2 temp;
		for(auto&v:oldverts){
			temp=glm::vec2(atan2(v.y-position.y, v.x-position.x)+rotation,
			               sqrt(pow(v.x-position.x, 2)+pow(v.y-position.y, 2)));
			newverts.push_back(glm::vec2(temp.y*cos(temp.x), temp.y*sin(temp.x)));
		}
		tris=newverts;
	}
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
	glDeleteBuffers(1, &vb);
	glDeleteBuffers(1, &cb);
}

std::vector<glm::vec2> Drawable2D::wrapPolar(std::vector<glm::vec2> oldverts, int rate){
	std::vector<glm::vec2> newverts;
	float theta;
	int x;
	for(x=0;x<oldverts.size();x++){
		theta=oldverts.at(x).x*6.28/rate;
		newverts.push_back(glm::vec2(oldverts.at(x).y*cos(theta), oldverts.at(x).y*sin(theta)));
	}
	return newverts;
}

void Drawable2D::fourierTest(int width, int step){
	//changes here should be accounted for in findPeaks()
	std::vector<glm::vec2> oldverts=vertices, polarverts, newverts;
	float avg, y;
	int x;
//	for(y=20;y<2000;y*=(pow(2, 1.0/12.0))){
	for(y=0;y<width+step;y+=step){
		//tries frequencies from 20 Hz to 2 kHz
		//one frequency->one vertex
		avg=0.0;
		polarverts=wrapPolar(oldverts, 2.90189/(0.0000328642*y));
		for(x=0;x<polarverts.size();x++){//lowering resolution has little to no effect on efficiency
			avg+=polarverts.at(x).x;
			avg+=polarverts.at(x).y;
		}
		avg/=polarverts.size();
		newverts.push_back(glm::vec2(y, abs(avg)*10*sqrt(y/200.0)));
		color.push_back(glm::vec4(0, float(y)/width, 1-float(y)/width, 1));
	}
	vertices=newverts;
}

void Drawable2D::DFT(float low, float high, float resolution, Drawable2D profile){
	//balance down bass a bit, up mids a bit
	std::vector<glm::vec2> newVertices;
	float t=1.0f/vertices.size(), n=vertices.size();
	int k=0;
	bool p=profile.getVertices().size()>1;
	std::complex<float> sum, ti=std::complex<float>(t, 0.0)*std::complex<float>(0.0, -1.0);
	for(float freq=low; freq<high; freq*=pow(2, 1.0/(12.0*resolution))){
		sum=std::complex<float>(0.0, 0.0);
		for(k=0; k<n; k++){
			sum+=std::complex<float>(vertices[k].x, vertices[k].y*10)*exp(ti*freq*(float)k);
		}
		//move if outside of for?
		//average real and imaginary?
		if(p){
			newVertices.push_back(glm::vec2((log(freq/low)/log(2))*1920, abs((sum.real()/1000+sum.imag()/1000)/2-profile.getVertices().at(newVertices.size()).y)));
//			std::cout<<newVertices[newVertices.size()-1].y<<std::endl;
//			newVertices.push_back(glm::vec2((log(freq)/log(low*pow(2.0, 1.0/(12.0*resolution))))*1920, abs((sum.real()/1000+sum.imag()/1000)/2-profile.getVertices().at(newVertices.size()).y)));
//			color.push_back(glm::vec4(0, freq/high, (high-freq)/high, 1));
			if(freq<450&&freq>350){color.push_back(glm::vec4(1, 0, 0, 1));}
			else{color.push_back(glm::vec4(1, 1, 1, 1));}

		}
		else{
			newVertices.push_back(glm::vec2((log(freq/low)/log(2))*1920, (sum.real()/1000+sum.imag()/1000)/2));
//			newVertices.push_back(glm::vec2((log(freq)/log(low*pow(2.0, 1.0/(12.0*resolution))))*1920, (sum.real()/1000+sum.imag()/1000)/2));
			color.push_back(glm::vec4(1, 1, 1, 1));
		}
	}
	vertices=newVertices;
}

//dft2 treats frequencies realistically, and possibly optimizes with array of W's with varying power

void Drawable2D::DFT2(Drawable2D profile, std::complex<float> ws[]){
	int n=sizeof(ws)/sizeof(std::complex<float>), k;
	std::complex<float> sum;
	for(int freq=0;freq<n;freq++){
		sum=std::complex<float>(0.0f, 0.0f);
		for(k=0;k<n;k++){
			sum+=std::complex<float>(vertices[k].x, vertices[k].y)*ws[freq*k];
		}
		std::cout<<sum<<std::endl;
	}
}

void Drawable2D::FFT(std::vector<std::vector<std::complex<float>>> matrix, Drawable2D profile){
	std::vector<glm::vec2> newverts;
	int n=vertices.size();
	std::complex<float> sum;
	for(int x=0;x<n;x++){
		sum=std::complex<float>(0.0f, 0.0f);
		for(int y=0;y<n;y++){
			sum+=matrix[x][y]*std::complex<float>(vertices[y].x, vertices[y].y);
		}
		newverts.emplace_back(glm::vec2(sum.real(), sum.imag()));
		color.emplace_back(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
	vertices=newverts;
}

std::complex<float> Drawable2D::W(float t, float k, float omega){
	return exp(std::complex<float>(0.0f, -t*k*omega));
}

std::vector<glm::vec2> Drawable2D::findPeaks(){
	std::vector<glm::vec2> result;
	float max;
	//really 20.60
	for(float x=20;x<2000;x*=pow(2, 1.0/12.0)){
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