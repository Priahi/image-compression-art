
#include "stats.h"

// initialize the private vectors so that, for each color,  entry
// (x,y) is the cumulative sum of the the color values from (0,0)
// to (x,y). Similarly, the sumSq vectors are the cumulative
// sum of squares from (0,0) to (x,y).
stats::stats(PNG & im){
    int w = (int) im.width();
    int h = (int) im.height();
    this->sumRed     = vector< vector< long >> (w, vector <long> (h));
    this->sumGreen   = vector< vector< long >> (w, vector <long> (h));
    this->sumBlue    = vector< vector< long >> (w, vector <long> (h));
    this->sumsqRed   = vector< vector< long >> (w, vector <long> (h));
    this->sumsqGreen = vector< vector< long >> (w, vector <long> (h));
    this->sumsqBlue  = vector< vector< long >> (w, vector <long> (h));

    for (int x = 0; x < w; x++){
        for (int y = 0; y < h; y++){
            long r = (long)(im.getPixel(x,y)->r);
            long g = (long)(im.getPixel(x,y)->g);
            long b = (long)(im.getPixel(x,y)->b);

            this->sumRed    [x][y] = r;
            this->sumGreen  [x][y] = g;
            this->sumBlue   [x][y] = b;
            this->sumsqRed  [x][y] = r*r;
            this->sumsqGreen[x][y] = g*g;
            this->sumsqBlue [x][y] = b*b;

            if (x == 0 && y != 0) {
                statHelper(0, y, 0, y - 1, 1);
            } else if (x != 0 && y == 0) {
                statHelper(x, 0, x - 1, 0, 1);
            } else if (x !=0 && y != 0){
                statHelper(x, y, x, y - 1, 1);
                statHelper(x, y, x - 1, y, 1);
                statHelper(x, y, x - 1, y - 1, -1);
                //last line is false to make up for adding upper left rectangle twice
            }
        }

    }
}

void stats::statHelper(int x1, int y1, int x2, int y2, int plus) {
    this->sumRed    [x1][y1] += plus * this->sumRed    [x2][y2];
    this->sumGreen  [x1][y1] += plus * this->sumGreen  [x2][y2];
    this->sumBlue   [x1][y1] += plus * this->sumBlue   [x2][y2];
    this->sumsqRed  [x1][y1] += plus * this->sumsqRed  [x2][y2];
    this->sumsqGreen[x1][y1] += plus * this->sumsqGreen[x2][y2];
    this->sumsqBlue [x1][y1] += plus * this->sumsqBlue [x2][y2];
}

/* returns the sums of all pixel values across all color channels.
* useful in computing the score of a rectangle
* PA3 function
* @param channel is one of r, g, or b
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::getSum(char channel, pair<int,int> ul, pair<int,int> lr){
    vector< vector< long >> summer;
    switch (channel) {
        case 'r':
            summer = this->sumRed;
            break;
        case 'g':
            summer = this->sumGreen;
            break;
        case 'b':
            summer = this->sumBlue;
            break;
        default:
            return -1; // not possible with good usage
    }
    return sumHelper(summer, ul, lr);
}

/* returns the sums of squares of all pixel values across all color channels.
* useful in computing the score of a rectangle
* PA3 function
* @param channel is one of r, g, or b
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::getSumSq(char channel, pair<int,int> ul, pair<int,int> lr){
    vector< vector< long >> summer;
    switch (channel) {
        case 'r':
            summer = this->sumsqRed;
            break;
        case 'g':
            summer = this->sumsqGreen;
            break;
        case 'b':
            summer = this->sumsqBlue;
            break;
        default:
            return -1; // not possible with good usage
    }
    return sumHelper(summer, ul, lr);
}

long stats::sumHelper(vector< vector< long >> summer, pair<int,int> ul, pair<int,int> lr) {
    int xul = ul.first, yul = ul.second, xlr = lr.first, ylr = lr.second;
    if (xul == 0 && yul == 0) {
        return summer[xlr][ylr];
    } else if (xul == 0) {
        return summer[xlr][ylr] - summer[xlr][yul-1];
    } else if (yul == 0) {
        return summer[xlr][ylr] - summer[xul - 1][ylr];
    } else {
        return summer[xlr][ylr] - summer[xul - 1][ylr] - summer[xlr][yul-1]
                + summer[xul - 1][yul-1];
        // last term to cover up for second removal of ul rectangle
    }
}

/* given a rectangle, return the number of pixels in the rectangle
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::rectArea(pair<int,int> ul, pair<int,int> lr){
    return (lr.second - ul.second + 1) * (lr.first - ul.first + 1);
}

/* given a rectangle, compute its sum of squared deviations from mean, over all color channels.
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
long stats::getScore(pair<int,int> ul, pair<int,int> lr){
    long ar = rectArea(ul, lr);
    long r   = getSum('r', ul, lr);
    long g   = getSum('g', ul, lr);
    long b   = getSum('b', ul, lr);
    long rsq = getSumSq('r', ul, lr);
    long gsq = getSumSq('g', ul, lr);
    long bsq = getSumSq('b', ul, lr);
    return (rsq - r * r / ar) + (gsq - g * g / ar) + (bsq - b * b / ar);
}

/* given a rectangle, return the average color value over the rectangle as a pixel.
* Each color component of the pixel is the average value of that component over
* the rectangle.
* @param ul is (x,y) of the upper left corner of the rectangle
* @param lr is (x,y) of the lower right corner of the rectangle */
RGBAPixel stats::getAvg(pair<int,int> ul, pair<int,int> lr){
    long ar = rectArea(ul, lr);
    int r = (int) getSum('r', ul, lr) / ar;
    int g = (int) getSum('g', ul, lr) / ar;
    int b = (int) getSum('b', ul, lr) / ar;
    return RGBAPixel(r, g, b);
}
