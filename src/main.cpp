#include <iostream>
#include <complex>
#include <chrono>

#include <PortAudio/portaudio.h>

#include "Viewport.h"

#define SAMPLE_RATE 44100
//sample rate hard-coded in Drawable::DFT
#define TABLE_SIZE 200
#define NUM_CHANNELS 2
#define FRAMES_PER_BUFFER 1024
#define GAIN 10000

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
//	glfwWindowHint(GLFW_STEREO, GLFW_TRUE);

//	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

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
	int deviceindex;
	std::cout<<"Choose which input device to use (return the corresponding interger. Ensure the device is input, not output)"<<std::endl;
	for(int x=0;x<Pa_GetDeviceCount();x++){
		std::cout<<x<<": "<<Pa_GetDeviceInfo(x)->name<<std::endl;
	}
	std::cin>>deviceindex;


	//try using functioning output device as input


	paTestData data;
	PaStream*stream;
	PaStreamParameters inputParameters;
//	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	inputParameters.device = deviceindex; /* default input device */
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



	Viewport*v=new Viewport(window);
	v->addFlat(new Drawable2D());
	v->addFlat(new Drawable2D());
	v->addFlat(new Drawable2D());
	v->addFlat(new Drawable2D());
	v->addFlat(new Drawable2D());

	v->getFlat(0)->setScale(glm::vec2(0.3889, 1));
	v->getFlat(0)->setPosition(glm::vec2(0, 100));
	v->getFlat(1)->setScale(glm::vec2(0.5, 0.5));
	v->getFlat(1)->setPosition(glm::vec2(0, 300));
	v->getFlat(3)->setScale(glm::vec2(1.0, 0.5));
//	v->getFlat(3)->setPosition(glm::vec2(0, 600));
	v->getFlat(3)->setPosition(glm::vec2(0, 600));
//	v->getFlat(4)->setScale(glm::vec2(1.0/23.8, 1.0/100000.0));
	v->getFlat(4)->setScale(glm::vec2(1.0/25.0, 1.0/100000.0));
	v->getFlat(4)->setPosition(glm::vec2(0, 200));

	v->getFlat(1)->setMode(POINTS);
	v->getFlat(3)->setMode(SPECIAL);
	v->getFlat(4)->setMode(FILL);

	glClearColor(0.1, 0.1, 0.1, 1.0);

	float t=0, tl=0, frames=0, gainMax;

	GLuint flatsShader=Viewport::loadShaders("../resources/shaders/2DVertShader.glsl", "../resources/shaders/2DFragShader.glsl");



	//temporary measure has been taken to reduce lag, by ignoring three of every four



	std::vector<glm::vec2> blank;
	for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width*8){blank.push_back(glm::vec2(x, 0.0));}
	Drawable2D fourierProfile=Drawable2D(blank);
//	fourierProfile.DFT(20.0, 20000.0, 1.0, Drawable2D(std::vector<glm::vec2>{glm::vec2(0, 0)}));
	fourierProfile.FFT(20.0, 20000.0, 1.0, Drawable2D(std::vector<glm::vec2>{glm::vec2(0, 0)}));

	int gridSize=fourierProfile.getVertices().size()*((height-(v->getFlat(3)->getPosition().y*v->getFlat(3)->getScale().y))/20);
	std::vector<glm::vec2> gridTemp=std::vector<glm::vec2>();
	std::vector<glm::vec4> gridColorTemp=std::vector<glm::vec4>();

	do{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		t=glfwGetTime();

		v->getFlat(0)->setVertices(std::vector<glm::vec2>());
		v->getFlat(4)->setVertices(std::vector<glm::vec2>());

		gainMax=100.0;
		for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width*4){
			v->getFlat(0)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN));
			v->getFlat(0)->addColor(glm::vec4(float(x)/width, 0.2, 1-float(x)/width, 1));
			v->getFlat(4)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN*2));
			if(data.recordedSamples[x]*GAIN>gainMax){gainMax=data.recordedSamples[x]*GAIN;}
		}

		if(gainMax>100.0){
			v->getFlat(0)->setScale(glm::vec2(v->getFlat(0)->getScale().x, 100.0/gainMax));
			v->getFlat(4)->setScale(glm::vec2(v->getFlat(4)->getScale().x, 100.0/gainMax));
		}

//		v->getFlat(4)->DFT(20.0, 20000.0, 1.0, fourierProfile);
		v->getFlat(4)->FFT(20.0, 20000.0, 1.0, fourierProfile);

		gridTemp=v->getFlat(3)->getVertices();
		gridColorTemp=v->getFlat(3)->getColor();
		for(auto&vert:v->getFlat(4)->getVertices()){
			gridTemp.push_back(glm::vec2(vert.x*v->getFlat(4)->getScale().x, 0));
			gridColorTemp.push_back(glm::vec4(1, 1, 1, vert.y/1500.0));
		}
		for(int x=0;x<gridTemp.size();x++){
			gridTemp.at(x).y+=20.0;
		}
//		if(v->getFlat(3)->getVertices().size()>3600){
		if(v->getFlat(3)->getVertices().size()>3012){
			gridTemp.erase(gridTemp.begin(), gridTemp.begin()+fourierProfile.getVertices().size());
			gridColorTemp.erase(gridColorTemp.begin(), gridColorTemp.begin()+fourierProfile.getVertices().size());
		}
		v->getFlat(3)->setVertices(gridTemp);
		v->getFlat(3)->setColor(gridColorTemp);

		v->drawFlats(flatsShader);

		glfwSwapBuffers(window);
		glfwPollEvents();

		tl=t;
		frames++;
		if(t!=0){std::cout<<frames/t<<" fps"<<std::endl;}

	}while(!glfwWindowShouldClose(window)&&glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS);


	glfwTerminate();

	Pa_StopStream(stream);
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