#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include "video.h"
#include "grid.h"

using namespace std;

void usage()
{
    cerr << "./gridmedian <dimensions> <video path> <output path>\n";
    cerr << "    dimensions: grid dimensions in format widthxheight (e.g. 3x3)\n";
    cerr << "    video path: path to video file\n";
    cerr << "    output path: path to output csv file\n";
}

// Parses "dimensions" cmdline argument.
int parseDimensions(const char* dimensions, int& width, int& height)
{
    stringstream dimensionsStream(dimensions);
    
    dimensionsStream >> width;
    if (!dimensionsStream)
        return EINVAL;

    if (dimensionsStream.peek() != 'x')
        return EINVAL;
    dimensionsStream.ignore();

    dimensionsStream >> height;
    if (!dimensionsStream)
        return EINVAL;
    if (!dimensionsStream.eof())
        return EINVAL;

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != 4) {
        usage();
        return EINVAL;
    }

    // Determine grid dimensions.
    int gridWidth;
    int gridHeight;
    int result = parseDimensions(argv[1], gridWidth, gridHeight);
    if (result != 0) {
        cerr << "Invalid dimensions '" << argv[1] << "'\n";
        usage();
        return result;
    }

    try {

        VideoDecoder videoDecoder(argv[2]);

        ofstream outputFile{argv[3]};
        if (!outputFile.is_open()) {
            cerr << "Failed to open output file '" << argv[3] << "': " << strerror(errno) << endl;
            return errno;
        }

        int frameIndex = 1;
        while (auto frame = videoDecoder.getNextKeyFrame()) {

            assert(frame->isKeyFrame());

#ifdef DEBUG_PRINT_FRAMES
            stringstream frameName;
            frameName << "frame_" << frameIndex;
            frameName << "_at" << setprecision(2) << frame->getTime();
            frameName << ".ppm";

            frame->print(frameName.str().c_str());
#endif

            if (frame->getWidth() < gridWidth || frame->getHeight() < gridHeight) {
                cerr << "Invalid dimensions. Given dimensions " << gridWidth << "x" << gridHeight
                     << " are bigger than frame dimensions " << frame->getWidth() << "x" << frame->getHeight() << endl;
                return EINVAL;
            }

            outputFile << std::fixed << setprecision(2) << frame->getTime();
            auto medians = cellSplitter<VideoDecoder::Frame>(*frame, gridWidth, gridHeight, cellMedian<VideoDecoder::Frame>);
            for (const auto& median : medians) {
                outputFile << ',' << median;
            }
            outputFile << endl;

            frameIndex++;
        }

    } catch (const VideoDecoderException& ex) {
        // TODO Delete output file on error.
        cerr << ex.what() << endl;
        return ex.code();
    } catch (...) {
        cerr << "Unexpected exception\n";
        return -1;
    }


    return 0;
}