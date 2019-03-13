#include <iostream>
#include <complex>

#include <PortAudio/portaudio.h>

#include "Viewport.h"

#define SAMPLE_RATE 44100
#define TABLE_SIZE 200
#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 1024
#define GAIN 1000

//#define PA_SAMPLE_TYPE  paFloat32
//typedef float SAMPLE;
//#define SAMPLE_SILENCE  (0.0f)
//#define PRINTF_S_FORMAT "%.8f"

#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
	float left_phase;
	float right_phase;
	float tone;
	float volume;
	SAMPLE*recordedSamples;
	int frameIndex;
	int maxFrameIndex;
	std::vector<glm::vec2> trying;
}paTestData;

//static paTestData test;

int paCallback(const void *inputBuffer,
               void *outputBuffer,
               unsigned long framesPerBuffer,
               const PaStreamCallbackTimeInfo* timeInfo,
               PaStreamCallbackFlags statusFlags,
               void *userData);
int recordCallback(const void *inputBuffer,
                   void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData);
int recordCallbackExperimental(const void *inputBuffer,
                   void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData);
int main(){
	if(!glfwInit()){
		std::cout<<"GLFW Failure ("<<stderr<<")\n";
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	const GLFWvidmode*mode=glfwGetVideoMode(glfwGetPrimaryMonitor());
	GLFWwindow*window=glfwCreateWindow(1440, 900, "Audio Visualizer", nullptr,
	                                   nullptr);
	if(window==nullptr){
		std::cout<<"Window Failure ("<<stderr<<")\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if(glewInit()!=GLEW_OK){
		std::cout<<"GLEW Failure ("<<stderr<<")\n";
		return -1;
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	width/=2; height/=2;
	PaError error=Pa_Initialize();
	if(error!=paNoError){std::cout<<"PortAudio Initialization Error: \n"<<Pa_GetErrorText(error)<<std::endl;}
	std::cout<<"OpenGL version: "<<glGetString(GL_VERSION)<<"\n";
	std::cout<<"GLSL version: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<"\n";
	std::cout<<"GLFW version: "<<glfwGetVersionString()<<std::endl;
	std::cout<<Pa_GetVersionInfo()->versionText<<'\n';
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
//	_________________________________________________________________
	paTestData data;
	PaStream*stream;
	PaStreamParameters inputParameters;
	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	inputParameters.channelCount = 2;                    /* stereo input */
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = nullptr;

	data.left_phase=0.0; data.right_phase=0.0;
	data.tone=0.01;
	data.volume=1.00;

	data.maxFrameIndex=FRAMES_PER_BUFFER;
	data.frameIndex=0;
//	data.recordedSamples = (SAMPLE*) malloc(sizeof(SAMPLE)*NUM_CHANNELS*SAMPLE_RATE*5);
	data.recordedSamples=(SAMPLE*) malloc(sizeof(SAMPLE)*NUM_CHANNELS*FRAMES_PER_BUFFER);

//	Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, SAMPLE_RATE, 1024, paCallback, &data);
	Pa_OpenStream(&stream, &inputParameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallbackExperimental, &data);
//	Pa_OpenStream(&stream, &inputParameters, nullptr, SAMPLE_RATE, 1024, paClipOff, nullptr, nullptr);
	Pa_StartStream(stream);


	//rainbow as wide as ***FRAMES_PER_BUFFER*** vertices
	std::vector<glm::vec3> rainbow(FRAMES_PER_BUFFER);
	for(int x=0;x<FRAMES_PER_BUFFER/3;x++){
		rainbow.at(x).r=((float)FRAMES_PER_BUFFER-(float)x)/(float)FRAMES_PER_BUFFER;
	}for(int x=FRAMES_PER_BUFFER/3;x<2*FRAMES_PER_BUFFER/3;x++){
		rainbow.at(x).g=((float)FRAMES_PER_BUFFER-(float)x)/(float)FRAMES_PER_BUFFER;
	}for(int x=2*FRAMES_PER_BUFFER/3;x<FRAMES_PER_BUFFER;x++){
		rainbow.at(x).b=((float)FRAMES_PER_BUFFER-(float)x)/(float)FRAMES_PER_BUFFER;
	}

	std::vector<glm::vec2> temp;
//	for(int x=0;x<500;x++){r
//		temp.push_back(glm::vec2(x, (x*x)/100));
//	}
	temp.push_back(glm::vec2(0, 0));
//	temp.push_back(glm::vec2(0, 0));
	Viewport*v=new Viewport(window);
	v->addFlat(new Drawable2D(temp));
	v->getFlat(0)->addColor(glm::vec4(1, 1, 1, 1));
	v->addFlat(new Drawable2D(temp));
	v->getFlat(1)->addColor(glm::vec4(1, 1, 1, 1));
	v->addFlat(new Drawable2D(temp));
	v->getFlat(2)->addColor(glm::vec4(1, 1, 1, 1));
	v->addFlat(new Drawable2D(temp));
	v->getFlat(3)->addColor(glm::vec4(1, 1, 1, 1));

	v->getFlat(0)->setScale(glm::vec2(0.3889, 0.25));
	v->getFlat(0)->setPosition(glm::vec2(0, 100));
	v->getFlat(1)->setScale(glm::vec2(0.5, 0.5));
	v->getFlat(1)->setPosition(glm::vec2(0, 300));
	v->getFlat(2)->setScale(glm::vec2(0.5, 0.5));
	v->getFlat(2)->setPosition(glm::vec2(0, 300));
	v->getFlat(3)->setScale(glm::vec2(0.5, 0.5));
	v->getFlat(3)->setPosition(glm::vec2(0, 600));

	v->getFlat(2)->setMode(FILL);
	v->getFlat(1)->setMode(POINTS);
	v->getFlat(3)->setMode(SPECIAL);
//	v->getFlat(1)->addVertex(glm::vec2(400, 300));
//	v->getFlat(1)->setMode(LINES);
//	v->getFlat(1)->setMode(TRIANGLES);
//	v->getFlat(1)->addVertex(glm::vec2(175, 601));
//	v->getFlat(1)->addVertex(glm::vec2(175, -1));
//	v->getFlat(1)->addVertex(glm::vec2(350, -1));
//	v->getFlat(1)->addVertex(glm::vec2(350, 601));
//	v->getFlat(1)->addVertex(glm::vec2(88, 601));
//	v->getFlat(1)->addVertex(glm::vec2(88, -1));
//	v->getFlat(1)->addVertex(glm::vec2(220, -1));
//	v->getFlat(1)->addVertex(glm::vec2(220, 601));

//	v->getFlat(1)->setVertices(std::vector<glm::vec2>());

//	for(int x=0;x<1600;x+=5){
//		v->getFlat(1)->addVertex(glm::vec2(x, pow(x/150, 2)+25));
//	}
	float strobetime=2.0;
	glm::vec3 strobecolor=glm::vec3(0, 0, 1);

	std::vector<glm::vec2> guidelines={
			//~20 Hz
			glm::vec2(10, 601),
			glm::vec2(10, -1),
			//~200 Hz
			glm::vec2(100, -1),
			glm::vec2(100, 601),
			//~800 Hz
			glm::vec2(400, 601),
			glm::vec2(400, -1)

	};
//	v->getFlat(1)->setVertices(guidelines);


	glClearColor(0.1, 0.1, 0.1, 1.0);
//	v->getFlat(0)->setRotation(1.57);
//	glClearColor(1.0, 1.0, 1.0, 1.0);

//	for(int x=0;x<100;x++){
////		std::complex<float> temp(x, )
//		std::cout<<exp(std::complex<float>(0, 1))<<std::endl;
//	}

//	for(auto&f:data.trying){std::cout<<f<<std::endl;}
//	v->getFlat(0)->findPeaks();

	float t=0, tl=0, frames=0;
	float min=0.0, max=0.0, strobetemp=0.0;
	GLuint flatsShader=Viewport::loadShaders("../resources/shaders/2DVertShader.glsl", "../resources/shaders/2DFragShader.glsl");
//	v->getFlat(0)->setScale(glm::vec2(width/SAMPLE_RATE, 1));

	for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width){
		v->getFlat(0)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN/*+280*/));
//				if(v->getFlat(0)->getVertices().at(x).y>max){max=v->getFlat(0)->getVertices().at(x).y;}
//				v->getFlat(0)->addColor(glm::vec3(0, 0, data.recordedSamples[x]*10000));
//				v->getFlat(0)->addColor(glm::vec3(1, 1, 1));

		v->getFlat(2)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN));
		if(x<width){v->getFlat(3)->addVertex(glm::vec2(x, v->getFlat(3)->getPosition().y));}
//				v->getFlat(2)->addColor(glm::vec3(1, 0, 0));
	}
	v->getFlat(0)->setScale(glm::vec2((float)width/(float)v->getFlat(0)->getVertices().size(), 0.25));
	v->getFlat(2)->fourierTest();
	v->getFlat(1)->setVertices(v->getFlat(2)->findPeaks());


	//SET COLORS BEFORE MAIN LOOP no maybe
	std::vector<glm::vec4> colorzero, colortwo;
	v->getFlat(3)->setVertices(std::vector<glm::vec2>(v->getFlat(2)->getVertices().size()));

	v->getFlat(0)->setColor(std::vector<glm::vec4>(v->getFlat(0)->getVertices().size()));
	v->getFlat(1)->setColor(std::vector<glm::vec4>(v->getFlat(1)->getVertices().size()));
	v->getFlat(2)->setColor(std::vector<glm::vec4>(v->getFlat(2)->getVertices().size()));
	v->getFlat(3)->setColor(std::vector<glm::vec4>(v->getFlat(2)->getVertices().size()));

//	for(int x=0;x<v->getFlat(0)->getColor().size();x++){
////		colorzero.push_back(glm::vec4(1, 0.3, 0, v->getFlat(0)->getVertices().at(x).x/width));
//		colorzero.push_back(glm::vec4(1, 1, 1, 1));
//	}
//	for(int x=0;x<v->getFlat(2)->getColor().size();x++){
////		colortwo.push_back(glm::vec4(0, 0.5, 0.1, v->getFlat(2)->getVertices().at(x).x/width));
//		colortwo.push_back(glm::vec4(1, 1, 1, 1));
//	}
//	v->getFlat(0)->setColor(colorzero);
//	v->getFlat(2)->setColor(colortwo);


//  TIME EACH LARGE OPERATION
//  TEST PITCH CHANGE LATENCY
//  ADJUST BASS IN FOURIER???

	std::vector<glm::vec4> activerow;


	do{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		t=glfwGetTime();
		max=0.0;
//		v->getFlat(3)->getVertices().at(0).y=100;


//		if(glfwGetKey(window, GLFW_KEY_UP)==GLFW_PRESS){v->getFlat(0)->setPosition(v->getFlat(0)->getPosition()+glm::vec2(0, 10));}
//		if(glfwGetKey(window, GLFW_KEY_DOWN)==GLFW_PRESS){v->getFlat(0)->setPosition(v->getFlat(0)->getPosition()+glm::vec2(0, -10));}
//		if(glfwGetKey(window, GLFW_KEY_RIGHT)==GLFW_PRESS){v->getFlat(0)->setPosition(v->getFlat(0)->getPosition()+glm::vec2(10, 0));}
//		if(glfwGetKey(window, GLFW_KEY_LEFT)==GLFW_PRESS){v->getFlat(0)->setPosition(v->getFlat(0)->getPosition()+glm::vec2(-10, 0));}


//		v->getFlat(0)->setColor(std::vector<glm::vec4>(v->getFlat(0)->getVertices().size()));
//		v->getFlat(1)->setColor(std::vector<glm::vec4>(v->getFlat(1)->getVertices().size()));
//		v->getFlat(2)->setColor(std::vector<glm::vec4>(v->getFlat(2)->getVertices().size()));

		//add graph that is linear source, alpha scaled by each vert fourier height compared to instantaneous max of fourier


			//graphs as many samples as there are ***FRAMES_PER_BUFFER***

//		for(int x=0;x<v->getFlat(3)->getVertices().size();x++){
//			v->getFlat(3)->getVertices().at(x).y+=1.0;
//			std::cout<<v->getFlat(3)->getVertices().at(x).y<<std::endl;
//		}
		v->getFlat(2)->refreshColor();
		for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width){
			v->getFlat(0)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN/*+280*/));
			v->getFlat(0)->addColor(glm::vec4(float(x)/width, 0.2, 1-float(x)/width, 1));
//			if(x<v->getFlat(3)->getVertices().size()){std::cout<<v->getFlat(3)->getVertices().at(x).y<<std::endl;}
//				v->getFlat(0)->addColor(glm::vec4(1, 0, 1, 1));
//				if(v->getFlat(0)->getVertices().at(x).y>max){max=v->getFlat(0)->getVertices().at(x).y;}
//				v->getFlat(0)->addColor(glm::vec3(0, 0, data.recordedSamples[x]*10000));
//				v->getFlat(0)->addColor(glm::vec3(1, 1, 1));

			v->getFlat(2)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN));
//				v->getFlat(2)->addColor(glm::vec4(1, 1, 1, 1));
//				v->getFlat(2)->addColor(glm::vec3(1, 0, 0));
		}

		v->getFlat(0)->setScale(glm::vec2((float)width/(float)v->getFlat(0)->getVertices().size(), 0.25));
		v->getFlat(2)->fourierTest();
		for(auto&vert:v->getFlat(2)->getVertices()){
			v->getFlat(3)->addVertex(glm::vec2(vert.x, 0));
			//color/intensity falloff???
			//check alignment/column width of fourier transform
			v->getFlat(3)->addColor(glm::vec4(1, 1, 1, vert.y/400));
//			v->getFlat(3)->addColor(glm::vec4(1, 1, 1, 1));
		}

//		v->getFlat(2)->setColor(std::vector<glm::vec4>(v->getFlat(2)->getVertices().size()));
		v->getFlat(1)->setVertices(v->getFlat(2)->findPeaks());
		v->getFlat(1)->setColor(std::vector<glm::vec4>(v->getFlat(1)->getVertices().size()));

//		v->getFlat(0)->setColor(std::vector<glm::vec4>(v->getFlat(0)->getVertices().size()));


		//efficiency is shit
//		std::vector<glm::vec4> colorzero, colortwo;
//		for(int x=0;x<v->getFlat(0)->getColor().size();x++){
//			colorzero.push_back(glm::vec4(1, 0.3, 0, v->getFlat(0)->getVertices().at(x).x/width));
//		}
//		for(int x=0;x<v->getFlat(2)->getColor().size();x++){
//			colortwo.push_back(glm::vec4(0, 0.5, 0.1, v->getFlat(2)->getVertices().at(x).x/width));
//		}
//		v->getFlat(0)->setColor(colorzero);
//		v->getFlat(2)->setColor(colortwo);

		std::cout<<"Pre-draw check: \n\t0) vertices: "<<v->getFlat(0)->getVertices().size()<<" colors: "<<v->getFlat(0)->getColor().size()
		<<"\n\t1) vertices: "<<v->getFlat(1)->getVertices().size()<<" colors: "<<v->getFlat(1)->getColor().size()
		<<"\n\t2) vertices: "<<v->getFlat(2)->getVertices().size()<<" colors: "<<v->getFlat(2)->getColor().size()
		<<"\n\t3) vertices: "<<v->getFlat(3)->getVertices().size()<<" colors: "<<v->getFlat(3)->getColor().size()<<std::endl;

//
//		v->getFlat(3)->setPosition(v->getFlat(3)->getPosition()+glm::vec2(0, 10));

//		for(auto&vert:v->getFlat(3)->getVertices()){
////			std::cout<<vert.y<<std::endl;
////			vert.y+=1.0;
//			std::cout<<vert.y<<std::endl;
//		}

		//EXTREMELY INEFFICIENT

		std::vector<glm::vec2> temp=v->getFlat(3)->getVertices();
		std::vector<glm::vec4> tempclr=v->getFlat(3)->getColor();
		for(int x=0;x<temp.size();x++){
			if(temp.at(x).y<height/*v->getFlat(3)->getPosition().y*/){temp.at(x).y+=10.0;/*tempclr.at(x)=glm::vec4(1, 1, 1, 1);*/}
			else{
				temp.erase(temp.begin()+x-1);
				tempclr.erase(tempclr.begin()+x-1);
			}
			std::cout<<"("<<temp.at(x).x<<", "<<temp.at(x).y<<")"<<std::endl;
		}
		v->getFlat(3)->setVertices(temp);
		v->getFlat(3)->setColor(tempclr);


//		std::cout<<v->getFlat(3)->getVertices().at(10).y<<std::endl;
//		for(int x=v->getFlat(3)->getVertices().size()-1;x>-1;x--){
//			v->getFlat(3)->getVertices().at(x).y+=1.0;
//			std::cout<<v->getFlat(3)->getVertices().at(x).y<<std::endl;
//		}

//		std::cout<<"here";
		v->drawFlats(flatsShader);
//		std::cout<<"HERE";


		glfwSwapBuffers(window);
		glfwPollEvents();
		tl=t;

//		std::cout<<v->getFlat(1)->getVertices().size()<<std::endl;

//		for(int x=0;x<v->getFlat(3)->getVertices().size();x++){
//			v->getFlat(3)->getVertices().at(x).y+=100.0;
//			std::cout<<v->getFlat(3)->getVertices().at(x).y<<std::endl;
//		}




		v->getFlat(0)->setVertices(std::vector<glm::vec2>(1440));
		v->getFlat(1)->setVertices(std::vector<glm::vec2>(1440));
		v->getFlat(2)->setVertices(std::vector<glm::vec2>(1440));
//		v->getFlat(3)->setVertices(std::vector<glm::vec2>(1440));

//		std::cout<<v->getFlat(2)->getColor().size()<<std::endl;

		v->getFlat(0)->setColor(std::vector<glm::vec4>(v->getFlat(0)->getVertices().size()));
		v->getFlat(1)->setColor(std::vector<glm::vec4>(v->getFlat(1)->getVertices().size()));
		v->getFlat(2)->setColor(std::vector<glm::vec4>(v->getFlat(2)->getVertices().size()));
//		v->getFlat(3)->setColor(std::vector<glm::vec4>(v->getFlat(3)->getVertices().size()));
		frames++;
//		if(t!=0){std::cout<<frames/t<<std::endl;}

	}while(!glfwWindowShouldClose(window)&&glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS);


	glfwTerminate();

	Pa_StopStream(stream);
//	for(int x=0;x<sizeof(SAMPLE)*NUM_CHANNELS*SAMPLE_RATE*5;x++){
//		SAMPLE asdf=data.recordedSamples[x];
//		std::cout<<"index: "<<x<<" sample: "<<asdf<<std::endl;
//	}
	Pa_CloseStream(stream);
	Pa_Terminate();

	return 0;
}

int paCallback(const void *inputBuffer,
                         void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData){

	paTestData *data = (paTestData*)userData;
	float *out = (float*)outputBuffer;
	(void) inputBuffer;

	for(unsigned int i=0;i<framesPerBuffer;i++){
		*out++ = data->left_phase;  /* left */
		*out++ = data->right_phase;  /* right */
//		/* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
//		data->left_phase += 0.01f;
//		/* When signal reaches top, drop back down. */
//		if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
//		/* higher pitch so we can distinguish left and right. */
//		data->right_phase += 0.10f;
//		if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
//		data->left_phase=sin(data->time);
		if(data->left_phase>=1.0*data->volume){data->left_phase-=2.0*data->volume;}
		data->left_phase+=data->tone*data->volume;
		if(data->right_phase>=1.0*data->volume){data->right_phase-=2.0*data->volume;}
		data->right_phase+=data->tone*data->volume;

	}
//	paTestData *data = (paTestData*)userData;
//	float *out = (float*)outputBuffer;
//	unsigned long i;
//
//	(void) timeInfo; /* Prevent unused variable warnings. */
//	(void) statusFlags;
//	(void) inputBuffer;
//
//	for( i=0; i<framesPerBuffer; i++ )
//	{
//		*out++ = data->sine[data->left_phase];  /* left */
//		*out++ = data->sine[data->right_phase];  /* right */
//		if(data->left_phase==1.0){data->left_phase=-1.0;}
//		else{data->left_phase=1.0;}
//		if(data->right_phase==-1.0){data->right_phase=1.0;}
//		else{data->right_phase=-1.0;}
//	}
//
//	return paContinue;

	return 0;
}

int recordCallback(const void *inputBuffer,
                    void *outputBuffer,
                    unsigned long framesPerBuffer,
                    const PaStreamCallbackTimeInfo* timeInfo,
                    PaStreamCallbackFlags statusFlags,
                    void *userData){
	paTestData *data = (paTestData*)userData;
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void) outputBuffer; /* Prevent unused variable warnings. */
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;

	if( framesLeft < framesPerBuffer )
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if( inputBuffer == NULL )
	{
		for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = 0.0;  /* left */
			if( NUM_CHANNELS == 2 ) *wptr++ = 0.0;  /* right */
//			std::cout<<0.0<<std::endl;
//			data->trying.push_back(glm::vec2(0.0, 1));
		}
	}
	else
	{
		for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = *rptr++;  /* left */
			if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
//			std::cout<<*rptr<<std::endl;
//			data->trying.push_back(glm::vec2(*rptr, 1));
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}
int recordCallbackExperimental(const void *inputBuffer,
                   void *outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo,
                   PaStreamCallbackFlags statusFlags,
                   void *userData){
	paTestData *data = (paTestData*)userData;
	data->frameIndex=0;
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
	SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void) outputBuffer; /* Prevent unused variable warnings. */
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;

	if( framesLeft < framesPerBuffer )
	{
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else
	{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if( inputBuffer == nullptr )
	{
		for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = 0.0;  /* left */
			if( NUM_CHANNELS == 2 ) *wptr++ = 0.0;  /* right */
//			std::cout<<0.0<<std::endl;
//			data->trying.push_back(glm::vec2(0.0, 1));
		}
	}
	else
	{
		for( i=0; i<framesToCalc; i++ )
		{
			*wptr++ = *rptr++;  /* left */
			if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
//			std::cout<<*rptr<<std::endl;
//			data->trying.push_back(glm::vec2(*rptr, 1));
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}