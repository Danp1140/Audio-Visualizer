#include <iostream>
#include <complex>
#include <chrono>
#include <queue>

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
	//above can be increased to 8, with fps drop of ~5
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



	int deviceindex;
	std::cout<<"Choose which input device to use (return the corresponding integer. Ensure the device is input, not output)"<<std::endl;
	for(int x=0;x<Pa_GetDeviceCount();x++){
		std::cout<<x<<": "<<Pa_GetDeviceInfo(x)->name<<std::endl;
	}
	std::cin>>deviceindex;

	paTestData data;
	PaStream*stream;
	PaStreamParameters inputParameters;
//	inputParameters.device = Pa_GetDefaultInputDevice();
	inputParameters.device = deviceindex;
	inputParameters.channelCount = 2;
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

	Pa_OpenStream(&stream, &inputParameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallbackExperimental, &data);
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
	v->getFlat(3)->setPosition(glm::vec2(0, 700));
	v->getFlat(4)->setScale(glm::vec2(1.0/20.0, 1.0/5.0));
//	v->getFlat(4)->setScale(glm::vec2(1.0/23.0, 1.0));
	v->getFlat(4)->setPosition(glm::vec2(0, 200));

	v->getFlat(1)->setMode(POINTS);
	v->getFlat(3)->setMode(SPECIAL);
	v->getFlat(4)->setMode(FILL);

	glClearColor(0.1, 0.1, 0.1, 1.0);

	float t=0, tl=0, frames=0, gainMax, low=20.0, high=2000.0, res=1.0;

	GLuint flatsShader=Viewport::loadShaders("../resources/shaders/2DVertShader.glsl", "../resources/shaders/2DFragShader.glsl");

	//temporary measure has been taken to reduce lag, by ignoring three of every four
	//i dont even know how much i'm ignoring anymore, its fine, everything's fine



	std::vector<glm::vec2> blank;
	for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width*8){blank.push_back(glm::vec2(x, 0.0));}
	std::cout<<blank.size()<<std::endl;

	int n=blank.size(), counter=0;
	std::complex<float> w=exp(std::complex<float>(6.28/(float)n, -1.0f));
	std::complex<float> ws[n];
	std::complex<float> matrixtemp[n][n];
	std::vector<std::vector<std::complex<float>>> matrix;

//	std::cout<<w<<std::endl;

	for(int x=0;x<n;x++){
		ws[x]=pow(w, x);
//		std::cout<<ws[x]<<std::endl;
	}

	for(int x=0;x<n;x++){
		std::cout<<" | ";
		counter=0;
		for(int y=0;y<n;y++){
			matrixtemp[x][y]=ws[counter];
			counter+=x;
			if(counter>=n){counter-=n;}
			std::cout<<(int)matrixtemp[x][y].real()<<"+"<<(int)matrixtemp[x][y].imag()<<"i, ";
		}
		std::cout<<" | "<<blank[x].y<<std::endl;
	}
	std::vector<std::complex<float>> loadtemp=std::vector<std::complex<float>>();
	for(int x=0;x<n;x++){
		matrix.push_back(loadtemp);
		for(int y=0;y<n;y++){
			matrix[x].push_back(matrixtemp[x][y]);
		}
	}

	Drawable2D fourierProfile=Drawable2D(blank);
//	fourierProfile.DFT(low, high, res, Drawable2D(std::vector<glm::vec2>{glm::vec2(0, 0)}));

	std::cout<<fourierProfile.getVertices().size()<<std::endl;
	int gridSize=std::ceil(2.0*fourierProfile.getVertices().size()*ceil((height-v->getFlat(3)->getPosition().y)/20.0));
	std::vector<glm::vec2> gridTemp=std::vector<glm::vec2>();
	std::vector<glm::vec4> gridColorTemp=std::vector<glm::vec4>();

//	v->getFlat(4)->setScale(glm::vec2(0.001f, 0.0001f));

	v->getFlat(3)->addVertex(glm::vec2(0, 0));
	v->getFlat(3)->addVertex(glm::vec2(0, 0));
	v->getFlat(3)->addVertex(glm::vec2(0, 0));
	v->getFlat(3)->addColor(glm::vec4(0, 0, 0, 0));
	v->getFlat(3)->addColor(glm::vec4(0, 0, 0, 0));
	v->getFlat(3)->addColor(glm::vec4(0, 0, 0, 0));

	do{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		t=glfwGetTime();

		v->getFlat(0)->setVertices(std::vector<glm::vec2>());
		v->getFlat(4)->setVertices(std::vector<glm::vec2>());

		gainMax=100.0;
		for(int x=0;x<NUM_CHANNELS*FRAMES_PER_BUFFER;x+=NUM_CHANNELS*FRAMES_PER_BUFFER/width*8){
			v->getFlat(0)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN));
			v->getFlat(0)->addColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
			v->getFlat(4)->addVertex(glm::vec2(x, data.recordedSamples[x]*GAIN*2));
			if(data.recordedSamples[x]*GAIN>gainMax){gainMax=data.recordedSamples[x]*GAIN;}
		}

		if(gainMax>100.0){
			v->getFlat(0)->setScale(glm::vec2(v->getFlat(0)->getScale().x, 100.0/gainMax));
//			v->getFlat(4)->setScale(glm::vec2(v->getFlat(4)->getScale().x, 100.0/gainMax));
		}

//		v->getFlat(4)->FFT(matrix, fourierProfile);
		v->getFlat(4)->DFT(low, high, res, fourierProfile);
//		v->getFlat(4)->DFT2(fourierProfile, ws);

//		gridTemp=v->getFlat(3)->getVertices();
//		gridColorTemp=v->getFlat(3)->getColor();
//		for(int x=0;x<v->getFlat(4)->getVertices().size();x+=2){
//			gridTemp.push_back(glm::vec2(v->getFlat(4)->getVertices().at(x).x*v->getFlat(4)->getScale().x, 0));
//			gridColorTemp.push_back(glm::vec4(1, 1, 1, v->getFlat(4)->getVertices().at(x).y/1500.0-0.05f));
//		}
//		for(int x=0;x<gridTemp.size();x++){
//			gridTemp.at(x).y+=20.0;
//		}
//		if(v->getFlat(3)->getVertices().size()>gridSize){
//			gridTemp.erase(gridTemp.begin(), gridTemp.begin()+fourierProfile.getVertices().size());
//			gridColorTemp.erase(gridColorTemp.begin(), gridColorTemp.begin()+fourierProfile.getVertices().size());
//		}
//		v->getFlat(3)->setVertices(gridTemp);
//		v->getFlat(3)->setColor(gridColorTemp);

		v->drawFlats(flatsShader);

		glfwSwapBuffers(window);
		glfwPollEvents();

		tl=t;
		frames++;
		//this isn't actually a proper fps counter (cumulative, that's why beginning is a climb)
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