#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

#define RADIUS 20

using namespace cv;
using namespace std;

struct pos{
    int x;
    int y;
};

Scalar s = Scalar(255,0,0);

VideoCapture cap;
Mat image, mouse, game, frame;
vector<pos> snake;
int size = 1;
pos newPos;

int width = 3, tickness = 8, shift = 0;

char key;

bool needToInit = false;
bool nightMode = false;
const int MAX_COUNT = 500;
TermCriteria termcrit(TermCriteria::COUNT|TermCriteria::EPS,20,0.03);
Size subPixWinSize(10,10), winSize(31,31);
Mat gray, prevGray;
vector<Point2f> points[2];

void updateScene(){
    frame.copyTo(game);

    if(snake.size() > 0){
        Point pt1 = Point(snake[0].x, snake[0].y);
        Point pt2 = Point(snake[0].x, snake[0].y);

        line(game, pt1, pt2, s, width, tickness, shift);

        for(int i = 1; i<snake.size();i++){
            printf("ponto %d : %d %d\n", i, snake[i].x, snake[i].y);
            Point pt1 = Point(snake[i-1].x, snake[i-1].y);
            Point pt2 = Point(snake[i].x, snake[i].y);

            line(game, pt1, pt2, s, width, tickness, shift);
        }
    }
}

void updateSnake(){
    if(snake.size() > 0)
        if(newPos.x != snake.back().x || newPos.y != snake.back().y){
            if(size == snake.size())
                snake.erase(snake.begin());
            
            pos p;

            p.x = newPos.x;
            p.y = newPos.y;

            snake.push_back(p);

            Point2f point;
            point.x = p.x;
            point.y = p.y;

            circle( image, point, 3, Scalar(255,0,0), -1, 8);
        }
}

Point2f point;
bool addRemovePt = false;

static void onMouse( int event, int x, int y, int /*flags*/, void* /*param*/ )
{
    if( event == EVENT_LBUTTONDOWN )
    {
        point = Point2f((float)x, (float)y);

        pos p;

        p.x = x;
        p.y = y;

        snake.clear();

        snake.push_back(p);

        addRemovePt = true;
    }
}

void findNewPos(){
    cvtColor(image, gray, COLOR_BGR2GRAY);

    if( nightMode )
        image = Scalar::all(0);

    if( needToInit )
    {
        // automatic initialization
        goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
        cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
        addRemovePt = false;
    }
    else if( !points[0].empty() )
    {
        if( needToInit )
        {
            // automatic initialization
            goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
            cornerSubPix(gray, points[1], subPixWinSize, Size(-1,-1), termcrit);
            addRemovePt = false;
        }
        else if( !points[0].empty() )
        {
            vector<uchar> status;
            vector<float> err;
            if(prevGray.empty())
                gray.copyTo(prevGray);
            calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
                    3, termcrit, 0, 0.001);
            size_t i, k;
            for( i = k = 0; i < points[1].size(); i++ )
            {
                if( addRemovePt )
                {
                    if( norm(point - points[1][i]) <= 5 )
                    {
                        addRemovePt = false;
                        continue;
                    }
                }

                if( !status[i] )
                    continue;

                points[1][k++] = points[1][i];
                circle( image, points[1][i], 3, Scalar(0,255,0), -1, 8);
            }
            points[1].resize(k);
        }
    }

    if( addRemovePt && points[1].size() < (size_t)MAX_COUNT )
    {
        vector<Point2f> tmp;
        tmp.push_back(point);
        cornerSubPix( gray, tmp, winSize, Size(-1,-1), termcrit);
        points[1].push_back(tmp[0]);
        addRemovePt = false;
    }

    needToInit = false;

    if(points[1].size() > 0){
        newPos.x = points[1][0].x;
        newPos.y = points[1][0].y;
    }

    needToInit = false;

    std::swap(points[1], points[0]);
    cv::swap(prevGray, gray);
}

bool checkGame(){

    if(snake.size() > 0)
        for(int i=0; i<snake.size()-1; i++)
            if(snake[i].x == newPos.x && snake[i].y == newPos.y)
                return true;

    return false;
}

int main(int , char**){

    // abre a câmera default
    cap.open(0);
    if(!cap.isOpened())
        return -1;

    // captura uma imagem para recuperar as
    // informacoes de gravação
    cap >> frame;

    namedWindow("Snake", 1);
    setMouseCallback( "Snake", onMouse, 0 );

    for(;;){
        cap >> frame;

        frame.copyTo(image);

        findNewPos();

        if( checkGame() )
            break;

        updateSnake();

        updateScene();

        imshow("Snake", game);

        key = (char) waitKey(10);
        if( key == 27 ) break;

    }

    return 0;
}
