#pragma once

struct TextModel
{   
    TextModel(std::string DetectorPath, std::string RecognizerPath, std::string VocabularyPath) :
        DetectorPath(DetectorPath),
        RecognizerPath(RecognizerPath),
        VocabularyPath(VocabularyPath)
    {}

    std::string DetectorPath;
    std::string RecognizerPath;
    std::string VocabularyPath;


    //Paramters for Text Detector: DB
    //DOWNLOAD TEXTDETECTION MODEL: https://drive.google.com/file/d/1vY_KsDZZZb_svd5RT6pjyI8BS1nPbBSX/view
    //GitHub: https://github.com/MhLiao/DB
    TextDetectionModel_DB detector{ DetectorPath };
    float binThresh = 0.3f; //Confidence threshold of the binary map, 0.3
    float polyThresh = 0.5; //Confidence threshold of polygons, 0.5
    uint maxCandidates = 4048; //Max candidates of polygons
    double unclipRatio = 2.0; //unclip ratio
    int width = 2560; //2560,1696
    int height = 1472; //1472,1056
    double detScale = 1.0 / 255.0;
    Size detInputSize = Size(width, height);
    Scalar detMean = Scalar(122.67891434, 116.66876762, 104.00698793);

    //Parameters for Text Recognizer: deep-text-recognition-benchmark 
    //DOWNLOAD TEXTRECOGNITION MODEL: https://drive.google.com/file/d/1JPIhUeXldbUGrTuYt_Y2_MuUyQU7bDZ2/view?usp=sharing
    //DOWNLOAD VOCABULARY: https://drive.google.com/uc?export=dowload&id=1oPOYx5rQRp8L6XQciUwmwhMCfX0KyO4b
    // GitHub: https://github.com/zihaomu/deep-text-recognition-benchmark
    TextRecognitionModel recognizer{ RecognizerPath };
    double recScale = 1.0 / 127.5;
    Scalar recMean = Scalar(127.5);
    Size recInputSize = Size(100, 32);
    int imreadRGB = IMREAD_GRAYSCALE; //imread with flags=IMREAD_GRAYSCALE; 1: imread with flags=IMREAD_COLOR

    void LoadTextDetectorNet();
    void LoadTextRecognizerNet();
};


void TextModel::LoadTextDetectorNet()
{
    //Using Paramter for Db Text Detector 
    detector.setBinaryThreshold(binThresh)
        .setPolygonThreshold(polyThresh)
        .setUnclipRatio(unclipRatio)
        .setMaxCandidates(maxCandidates);

    detector.setInputParams(detScale, detInputSize, detMean);
}

void TextModel::LoadTextRecognizerNet()
{
    // Load vocabulary
    std::ifstream vocFile;
    vocFile.open(samples::findFile(VocabularyPath));
    CV_Assert(vocFile.is_open());

    String vocLine;
    std::vector<String> vocabulary;
    while (std::getline(vocFile, vocLine)) {
        vocabulary.push_back(vocLine);
    }

    recognizer.setVocabulary(vocabulary);
    recognizer.setDecodeType("CTC-greedy");

    //Using Parameters for Text Recognizer
    recognizer.setInputParams(recScale, recInputSize, recMean, false);

    vocFile.close();
}