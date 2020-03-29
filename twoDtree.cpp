
/** twoDtree (pa3)
 * slight modification of a Kd tree of dimension 2.
 * twoDtree.cpp
 * This file will be used for grading. */

#include "twoDtree.h"

// Node constructor, given.
twoDtree::Node::Node(pair<int,int> ul, pair<int,int> lr, RGBAPixel a)
        :upLeft(ul),lowRight(lr),avg(a),left(NULL),right(NULL)
{}

twoDtree::~twoDtree(){
    clear();
}

twoDtree::twoDtree(const twoDtree & other) {
    copy(other);
}

twoDtree & twoDtree::operator=(const twoDtree & rhs){
    if (this != &rhs) {
        clear();
        copy(rhs);
    }
    return *this;
}

twoDtree::twoDtree(PNG & imIn){
    this->height = imIn.height();
    this->width = imIn.width();

    pair <int, int> ul (0, 0);
    pair <int, int> lr (this->width - 1, this->height - 1);
    stats s = stats(imIn);
    this->root = buildTree(s, ul, lr, true);
}

twoDtree::Node * twoDtree::buildTree(stats & s, pair<int,int> ul, pair<int,int> lr, bool vert) {
    int xul = ul.first, yul = ul.second, xlr = lr.first, ylr = lr.second;
    Node * curr = new Node(ul, lr, s.getAvg(ul, lr));
    long maxVal = LONG_MAX;
    pair<int,int> slr = lr;
    pair<int,int> sul = ul;
    if (yul == ylr && xul == xlr && s.rectArea(ul, lr) == 1) {
        curr->left = NULL;
        curr->right = NULL;
        return curr; // check for base case
    } else if (yul == ylr) { // vertical split regardless
        goto vertLabel;
    } else if (xul == xlr) { // horizontal split regardless
        goto horizLabel;
    } else if (vert) { //vertical split
        vertLabel:
        slr.first  = xul;
        sul.first  = xul + 1;
        for (int x = xul; x < xlr; x++) {
            pair<int,int> leftLR (x, ylr);
            pair<int,int> rightUL (x + 1, yul);
            //sum of variances of two rectangles
            long rectScore = s.getScore(ul, leftLR) + s.getScore(rightUL, lr);
            if (rectScore <= maxVal) {
                maxVal = rectScore;
                slr.first = x;
                sul.first = x + 1;
            }
        }
        goto endLabel;
    } else { // horizontal split
        horizLabel:
        slr.second = yul;
        sul.second = yul + 1;
        for (int y = yul; y < ylr; y++) {
            pair<int,int> topLR (xlr, y);
            pair<int,int> bottomUL (xul, y + 1);
            //sum of variances of two rectangles
            long rectScore = s.getScore(ul, topLR) + s.getScore(bottomUL, lr);
            if (rectScore <= maxVal) {
                maxVal = rectScore;
                slr.second = y;
                sul.second = y + 1;
            }
        }
        goto endLabel;
    }

    endLabel:
    curr->left = buildTree(s,  ul, slr, !vert);
    curr->right = buildTree(s, sul, lr, !vert);
    return curr;
}


PNG twoDtree::render(){
//    PNG image = PNG((unsigned int) this->width, (unsigned int) this->height);
    PNG image = PNG(this->width, this->height);
    renderHelper(this->root, image);
    return image;
}

void twoDtree::renderHelper(Node * curr, PNG &img){
    if (curr) {
        if(!curr->left && !curr->right) {
            *(img.getPixel(curr->upLeft.first, curr->upLeft.second)) = curr->avg;
        }
/*            for (int y = curr->upLeft.second; y < curr->lowRight.second; y++) {
*                for (int x = curr->upLeft.first; x < curr->lowRight.first; x++) {
*                    *img.getPixel((unsigned int) x, (unsigned int) y) = curr->avg;
*          //the leaf holds the average color for its rectangle
*                }
*            }
*        } else {
*        recursion to find leaves and thus smallest current rectangles
*        chance that l or r is NULL, so I still check notNull at start */
        renderHelper(curr->left, img);
        renderHelper(curr->right, img);
        //}
    }
}

/*
 * The idealPrune function can be considered to be the inverse
 * of the pruneSize function. It takes as input a number of leaves
 * and returns the minimum tolerance that would produce that resolution
 * upon a prune. It does not change the structure of the tree.*/
int twoDtree::idealPrune(int leaves){
    int tempLeaves = pruneSizeHelper(1, this->root);//decreases as tol rises
    int tol;
    for (tol = 1; tempLeaves > leaves; tol ++) {
        tempLeaves = pruneSizeHelper(tol, this->root);
    }
    if (tempLeaves < leaves) {
        tol--; // we want smallest value that still gives |leaves| leaves, and no less leaves
    }
    return tol;
}

/*
 * The pruneSize function takes a tolerance as input, and returns
 * the number of leaves that would result _if_ the tree were to
 * be pruned with that tolerance. Consistent with the definition
 * of prune, a node is counted if all of the leaves in its subtree
 * are within tol of the node's color.*/
int twoDtree::pruneSize(int tol){
    int totalLeaves = numLeaves(this->root, false, -1, this->root->avg);
    return totalLeaves - pruneSizeHelper(tol, this->root);
}

int twoDtree::pruneSizeHelper(int tol, Node *curr) {
    if (curr) {
        bool isPrune = true;
        int numL = numLeaves(curr, !isPrune, -1, curr->avg);
        if ((numLeaves(curr, isPrune, tol, curr->avg) == numL)) {
            return numL;
        } else {
            return pruneSizeHelper(tol, curr->right)
                   + pruneSizeHelper(tol, curr->left);
        }
    } else {
        return 0;
    }
}

/*
 *  Prune function trims subtrees as high as possible in the tree.
 *  A subtree is pruned (cleared) if all of its leaves are within
 *  tol of the average color stored in the root of the subtree.
 *  Pruning criteria should be evaluated on the original tree, not
 *  on a pruned subtree. (we only expect that trees would be pruned once.) */
void twoDtree::prune(int tol){
    pruneHelper(tol, this->root);
}

/* both number of leaves and number of prunable leaves are similar,
 * so we can combine them closely into one helper func*/
int twoDtree::numLeaves(Node *curr, bool checkPrune, int tol, RGBAPixel av) {
    if (!curr) {
        return 0;
    } else if (!curr->left && !curr->right) {
        if (!checkPrune) {
            return 1;
        } else {
            int dr = curr->avg.r - av.r;
            int dg = curr->avg.g - av.g;
            int db = curr->avg.b - av.b;
            int var = dr * dr + dg * dg + db * db;
            return (int) (var < tol);
//            return (int) (var <= tol);
        }
    } else {
        return numLeaves(curr->left, checkPrune, tol, av)
               + numLeaves(curr->right, checkPrune, tol, av);
    }
}

void twoDtree::pruneHelper(int tol, Node *curr) {
    if (curr) {
        bool isCopy = false, isPrune = true;

        if ((numLeaves(curr, isPrune, tol, curr->avg)
             == numLeaves(curr, !isPrune, tol, curr->avg))) {
            curr->avg = pruner(curr, curr->avg);
//            curr->left = copyClearHelper(curr->left, isCopy);
//            curr->right = copyClearHelper(curr->right, isCopy);

        } else {
            pruneHelper(tol, curr->right);
            pruneHelper(tol, curr->left);
        }
    }
}

RGBAPixel twoDtree::pruner(Node *curr, RGBAPixel avgValue) {
    if (curr) {
        curr->left->avg = pruner(curr->left, avgValue);
        curr->right->avg = pruner(curr->right, avgValue);
        return avgValue;
    }
}

void twoDtree::clear() {
    bool isCopy = false;
    this->root = copyClearHelper(this->root, isCopy);
    this->height = 0;
    this->width = 0;
}

void twoDtree::copy(const twoDtree & orig){
    this->width = orig.width;
    this->height = orig.height;
    bool isCopy = true;
    this->root = copyClearHelper(orig.root, isCopy);
}

/*both copy and clear use pre-order traversal so we can
 * combine them closely into one helper func*/
twoDtree::Node* twoDtree::copyClearHelper(Node *curr, bool isCopy) {
    if (curr) {
        Node *roott;
        if (isCopy) {
            roott = new Node(curr->upLeft, curr->lowRight, curr->avg);
        } else {
            roott = curr;
        }
        roott->left = copyClearHelper(curr->left, isCopy);
        roott->right = copyClearHelper(curr->right, isCopy);
        if (!isCopy) {
            delete roott;
            return NULL; //return nullptr;
        } else {
            return roott;
        }
    }
    return NULL; //return nullptr;
}