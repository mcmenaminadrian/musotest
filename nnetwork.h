#ifndef NNETWORK_H
#define NNETWORK_H

class NNetwork
{
private:
	const int logisticTableSize;
	const double logisticTableMax;
	int row_stride;
	unsigned int widthJPEG;
	unsigned int heightJPEG;
	std::vector<double> weightsH; //weights into hidden layer from input
	std::vector<double> biasHidden; //weights of bias into hidden layer
	std::vector<double> biasOutput; //weight(s) of bias into output
	std::vector<double> outHidden; //output of hidden layer
	std::vector<double> weightsO; //weights of output from hidden to output
	std::vector<double> inputs; //values of input neurons
	std::vector<uint8_t> jpegBuffer; //representation of JPEG
	std::vector<uint8_t> copiedBuffer; // copy to mark up with scan scores
	double logistic(const double& inValue) const;
	double dotProduct(const bool isRow, const int number) const;
	void calculateHiddenValues();
	void loadJPEG(const std::string& jpegFile);
	void copyJPEG(const std::string& jpegFile);
	void saveJPEG() const;
	void loadWeights();
	double calculateOutputValue() const;
	void storeScannedLine(JSAMPROW jRow);
	void storeCopiedLine(JSAMPROW jRow);
	void processInputs(const int startRow, const int startCol);

public:
	NNetwork();
	void process(const std::string& jpegFile);
};	


#endif
