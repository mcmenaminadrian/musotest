#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstring>
#include "stdint.h"
#include "jpeglib.h"
#include "nnetwork.h"


using namespace std;

//==========logistic function stuff==========

double NNetwork::logistic(const double& inValue) const
{

	double result = 1.0 / (1.0 + exp(-inValue));
	return result;
}


//==========load weights==========

void NNetwork::loadWeights()
{

	ifstream weightStream("nnetweights.txt");
	double x;
	char c;
	for (int i = 0; i < 20000; i++) {
		weightStream >> x;
		weightsH.push_back(x);
	}
	weightStream >> c;
	for (int i = 0; i < 200; i++) {
		weightStream >> x;
		biasHidden.push_back(x);
	}
	weightStream >> c;
	for (int i = 0; i < 200; i++) {
		weightStream >> x;
		weightsO.push_back(x);
	}
	weightStream >> c >> x;
	biasOutput.push_back(x);
	weightStream.close();
}

//==========calculate outputs==========

//200 hidden neurons
//one for each row in input
//one for each column in input
//20000 weights as each input neuron linked to 2 hidden neurons
//in hidden weights 0 - 99 weights for row 0
//100 - 199 weights for column 0 etc
//and for hidden neuron 0, all of row 0 connected
//and for hidden neuron 1, all of column 0 connected etc 

double NNetwork::dotProduct(const int number) const
{
	double result = 0.0;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 10; j++) {
			unsigned int node = (number / 10) * 500 +
				number % 10 * 10 + i * 100 + j;
			result += inputs.at(node) * weightsH.at(node);
		}
	}
	return result;
}

void NNetwork::calculateHiddenValues()
{
	outHidden.clear();
	for (int i = 0; i < 200; i++) {
		outHidden.push_back(logistic(dotProduct(i)));
	}
}

double NNetwork::calculateOutputValue() const
{
	double result = 0.0;
	for (unsigned int i = 0; i < outHidden.size(); i++) {
		double neuronResult = outHidden.at(i) * weightsO.at(i);
		result += neuronResult;
	}
	result += biasOutput.at(0);
	return logistic(result);
}

//==========image handling==========

void NNetwork::storeScannedLine(JSAMPROW sampledLine)
{
	for (int i = 0; i < row_stride; i++) {
		unsigned char x = *(sampledLine + i);
		jpegBuffer.push_back(x);
	}
}

void NNetwork::storeCopiedLine(JSAMPROW sampledLine)
{
	for (int i = 0; i < row_stride; i++) {
		unsigned char x = *(sampledLine + i);
		copiedBuffer.push_back(x);
	}
}

void NNetwork::loadJPEG(const string & jpegFile)
{
	//load the jpeg
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE* inFile;
	JSAMPARRAY buffer;

	cout << "Opening " << jpegFile << endl;

	if ((inFile = fopen(jpegFile.c_str(), "rb")) == NULL) {
		fprintf(stderr, "cannot open %s\n", jpegFile.c_str());
		return;
	}

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, inFile);

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_GRAYSCALE;
	jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)
		&cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		storeScannedLine(buffer[0]);
	}
	widthJPEG = cinfo.output_width;
	heightJPEG = cinfo.output_height;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(inFile);

	return;
}

void NNetwork::saveJPEG() const
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outFile;
	JSAMPROW row_pointer[1];

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	if ((outFile = fopen("testingO.jpg", "wb")) == NULL) {
		fprintf(stderr, "can't open testingO file");
		exit(1);
	}
	jpeg_stdio_dest(&cinfo, outFile);
	cinfo.image_width = widthJPEG;
	cinfo.image_height = heightJPEG;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);

	jpeg_set_quality(&cinfo, 75, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	unsigned char* x = (unsigned char *)malloc(row_stride);
	while (cinfo.next_scanline < cinfo.image_height) {
		for (int i = 0; i < row_stride; i++) {
			x[i] = copiedBuffer.at(cinfo.next_scanline * row_stride
				+ i);
		}
		row_pointer[0] = x;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(outFile);
	jpeg_destroy_compress(&cinfo);
	free(x);
}


void NNetwork::copyJPEG(const string & jpegFile)
{
	//load the jpeg
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE* inFile;
	JSAMPARRAY buffer;

	if ((inFile = fopen(jpegFile.c_str(), "rb")) == NULL) {
		fprintf(stderr, "cannot open %s\n", jpegFile.c_str());
		return;
	}

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, inFile);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)
		&cinfo, JPOOL_IMAGE, row_stride, 1);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		storeCopiedLine(buffer[0]);
	}
	widthJPEG = cinfo.output_width;
	heightJPEG = cinfo.output_height;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(inFile);

	return;
}

void NNetwork::processInputs(const int startRow, const int startCol)
{
	inputs.clear();
	for (unsigned int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			double jpegValue = jpegBuffer.at(
				(startRow + i) * widthJPEG + j + startCol);
			inputs.push_back(jpegValue/1000);
		}
	}
}

void NNetwork::process(const string& jpegFile)
{
	jpegBuffer.clear();
	loadJPEG(jpegFile);
	copyJPEG(jpegFile);
	for (unsigned i = 0; i < heightJPEG - 100; i+=25) {
		for (unsigned int j = 0; j < widthJPEG - 100; j+=25) {
			processInputs(i, j);
			calculateHiddenValues();
			//cout << "For JPEG beginning at ( " << i << "," << j;
			//cout << " ) output value is ";
			double outputValue = calculateOutputValue();
		     	//cout << outputValue << endl;
			
			for (int k = 0; k < 100; k++) {
				for (int l = 0; l < 100; l++) {
					int point = (i + k) * row_stride +
						(j + l) * 3;
					copiedBuffer.at(point) =
						255 * outputValue;
					copiedBuffer.at(point + 1) = 0;
					copiedBuffer.at(point + 2) = 0;
				}
			}
	/*		int point = i * row_stride + j * 3;
			copiedBuffer.at(point) = 255 * outputValue;
			copiedBuffer.at(point + 1) = 0;
			copiedBuffer.at(point + 2) = 0; */
		}
	}
	saveJPEG();
}

NNetwork::NNetwork():logisticTableSize(100), logisticTableMax(10.0)
{
	//test with same sequence
//	srand(100);
	//proper psuedo random sequence
	srand(time(0));
	loadWeights();
}

