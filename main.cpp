#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>
#include "model.h"
#include <cstddef>         // std::size_t
#include <vector>
#include <algorithm>
#include <string>
#include <boost/filesystem.hpp>
#include "sys/types.h"
#include "sys/stat.h" 


using namespace boost::system;
namespace filesys = boost::filesystem;
#ifndef USING_BOOST
#define USING_BOOST
#endif

using namespace std;
using namespace cv;
tuple<Mat, Mat, Mat, Mat> edgesDetect(Mat, _model, int);



void mkdirTree(string sub, string dir)
{
if (sub.length() == 0)
return;

int i=0;

for (i; i < sub.length(); i++)
{
dir += sub[i];
if (sub[i] == '/')
break;
}

mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

if (i+1 < sub.length())
mkdirTree(sub.substr(i+1), dir);
}


std::vector<std::string> getAllFilesInDir(const std::string &dirPath, 	const std::vector<std::string> dirSkipList = { })
{
 
	// Create a vector of string
	std::vector<std::string> listOfFiles;
	try {
		// Check if given path exists and points to a directory
		if (filesys::exists(dirPath) && filesys::is_directory(dirPath))
		{
			// Create a Recursive Directory Iterator object and points to the starting of directory
			filesys::recursive_directory_iterator iter(dirPath);
 
			// Create a Recursive Directory Iterator object pointing to end.
			filesys::recursive_directory_iterator end;
 
			// Iterate till end
			while (iter != end)
			{
				// Check if current entry is a directory and if exists in skip list
				if (filesys::is_directory(iter->path()) &&
						(std::find(dirSkipList.begin(), dirSkipList.end(), iter->path().filename()) != dirSkipList.end()))
				{
					// Skip the iteration of current directory pointed by iterator
#ifdef USING_BOOST
					// Boost Fileystsem  API to skip current directory iteration
					iter.no_push();
#else
					// c++17 Filesystem API to skip current directory iteration
					iter.disable_recursion_pending();
#endif
				}
				else
				{
					// Add the name in vector
					listOfFiles.push_back(iter->path().string());
				}
 
				boost::system::error_code ec;
				// Increment the iterator to point to next entry in recursive iteration
				iter.increment(ec);
				if (ec) {
					std::cerr << "Error While Accessing : " << iter->path().string() << " :: " << ec.message() << '\n';
				}
			}
		}
	}
	catch (std::system_error & e)
	{
		std::cerr << "Exception :: " << e.what();
	}
	return listOfFiles;
}


string GetDirectory (const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return(path.substr(0, found));
}


_model loadmodel();

_para initial_para();
void edgebox(string, _model, _para);

tuple<int, int> siftflow(Mat image, int option, int pin);


int main() {

    
    std::string BasePath = "/home/leo/docker_ws/datasets/24tokyo";
    std::size_t found = BasePath.size();
    std::string SavePath = "/home/leo/docker_ws/datasets/24tokyo-edges";

    //std::string dirPath = "img-test";
    cout << "Please waiting for the model to load!" << endl;
    //load model and set opts
   
   
   
    _model model = loadmodel();
    model.opts.sharpen = 2; //model.opts.sharpen = 2;
    model.opts.multiscale = 0;
    model.opts.nThreads = 4;
    model.opts.nTreesEval = 1; //model.opts.nTreesEval = 4;
    model.opts.showpic = 1;//set if show picture or not
    model.opts.showtime = 10;//in s
    model.opts.stride = 2;
    model.opts.showboxnum = 15;
    _para para = initial_para();
    cout << "model loaded!" << endl;





	// Get recursive list of files in given directory and its sub directories
	std::vector<std::string> listOfFiles = getAllFilesInDir(BasePath);
 
	// Iterate over the vector and print all files
	for (auto str : listOfFiles){
		if(str.substr(str.find_last_of(".") + 1) == "jpg"){
            
            //  std::string image_path = "/home/leo/docker_ws/datasets/24tokyo-edges/03814/381489.331_3946812.467/t3SQSc3FcyVyob-w52ds-g_012_150.jpg";
            std::cout << str << std::endl;
            
            string file_dirs = GetDirectory(str);
            cout << "file_dirs = " << file_dirs <<  endl;

            // extract directroy after base path
            string file_dir = file_dirs.substr(found+1);
            cout << "file_dir " << file_dir << endl;

            // Extract file name    
            std::size_t founds = str.find_last_of("/\\");
            string file_name = str.substr(founds+1);
            cout << "file_name " << file_name << endl;    
            
            string outputDir = SavePath + "/" + file_dir + "/";
            mkdirTree(outputDir, "");

            string save_img = outputDir + file_name ;
            
            clock_t begin = clock();
            Mat I = imread(str);

            tuple<Mat, Mat, Mat, Mat> detect = edgesDetect(I, model, 4);
            Mat E, O, unuse1, unuse2;
            tie(E, O, unuse1, unuse2) = detect;
            Mat E_draw = E*255;
            cout << "time:" << ((double) clock() - begin) / CLOCKS_PER_SEC << "s" << endl;
            std::cout << save_img << std::endl; 
            imwrite(save_img, E_draw);
            

            O.release();
            
            unuse1.release();
            
            unuse2.release();
            
            E.release();

            I.release();        

                     
            E_draw.release();
            
            
        }
    }
	std::cout << "**********************" << std::endl;
 
 
    std::cout << "Done!" << std::endl;//for test
    return 0;
}

