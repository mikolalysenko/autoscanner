#include "image.h"

using namespace std;
using namespace boost;


//Clean up for IplImages
struct IplImageDestructor
{
    void operator()(IplImage* img)
    {
        cvReleaseImage(&img);
    }
};

//Sets image pointer
void Image::set_image(IplImage * img)
{
    //Check image format
    if(img->nChannels != 3 && img->depth != IPL_DEPTH_8U)
    {
        IplImage * tmp = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
        cvConvertImage(img, tmp);
        cvReleaseImage(&img);
        img = tmp;
    }
    
    image_ptr = shared_ptr<IplImage>(img, IplImageDestructor());
}

//Does the copy-on-write check
void Image::check_copy()
{
    if(image_ptr.unique())
        return;
    *this = dup();
}

//Duplicates an image
Image Image::dup() const
{
    return Image(cvCloneImage(image_ptr.get()));
}

//Places image under control
Image::Image(IplImage * img)
{
    set_image(img);
}

//Constructs image
Image::Image(const string& filename) : image_ptr()
{
    //Try reading image
    IplImage * img = cvLoadImage(filename.c_str());
    assert(img);
    set_image(img);
}

//Saves image data
Image::Image(int w, int h)
{
    IplImage * img = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
    assert(img);
    set_image(img);
}

//Saves an image to file
void Image::save(const string& filename) const
{
    cvSaveImage(filename.c_str(), image_ptr.get());
}

//Reads a movie file from disk and chops it into a set of pictures
vector<Image> loadMovie(const string& filename, int frameskip)
{
    //Check frameskip
    assert(frameskip >= 0);
    
    //Open capture object
    CvCapture * capture = cvCaptureFromAVI(filename.c_str());
    assert(capture);
    
    vector<Image> frames;
    
    //Scan through capture object
    while(true)
    {
        //Skip frames
        for(int i=0; i<=frameskip; i++)
        {
            if(!cvGrabFrame(capture))
            {
                cvReleaseCapture(&capture);
                return frames;
            }
        }
        
        //Retrieve frame
        IplImage *tmp = cvRetrieveFrame(capture);
        assert(tmp);
        IplImage *img = cvCloneImage(tmp);
        assert(img);
        
        //Append to frame list
        frames.push_back(Image(img));
    }
}
