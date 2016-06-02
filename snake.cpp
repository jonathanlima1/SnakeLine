#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>
#include <time.h>

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
vector<pos> snake, list;
int size = 1, iter = 0;
pos newPos, mouseP;

int width = 8, tickness = 8, shift = 0;

char key;

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

    for(int i = 0; i < mouse.size().height; i++)
        for (int j = 0; j < mouse.size().width; j++){
            Vec3b v = mouse.at<Vec3b>(i,j);
            if(v[0] != 0 || v[1] != 0 || v[2] != 0)
                game.at<Vec3b>(i + mouseP.x,j + mouseP.y) = mouse.at<Vec3b>(i,j);
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

void findNewPos(){
    Mat imageHsv;

    cvtColor(image, imageHsv, COLOR_BGR2HSV);

    Mat imageThresh, imageThresh1, imageThresh2;

    inRange(imageHsv, cv::Scalar(10, 190, 80), cv::Scalar(90, 255, 255), imageThresh);      //Yellow
   // inRange(imageHsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), imageThresh);   // Red
   // inRange(imageHsv, cv::Scalar(110, 50, 50), cv::Scalar(130, 255, 255), imageThresh);   //  Blue
   // inRange(imageHsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), imageThresh1);
  // inRange(imageHsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), imageThresh2);

 //  addWeighted(imageThresh1, 1.0, imageThresh2, 1.0, 0.0, imageThresh);

    GaussianBlur(imageThresh, imageThresh, Size(9, 9), 2, 2);

    vector<Vec3f> circles;
    //HoughCircles(imageThresh, circles, CV_HOUGH_GRADIENT, 1, imageThresh.rows/8, 100, 20, 0, 0);

    Moments m = moments(imageThresh, true);
    Point center;

    //if(circles.size() > 0){
        //center = Point(round(circles[0][0]), round(circles[0][1]));
    if(m.m00 !=0){
        center = Point(m.m10/m.m00, m.m01/m.m00);

        pos p1;

        p1.x = center.x;
        p1.y = center.y;

        if(list.size() == 5)
            list.erase(list.begin());

        list.push_back(p1);

        printf("%d %d %d\n", center.x, center.y, (int)snake.size());

        if(snake.size() == 0){
            pos p;

            p.x = center.x;
            p.y = center.y;

            snake.push_back(p);

            Point2f point;
            point.x = p.x;
            point.y = p.y;

        }

        int meanX = 0, meanY = 0;
        for(int i = 0; i<list.size(); i++){
            meanX += list[i].x;
            meanY += list[i].y;
        }

        newPos.x = meanX/list.size();
        newPos.y = meanY/list.size();
    }
}

bool checkGame(){

    /*Vec3b val = game.at<Vec3b>(newPos.x, newPos.y);
    
    return val[0] == 255 && val[1] == 0 && val[2] == 0;*/

    if(snake.size() > 0)
        for(int i=0; i<snake.size()-1; i++)
            if(snake[i].x == newPos.x && snake[i].y == newPos.y)
                return true;

    return false;
}

void updateMouse(){
    srand(time(NULL));

    mouseP.x = rand()%400;
    mouseP.y = rand()%500;
}

void checkMouse(){
    if(newPos.x > mouseP.x && newPos.y > mouseP.y){
        size++;

        updateMouse();
    }
}

int main(int , char**){

    // abre a câmera default
    cap.open(0);
    if(!cap.isOpened())
        return -1;

    // captura uma imagem para recuperar as
    // informacoes de gravação
    cap >> frame;

    mouse = imread("rato.png");

    namedWindow("Snake", 1);

    updateMouse();

    for(;;){
        cap >> frame;

        frame.copyTo(image);

        findNewPos();

        updateScene();

        if( checkGame() )
            break;

        checkMouse();

        updateSnake();

        updateScene();

        imshow("Snake", game);

        key = (char) waitKey(10);
        if( key == 27 ) break;

    }

    return 0;
}
