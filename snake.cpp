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
vector<pos> snake, list, listBlue;
int size = 10, iter = 0;
pos newPos, mouseP;

int width = 8, tickness = 8, shift = 0;

char key;

void updateScene(){
    frame.copyTo(game);

    if(snake.size() > 0){
        Point pt1 = Point(snake[0].x, snake[0].y);
        Point pt2 = Point(snake[0].x, snake[0].y);

        //line(game, pt1, pt2, s, width, tickness, shift);
        circle( game, pt1, 5 , Scalar(255,0,0), -1, 8);

        for(int i = 1; i<snake.size();i++){
            Point pt1 = Point(snake[i-1].x, snake[i-1].y);
            Point pt2 = Point(snake[i].x, snake[i].y);

            //circle( game, pt1, 5 , Scalar(255,0,0), -1, 8);
            line(game, pt1, pt2, s, width, tickness, shift);
        }
    }

    for(int i = 0; i < mouse.size().height; i++)
        for (int j = 0; j < mouse.size().width; j++){
            Vec3b v = mouse.at<Vec3b>(i,j);
            if(v[0] != 0 || v[1] != 0 || v[2] != 0)
                game.at<Vec3b>(i + mouseP.y,j + mouseP.x) = mouse.at<Vec3b>(i,j);
        }

    listBlue.clear();

    for(int i = 0; i < game.size().height; i++)
        for (int j = 0; j < game.size().width; j++){
            Vec3b v = game.at<Vec3b>(i,j);

            if(v[0] == 255 && v[1] == 0 && v[2] == 0){
                pos p;
                p.x = j;
                p.y = i;
                listBlue.push_back(p);
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

        }
}

void findNewPos(){
    Mat imageHsv;

    cvtColor(image, imageHsv, COLOR_BGR2HSV);

    Mat imageThresh, imageThresh1, imageThresh2;

    inRange(imageHsv, cv::Scalar(10, 190, 80), cv::Scalar(90, 255, 255), imageThresh);      //Yellow
   // inRange(imageHsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), imageThresh);   // Red
   // inRange(imageHsv, cv::Scalar(110, 50, 50), cv::Scalar(130, 255, 255), imageThresh);   //  Blue

    GaussianBlur(imageThresh, imageThresh, Size(9, 9), 2, 2);

    Moments m = moments(imageThresh, true);
    Point center;

    if(m.m00 !=0){
        center = Point(m.m10/m.m00, m.m01/m.m00);

        pos p;

        p.x = center.x;
        p.y = center.y;

        if(p.x != 0 && p.y != 0){
            if(list.size() == 10)
                list.erase(list.begin());

            list.push_back(p);

            if(snake.size() == 0)
                snake.push_back(p);
        }

        int meanX = 0, meanY = 0;

        for(int i = 0; i < list.size(); i++){
            meanX += list[i].x;
            meanY += list[i].y;
        }

        newPos.x = meanX / 10;
        newPos.y = meanY / 10;
    }
}

bool checkGame(){

    if(snake.size() > 9)
        if(newPos.x != snake.back().x || newPos.y != snake.back().y){
            printf("%d\n", (int)listBlue.size());
            for(int i = 0; i < listBlue.size(); i++){
                pos p = listBlue[i];

                if(p.x == newPos.x && p.y == newPos.y){
                    printf("X:%d Y:%d\n", newPos.x, newPos.y);
                    printf("%d %d\n", p.x, p.y);
                    return true;
                }
            }
        }

    return false;
}

void updateMouse(){
    srand(time(NULL));

    mouseP.x = rand()%500;
    mouseP.y = rand()%350;
}

void checkMouse(){
    if(newPos.x >= mouseP.x && newPos.x <= (mouseP.x + mouse.size().width) 
        && newPos.y >= mouseP.y && newPos.y <= (mouseP.y + mouse.size().height)){
        size += 4;

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

    frame.copyTo(game);

    for(;;){
        cap >> frame;

        frame.copyTo(image);

        findNewPos();

        checkMouse();

        updateSnake();

        updateScene();

        if( checkGame() )
            break;

        imshow("Snake", game);

        key = (char) waitKey(10);
        if( key == 27 ) break;

    }

    int ratos = (size-10)/4;

    printf("\n\n\n\n\nVoce comeu %d Ratos!\n", ratos);

    if(ratos < 2)
        printf("Voce e horrivel\n");
    else if(ratos < 8)
        printf("Shooooow !\n");
    else
        printf("Mitoooooooooooooooooooooooooou!!!!!!!!!!!\n");

    return 0;
}
