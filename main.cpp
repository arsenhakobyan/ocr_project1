/*
 * textdetection.cpp
 *
 * A demo program of the Extremal Region Filter algorithm described in
 * Neumann L., Matas J.: Real-Time Scene Text Localization and Recognition, CVPR 2012
 *
 * Created on: Sep 23, 2013
 *     Author: Lluis Gomez i Bigorda <lgomez AT cvc.uab.es>
 */

#include  "opencv2/text.hpp"
#include  "opencv2/highgui.hpp"
#include  "opencv2/imgproc.hpp"
#include  "res/json.hpp"

#include <vector>
#include <set>
#include <list>
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <fstream>
#include <algorithm>


using namespace std;
using namespace cv;
using namespace cv::text;
using json = nlohmann::json;

void er_show(vector<Mat> &channels, vector<vector<ERStat> > &regions);

// helper functions

void show_help_and_exit(const char *cmd)
{
    cout << "    Usage: " << cmd << " <input_image> " << endl;
    exit(-1);
}


using namespace std;
static void system_exec(const std::string &cmd, const std::string &message)
{
    assert(! cmd.empty());
    assert(! message.empty());
    std::string command = cmd + "> /dev/null";
    if (std::system(command.c_str())) {
        // TODO exception instead of exiting
        std::cerr << message << std::endl;
        std::exit(1);
    }
}

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

set<string> allText;
json jsonText = json::array();
json res = json::object();

void textRecognition(Mat &src, vector<Rect> &groups) 
{
    for (auto g : groups) {
        const string name = to_string(g.x) + "_" +  to_string(g.y) + ".jpg"; // TODO Change this to the temprorary image.
        Mat gray;
        int p = 4;
        int x = g.x - p > 0 ? g.x - p : g.x;
        int y = g.y - p > 0 ? g.y - p : g.y;
        int w = (g.x + g.width + 2 * p < src.cols) ? g.width + 2 * p : src.cols - g.x - 1;
        int h = (g.y + g.height + 2 * p < src.rows) ? g.height + 2 * p : src.rows - g.y - 1;

        Rect newRect(x, y, w, h);
        cvtColor(src(newRect).clone(), gray, COLOR_BGR2GRAY);
        imwrite(name, gray);
        if (gray.cols < 400) {
            system_exec("convert " + name + " -resize 800 -sharpen 0x3 " + name, "Error in the conversion phase.");
        }
        system_exec("tesseract " + name + " output -psm 7", "Error in the text recognition phase.");
        ifstream file("output.txt");
        if (! file.is_open()) {
            std::cerr << "Could not open synonyms.json file." << std::endl;
            std::exit(1);
        }
        std::stringstream text;
        text << file.rdbuf();
        string str = text.str();
        str = ::trim(str);
        cout << str <<  endl;
        if (!str.empty()) {
            allText.insert(str);
        }
    }
    for (const auto& s : allText) {
        jsonText.push_back(s);
        res["text"] = jsonText;
        ofstream resultsFile("output.json");
        resultsFile << res.dump(4);
    }
}

vector<Rect> mergeIntersections(Mat src, vector<Rect> groups) {
    // Filtering the intersection as well.
    list<Rect> initialGroupOfBoxes(groups.begin(), groups.end());
    vector<Rect> filteredBoxes;
    list<Rect>::iterator Abegin = initialGroupOfBoxes.begin();
    list<Rect>::iterator Aend = initialGroupOfBoxes.end();
    bool isIntersected = false;
    while (Abegin != Aend) {
        list<Rect>::iterator Bbegin = initialGroupOfBoxes.begin();
        list<Rect>::iterator Bend = initialGroupOfBoxes.end();
        while (Bbegin != Bend) {
            if (*Abegin == *Bbegin) { 
                ++Bbegin;
                continue; 
            }
            int minOfTwoBoxAreas = std::min(Abegin->area(), Bbegin->area());
            if (((*Abegin & *Bbegin).area()) > (minOfTwoBoxAreas * 0.9)) {
                int x = std::min(Abegin->x, Bbegin->x);
                int y = std::min(Abegin->y, Bbegin->y);
                int w = std::max(Abegin->x + Abegin->width, Bbegin->x + Bbegin->width);
                int h = std::max(Abegin->y + Abegin->height, Bbegin->y + Bbegin->height);
                filteredBoxes.push_back(Rect(x, y, w - x, h - y));
                Bbegin = initialGroupOfBoxes.erase(Bbegin);
                isIntersected = true;

                break;
            }
            ++Bbegin;
        }
        if (isIntersected) {
            Abegin = initialGroupOfBoxes.erase(Abegin);
            isIntersected = false;
            continue;
        }
        ++Abegin;
    }
    return filteredBoxes;
}


vector<Rect> removeVerySmallBoxes(Mat src, vector<Rect>* groups) {
    // Erase useless small sections.
    vector<Rect> filteredBoxes;
    const unsigned int imageArea = src.rows * src.cols;
    const unsigned int minWordArea = imageArea * 0.003;
    for (const auto& g : *groups) {
        int selfArea = g.area();
        if ( selfArea > minWordArea) {
            filteredBoxes.push_back(g);
        }
    }

    return filteredBoxes;
}

vector<Rect> runFilters(Mat src, vector<Rect>* groups) {
    vector<Rect> filteredBoxes = removeVerySmallBoxes(src, groups);
    filteredBoxes = mergeIntersections(src, *groups);
    filteredBoxes = mergeIntersections(src, filteredBoxes);
    return filteredBoxes;
}

void groups_draw(Mat &src, vector<Rect> &groups, bool isFiltered = false) {
    //for (int i=(int)groups.size()-1; i>=0; i--)
    //{
    //    const string name = to_string(groups.at(i).x) + "_" +  to_string(groups.at(i).y) + ".jpg";
    //    imwrite(name, src(groups.at(i)).clone());
    //}
    Scalar color(0, 0, 255);
    if (isFiltered) {
        color = Scalar( 255, 0, 0);
    }

    for (const auto& g : groups) {
        if (src.type() == CV_8UC3) {
            rectangle(src,g.tl(), g.br(), color, 1, 8 );
        } else {
            rectangle(src,g.tl(),g.br(),Scalar( 255 ), 1, 8 );
        }
    }
}

void er_show(vector<Mat> &channels, vector<vector<ERStat> > &regions)
{
    for (int c=0; c<(int)channels.size(); c++)
    {
        Mat dst = Mat::zeros(channels[0].rows+2,channels[0].cols+2,CV_8UC1);
        for (int r=0; r<(int)regions[c].size(); r++)
        {
            ERStat er = regions[c][r];
            if (er.parent != NULL) // deprecate the root region
            {
                int newMaskVal = 255;
                int flags = 4 + (newMaskVal << 8) + FLOODFILL_FIXED_RANGE + FLOODFILL_MASK_ONLY;
                floodFill(channels[c],dst,Point(er.pixel%channels[c].cols,er.pixel/channels[c].cols),
                          Scalar(255),0,Scalar(er.level),Scalar(0),flags);
            }
        }
        char buff[10]; char *buff_ptr = buff;
        sprintf(buff, "channel %d", c);
        imshow(buff_ptr, dst);
    }
    waitKey(-1);
}


//3 0.00001 0.8 0.2 0.1 0.3 0.3

int i1 = 3; // thresholdDelta   – Threshold step in subsequent thresholds when extracting the component tree
float f2 = 0.00001f; // minArea  – The minimum area (% of image size) allowed for retreived ER’s
float f3 = 0.8f; // maxArea – The maximum area (% of image size) allowed for retreived ER’s
float f4 = 0.2f; // minProbability  – The minimum probability P(er|character) allowed for retreived ER’s
float f5 = 0.1f; // minProbabilityDiff  – The minimum probability difference between local maxima and local minima ERs

float f6 = 0.3f; // minProbability  : The minimum probability P(er|character) allowed for retreived ER's

float f7 = 0.3f; // minProbability  : The minimum probability P(er|character) allowed for retreived ER's

void ERFilterRun(const string& inputImageName) {
    Mat src = imread(inputImageName);
    // Extract channels to be processed individually
    vector<Mat> channels;
    computeNMChannels(src, channels);

    int cn = (int)channels.size();
    // Append negative channels to detect ER- (bright regions over dark background)
    for (int c = 0; c < cn-1; c++) {
        channels.push_back(255-channels[c]);
    }

    // Create ERFilter objects with the 1st and 2nd stage default classifiers
    //Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1("./res/trained_classifierNM1.xml"), 1, 0.0001f, 0.1f, 0.6f, true, 0.1f);
    cout << "Accuracy 1 = " << f4 << endl;
    cout << "Accuracy 2 = " << f6 << endl;
    cout << "Accuracy 3 = " << f7 << endl;
    Ptr<ERFilter> er_filter1 = createERFilterNM1(loadClassifierNM1("./res/trained_classifierNM1.xml"), i1, f2, f3, f4, true, f5);
    Ptr<ERFilter> er_filter2 = createERFilterNM2(loadClassifierNM2("./res/trained_classifierNM2.xml"), f6);

    vector<vector<ERStat> > regions(channels.size());
    // Apply the default cascade classifier to each independent channel (could be done in parallel)
    cout << "Extracting Class Specific Extremal Regions from " << (int)channels.size() << " channels ..." << endl;
    cout << "    (...) this may take a while (...)" << endl << endl;
    for (int c=0; c<(int)channels.size(); c++) {
        er_filter1->run(channels[c], regions[c]);
        er_filter2->run(channels[c], regions[c]);
    }

    // Detect character groups
    cout << "Grouping extracted ERs ... ";
    vector< vector<Vec2i> > region_groups;
    vector<Rect> groups_boxes;
    //erGrouping(src, channels, regions, region_groups, groups_boxes, ERGROUPING_ORIENTATION_HORIZ);
    erGrouping(src, channels, regions, region_groups, groups_boxes, ERGROUPING_ORIENTATION_ANY, "./res/trained_classifier_erGrouping.xml", f7);


    cout << "Box count before filtering : " << groups_boxes.size() << endl;
    vector<Rect> filteredBoxes = runFilters(src, &groups_boxes);

    // draw groups
    cout << "Box count after filtering  : " << filteredBoxes.size() << endl;
    textRecognition(src, filteredBoxes);
    //Mat nsrc = src.clone();
    //groups_draw(src, groups_boxes);
    //imshow("grouping",src);
    //groups_draw(nsrc, filteredBoxes, true);
    //imshow("filtered",nsrc);
    //waitKey(0);

    // memory clean-up
    er_filter1.release();
    er_filter2.release();
    regions.clear();
    if (!groups_boxes.empty()) {
        groups_boxes.clear();
    }

}

int main(int argc, const char * argv[])
{
    if (argc < 2) {
        cout << "    Usage: " << argv[0] << " <input_image> " << endl;
        exit(-1);
    }
    if (argc == 9) {
        i1 = stoi(argv[2]);
        f2 = stof(argv[3]);
        f3 = stof(argv[4]);
        f4 = stof(argv[5]);
        f5 = stof(argv[6]);
        f6 = stof(argv[7]);
        f7 = stof(argv[8]);
    }
    string inputImageName = argv[1];
    system_exec("convert " + inputImageName + " -background white -flatten " + inputImageName, "Error in the conversion phase.");
    ERFilterRun(inputImageName);
}

