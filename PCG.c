// Colby Troutt
// AI Homework #3 - Procedural Content Generation


// Include statements.
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


// Constants.
#define MAPSIZE 200
#define NUM_MAPS 5
#define ROOMSIZE 5000
#define NUM_BRANCHES 4
#define MAX_CELL_SIZE 2000


// A bunch of structs.
typedef struct
{
    int x;
    int y;
} point;

typedef struct
{
    point *topleft;
    point *bottomright;
} room;

typedef struct
{
    room *myroom;
    int leftwall;
    int rightwall;
    int topwall;
    int bottomwall;
} cell;

typedef struct node
{
    cell *currentcell;
    struct node *children[NUM_BRANCHES];
} node;


// Functional Prototypes
int** allocateMapArray(int** maparray);                                             // DONE
int** initializeMapArray(int** maparray);                                           // DONE
node* createNode(node* root);                                                       // DONE
node* initializeTree(node* root);                                                   // DONE
int getCellSize(node* root);                                                        // DONE
int** drawRoom(node* root, int** maparray);                                         // DONE
void printMap(FILE* ofp, int mapNumber, int** maparray);                            // DONE
void printTree(FILE* ofp, int mapNumber, node* root);                               // DONE
void printTreeRecursive(FILE* ofp, node* currentnode, int spacing);                 // DONE
void destroyTree(node* root);                                                       // DONE
void destroyMapArray(int** maparray);                                               // DONE
node* createPartitions(node* root);                                                 // DONE
node* createRooms(node* root);                                                      // DONE
int** drawRooms(node* root, int** maparray);                                        // DONE
int** connectRooms(node* root, int** maparray);                                     // DONE
int** connectHorizontal(node* root, int** maparray, int a);                         // DONE
int** connectVertical(node* root, int** maparray, int a);                           // DONE
int rowChecker(int** maparray, int x, int y, int direction, node* root);            // DONE
int columnChecker(int** maparray, int x, int y, int direction, node* root);         // DONE
point** findShortest(int** maparray, point a[], point b[], int hV);                 // DONE


// Main function.
int main()
{
    // Declaring some variables.
    int **mapgrid;
    FILE* ofp = fopen("output.txt", "w");
    int i = 0;
    node *tree;

    // Seeding the random number generator.
    srand(time(NULL));

    // Allocating the map array.
    mapgrid = allocateMapArray(mapgrid);

    // Looping through and doing each map.
    for (i = 0; i < NUM_MAPS; i++)
    {
        // Initializing the map to all black before generating it.
        mapgrid = initializeMapArray(mapgrid);

        // Initializing the tree.
        tree = initializeTree(tree);

        // Creating the partitions.
        tree = createPartitions(tree);

        // Creating the rooms.
        tree = createRooms(tree);

        // Drawing the rooms.
        mapgrid = drawRooms(tree, mapgrid);

        // Setting the paths between rooms.
        mapgrid = connectRooms(tree, mapgrid);

        // Printing the values in the tree.
//       printTree(ofp, i, tree);

        // Printing the completed map.
        printMap(ofp, i, mapgrid);

        // Destroying the tree after the fact.
        destroyTree(tree);
    }

    // Closing the file, freeing the map grid, and returning at the end.
    fclose(ofp);
    destroyMapArray(mapgrid);
    return 0;
}


// A function to dynamically allocate space for the map array.
int** allocateMapArray(int** maparray)
{
    // Declaring some variables.
    int i;

    // Allocating the first dimension.
    maparray = (int**)malloc(sizeof(int*) * MAPSIZE);

    // Allocating the second dimension.
    for (i = 0; i < MAPSIZE; i++)
    {
        maparray[i] = (int*)malloc(sizeof(int) * MAPSIZE);
    }

    // Returning the newly allocated array.
    return maparray;
}


// A function to initialize the map array before each map is generated.
int** initializeMapArray(int** maparray)
{
    // Declaring some variables.
    int i, j;

    // Looping to initialize.
    for (i = 0; i < MAPSIZE; i++)
    {
        for (j = 0; j < MAPSIZE; j++)
        {
            maparray[i][j] = 1;
        }
    }

    // Returning our initialized array.
    return maparray;
}


// A function to create a node and allocate/initialize necessary fields.
node* createNode(node* root)
{
    // Declaring some variables.
    int i;

    // Allocating the root node.
    root = (node*)malloc(sizeof(node));

    // Allocating the currentcell field of this node.
    root->currentcell = (cell*)malloc(sizeof(cell));

    // Initializing the children to NULL.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        root->children[i] = NULL;
    }

    // Allocating the room.
    root->currentcell->myroom = (room*)malloc(sizeof(room));

    // Setting all of the initial values of the walls.
    root->currentcell->leftwall = -1;
    root->currentcell->rightwall = -1;
    root->currentcell->topwall = -1;
    root->currentcell->bottomwall = -1;

    // Allocating the room fields.
    root->currentcell->myroom->topleft = (point*)malloc(sizeof(point));
    root->currentcell->myroom->bottomright = (point*)malloc(sizeof(point));

    // Initializing the point fields, then we're finally done!
    root->currentcell->myroom->topleft->x = -1;
    root->currentcell->myroom->topleft->y = -1;
    root->currentcell->myroom->bottomright->x = -1;
    root->currentcell->myroom->bottomright->y = -1;

    // Lastly, returning the newly created node.
    return root;
}


// A function to start the tree (root node, baby!).
node* initializeTree(node* root)
{
    // Creating the root node.
    root = createNode(root);

    // Initializing the cell  values.
    root->currentcell->leftwall = 0;
    root->currentcell->rightwall = MAPSIZE - 1;
    root->currentcell->topwall = 0;
    root->currentcell->bottomwall = MAPSIZE - 1;

    // Returning once it's all done.
    return root;
}


// A function to give the size of a partition based only on the node.
int getCellSize(node* root)
{
    // Declaring some variables.
    int base, height;

    // Finding the base and the height. You have to add one to include the actual partitions.
    base = (root->currentcell->rightwall) - (root->currentcell->leftwall) + 1;
    height = (root->currentcell->bottomwall) - (root->currentcell->topwall) + 1;

    // Finding the result by multiplying these two numbers together.
    return (base * height);
}


// A function that fills in a room once its outer limits have been established.
int** drawRoom(node* root, int** maparray)
{
    // Declaring some variables.
    int i, j;
    int leftx, rightx, topy, bottomy;

    // Initializing the coordinates.
    leftx = root->currentcell->myroom->topleft->x;
    rightx = root->currentcell->myroom->bottomright->x;
    topy = root->currentcell->myroom->topleft->y;
    bottomy = root->currentcell->myroom->bottomright->y;

    // Looping down the rows.
    for (i = topy; i <= bottomy; i++)
    {
        // Looping through each column in each row.
        for (j = leftx; j <= rightx; j++)
        {
            maparray[i][j] = 0;
        }
    }

    // Returning our modified array.
    return maparray;
}


// A function that prints a completed map to the output file.
void printMap(FILE* ofp, int mapNumber, int** maparray)
{
    // Declaring some variables.
    int i, j;

    // Printing an intro statement.
    fprintf(ofp, "Map #%d:\n****************************************************\n\n", mapNumber + 1);

    // Printing the map.
    for (i = 0; i < MAPSIZE; i++)
    {
        for (j = 0; j < MAPSIZE; j++)
        {
            fprintf(ofp, "%d", maparray[i][j]);
        }

        // Printing a new line at the end of each row.
        fprintf(ofp, "\n");
    }

    // Printing a couple of new lines after the map.
    fprintf(ofp, "\n\n\n");
}


// A wrapper function to make the tree-printing look cleaner from main.
void printTree(FILE* ofp, int mapNumber, node* root)
{
    // Printing an intro statement.
    fprintf(ofp, "Tree #%d:\n**************************\n\n", mapNumber + 1);

    // Doing the recursion part!
    printTreeRecursive(ofp, root, 0);

    // Printing some new lines at the end.
    fprintf(ofp, "\n\n\n");

    // Returning.
    return;
}


// A function to recursively go through the tree and print ALL the values!
void printTreeRecursive(FILE* ofp, node* currentnode, int spacing)
{
    // Base case - we can't print a node that isn't there!
    if (currentnode == NULL)
    {
        return;
    }

    // Declaring a ton of variables for neatness in print statements.
    int i, leftx, rightx, topy, bottomy, cellLeft, cellRight, cellTop, cellBottom;

    // Setting the values of all those variables.
    leftx = currentnode->currentcell->myroom->topleft->x;
    rightx = currentnode->currentcell->myroom->bottomright->x;
    topy = currentnode->currentcell->myroom->topleft->y;
    bottomy = currentnode->currentcell->myroom->bottomright->y;
    cellLeft = currentnode->currentcell->leftwall;
    cellRight = currentnode->currentcell->rightwall;
    cellTop = currentnode->currentcell->topwall;
    cellBottom = currentnode->currentcell->bottomwall;

    // Printing the appropriate number of extra spaces for formatting.
    for (i = 0; i < spacing; i++)
    {
        fprintf(ofp, "     ");
    }

    // Oh boy, here I go printing again!
    fprintf(ofp, "-> NODE DEPTH: %d || PARTITION EDGES (Left, Right, Top, Bottom): (%d, %d, %d, %d) || ROOM TOP LEFT (x, y): (%d, %d) || ROOM BOTTOM RIGHT (x, y): (%d, %d)\n",
            spacing, cellLeft, cellRight, cellTop, cellBottom, leftx, topy, rightx, bottomy);

    // Doing this same stuff on the children.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        printTreeRecursive(ofp, currentnode->children[i], spacing+1);
    }

    // Returning at the end.
    return;
}


// A function to free all the memory in the current tree.
void destroyTree(node* root)
{
    // Base case - we can't free a node that doesn't exist!
    if (root == NULL)
    {
        return;
    }

    // Declaring a variable to loop.
    int i;

    // Otherwise, we free the children first.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        destroyTree(root->children[i]);
    }

    // Freeing the current node.
    free(root);

    // Returning.
    return;
}


// A function to free all of the memory allocated for the map grid.
void destroyMapArray(int** maparray)
{
    // Declaring a variable for looping.
    int i;

    // Looping through and freeing things.
    for (i = 0; i < MAPSIZE; i++)
    {
        free(maparray[i]);
    }

    // Freeing the maparray now.
    free(maparray);

    // Returning.
    return;
}


// A function to create the partitions.
node* createPartitions(node* root)
{
    // Base case - if the node is null, we can't do anything.
    if (root == NULL)
    {
        return root;
    }

    // Another base case - if the node is the right size, just return it the way it is!
    if (getCellSize(root) <= ROOMSIZE)
    {
        return root;
    }

    // Declaring some variables now that we have to use them.
    int i, horizontal, vertical, horizontalRange, verticalRange;

    // Setting the horizontal and vertical ranges.
    horizontalRange = root->currentcell->bottomwall - root->currentcell->topwall;
    verticalRange = root->currentcell->rightwall - root->currentcell->leftwall;

    // Deciding where the horizontal split happens. I'm keeping it within the middle third of the cell to avoid having rooms that are too small.
    do {
        horizontal = rand() % horizontalRange + root->currentcell->topwall;
    } while (horizontal < (root->currentcell->topwall + horizontalRange/3) || horizontal > (root->currentcell->bottomwall - horizontalRange/3) ||
             (horizontal - root->currentcell->topwall) < 20 || (root->currentcell->bottomwall - horizontal) < 20);

    // Deciding where the vertical split happens. I'm keeping it within the middle third of the cell to avoid having rooms that are too small.
    do {
        vertical = rand() % verticalRange + root->currentcell->leftwall;
    } while (vertical < (root->currentcell->leftwall + verticalRange/3) || vertical > (root->currentcell->rightwall - verticalRange/3) ||
             (vertical - root->currentcell->leftwall) < 20 || (root->currentcell->rightwall - vertical) < 20);

    // Creating the children of this node.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        root->children[i] = createNode(root->children[i]);
    }

    // Top left child.
    root->children[0]->currentcell->topwall = root->currentcell->topwall;
    root->children[0]->currentcell->bottomwall = horizontal;
    root->children[0]->currentcell->leftwall = root->currentcell->leftwall;
    root->children[0]->currentcell->rightwall = vertical;

    // Top right child.
    root->children[1]->currentcell->topwall = root->currentcell->topwall;
    root->children[1]->currentcell->bottomwall = horizontal;
    root->children[1]->currentcell->leftwall = vertical + 1;
    root->children[1]->currentcell->rightwall = root->currentcell->rightwall;

    // Bottom left child.
    root->children[2]->currentcell->topwall = horizontal + 1;
    root->children[2]->currentcell->bottomwall = root->currentcell->bottomwall;
    root->children[2]->currentcell->leftwall = root->currentcell->leftwall;
    root->children[2]->currentcell->rightwall = vertical;

    // Bottom right child.
    root->children[3]->currentcell->topwall = horizontal + 1;
    root->children[3]->currentcell->bottomwall = root->currentcell->bottomwall;
    root->children[3]->currentcell->leftwall = vertical + 1;
    root->children[3]->currentcell->rightwall = root->currentcell->rightwall;

    // Now to partition the children.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        root->children[i] = createPartitions(root->children[i]);
    }

    // Returning at the end.
    return root;
}


// A function to create the rooms.
node* createRooms(node* root)
{
    // Base case - we can't do anything with a null root.
    if (root == NULL)
    {
        return root;
    }

    // Declaring some variables.
    int i, counter = 0, leftwall, rightwall, topwall, bottomwall, leftx, rightx, topy, bottomy, horizontalSpread, verticalSpread;

    // Checking that we're at the leaf nodes.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        // Making sure all children are null.
        if (root->children[i] == NULL)
        {
            counter++;
        }
    }

    // If this node has no children, we're making a room in this node.
    if (counter == 4)
    {
        // Setting these variables for convenience.
        leftwall = root->currentcell->leftwall;
        rightwall = root->currentcell->rightwall;
        topwall = root->currentcell->topwall;
        bottomwall = root->currentcell->bottomwall;

        // Setting the spreads.
        horizontalSpread = rightwall - leftwall;
        verticalSpread = bottomwall - topwall;

        // Getting the top left x value. I'm forcing it to be in the left 3/4 of the cell.
        do
        {
            root->currentcell->myroom->topleft->x = rand() % horizontalSpread + leftwall + 1;
        } while (root->currentcell->myroom->topleft->x > (leftwall + 1 + horizontalSpread/10));

        // Getting the top left y value. I'm forcing it to be in the top 3/4 of the cell.
        do
        {
            root->currentcell->myroom->topleft->y = rand() % verticalSpread + topwall + 1;
        } while (root->currentcell->myroom->topleft->y > (topwall + 1 + verticalSpread/10));

        // Getting the bottom right x value. I'm forcing the room to be at least 5x5.
        do
        {
            root->currentcell->myroom->bottomright->x = rand() % (rightwall - (root->currentcell->myroom->topleft->x)) + root->currentcell->myroom->topleft->x;
        } while ((root->currentcell->myroom->bottomright->x - root->currentcell->myroom->topleft->x) < 15);

        // Getting the bottom right y value. I'm forcing the room to be at least 5x5.
        do
        {
            root->currentcell->myroom->bottomright->y = rand() % (bottomwall - (root->currentcell->myroom->topleft->y)) + root->currentcell->myroom->topleft->y;
        } while ((root->currentcell->myroom->bottomright->y - root->currentcell->myroom->topleft->y) < 15);
    }

    // Now that the room has or has not been drawn, we go to the children of this node.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        root->children[i] = createRooms(root->children[i]);
    }

    // Returning now that we're done.
    return root;
}


// A function to traverse the tree specifically to draw the rooms.
int** drawRooms(node* root, int** maparray)
{
    // Base case - if there's not a tree, we just return.
    if (root == NULL)
    {
        return maparray;
    }

    // Another base case - if the map is null, we return.
    if (maparray == NULL)
    {
        return maparray;
    }

    // Declaring a variable.
    int i = 0;

    // If the values of the room are -1, we just move to a different node.
    if (root->currentcell->myroom->topleft->x == -1)
    {
        // Calling this on all of the children.
        for (i = 0; i < NUM_BRANCHES; i++)
        {
            maparray = drawRooms(root->children[i], maparray);
        }
    }

    // If the room has values, then we can draw a room!
    else
    {
        // Drawing the room.
        maparray = drawRoom(root, maparray);
    }

    // Returning when we're done regardless.
    return maparray;
}


// A function to connect the rooms on the map.
int** connectRooms(node* root, int** maparray)
{
    // Declaring a variable early.
    int loop, counter = 0, randomnumber;

    // Base case - we can't do anything if the node is null.
    if (root == NULL)
    {
        return maparray;
    }

    // Checking to see if this is a leaf node.
    for (loop = 0; loop < NUM_BRANCHES; loop++)
    {
        if (root->children[loop] == NULL)
        {
            counter++;
        }
    }

    // Second base case - we can't connect the children of a leaf node.
    if (counter == 4)
    {
        return maparray;
    }


    // Declaring some variables.
    int i;

    // We need to work our way down the tree before we start doing things.
    for (i = 0; i < NUM_BRANCHES; i++)
    {
        maparray = connectRooms(root->children[i], maparray);
    }

    // We call three of the connecting functions randomly! (At least three have to be called all rooms are reachable)
    randomnumber = rand() % 4;

    // Top is empty.
    if (randomnumber == 0)
    {
        maparray = connectHorizontal(root, maparray, 2);
        maparray = connectVertical(root, maparray, 0);
        maparray = connectVertical(root, maparray, 1);
    }

    // Right is empty.
    else if (randomnumber == 1)
    {
        maparray = connectHorizontal(root, maparray, 0);
        maparray = connectHorizontal(root, maparray, 2);
        maparray = connectVertical(root, maparray, 0);
    }

    // Bottom is empty.
    else if (randomnumber == 2)
    {
        maparray = connectHorizontal(root, maparray, 0);
        maparray = connectVertical(root, maparray, 0);
        maparray = connectVertical(root, maparray, 1);
    }

    // Left is empty.
    else if (randomnumber == 3)
    {
        maparray = connectHorizontal(root, maparray, 0);
        maparray = connectHorizontal(root, maparray, 2);
        maparray = connectVertical(root, maparray, 1);
    }

    // Returning the now connected maparray.
    return maparray;
}


// A function to find and connect two rooms horizontally.
int** connectHorizontal(node* root, int** maparray, int a)
{
    // Declaring some variables.
    int i, j, k = 0, randomnumber, counter = 0;
    point section1[MAPSIZE], section2[MAPSIZE], **connections;

    // Initializing the values in the sections.
    for (i = 0; i < MAPSIZE; i++)
    {
        section1[i].x = -1;
        section1[i].y = -1;
        section2[i].x = -1;
        section2[i].y = -1;
    }

    // We go through one section, and we make an array of all of the possible connections.
    for (i = root->children[a]->currentcell->topwall; i <= root->children[a]->currentcell->bottomwall; i++)
    {
        // Columns now.
        for (j = root->children[a]->currentcell->leftwall; j <= root->children[a]->currentcell->rightwall; j++)
        {
            // If it's a 0, we check it.
            if (maparray[i][j] == 0)
            {
                // If it's the rightmost 0, we keep it.
                if (rowChecker(maparray, j, i, 1, root->children[a]) == 1)
                {
                    // We also increment k.
                    section1[k].x = j;
                    section1[k].y = i;
                    k++;
                }
            }
        }
    }

    // Resetting k real quick.
    k = 0;

    // Next, we go through the other section and check it.
    for (i = root->children[a+1]->currentcell->topwall; i <= root->children[a+1]->currentcell->bottomwall; i++)
    {
        // Every single tile.
        for (j = root->children[a+1]->currentcell->leftwall; j <= root->children[a+1]->currentcell->rightwall; j++)
        {
            // If it's a 0, we check it.
            if (maparray[i][j] == 0)
            {
                // If it's the leftmost 0, we keep it.
                if (rowChecker(maparray, j, i, 0, root->children[a+1]) == 1)
                {
                    section2[k].x = j;
                    section2[k].y = i;
                    k++;
                }
            }
        }
    }

    // Finding where the minimum connection distances are.
    connections = findShortest(maparray, section1, section2, 0);

    // Making sure there are connections.
    if (connections[0]->x == -1)
    {
        // Error behavior.
        return maparray;
    }

    // Finding the number of possible connections.
    while(connections[counter]->x != -1)
    {
        // Counting, lol.
        counter++;
    }

    // Counter is the number of connections now, so let's choose one of them randomly.
    randomnumber = rand() % counter;

    // Now that we know where they are going to connect, we just connect them.
    i = connections[randomnumber]->x + 1;
    while (maparray[connections[randomnumber]->y][i] != 0)
    {
        // Making this tile a zero, and moving over a tile.
        maparray[connections[randomnumber]->y][i] = 0;
        i++;
    }

    // Returning the connected rooms!
    free(connections);
    return maparray;
}


// A function to find and connect two rooms vertically.
int** connectVertical(node* root, int** maparray, int a)
{
 // Declaring some variables.
    int i, j, k = 0, randomnumber, counter = 0;
    point section1[MAPSIZE], section2[MAPSIZE], **connections;

    // Initializing the values in the sections.
    for (i = 0; i < MAPSIZE; i++)
    {
        section1[i].x = -1;
        section1[i].y = -1;
        section2[i].x = -1;
        section2[i].y = -1;
    }

    // We go through one section, and we make an array of all of the possible connections.
    for (i = root->children[a]->currentcell->topwall; i <= root->children[a]->currentcell->bottomwall; i++)
    {
        // Columns now.
        for (j = root->children[a]->currentcell->leftwall; j <= root->children[a]->currentcell->rightwall; j++)
        {
            // If it's a 0, we check it.
            if (maparray[i][j] == 0)
            {
                // If it's the bottommost 0, we keep it.
                if (columnChecker(maparray, j, i, 1, root->children[a]) == 1)
                {
                    // We also increment k.
                    section1[k].x = j;
                    section1[k].y = i;
                    k++;
                }
            }
        }
    }

    // Resetting k real quick.
    k = 0;

    // Next, we go through the other section and check it.
    for (i = root->children[a+2]->currentcell->topwall; i <= root->children[a+2]->currentcell->bottomwall; i++)
    {
        // Every single tile.
        for (j = root->children[a+2]->currentcell->leftwall; j <= root->children[a+2]->currentcell->rightwall; j++)
        {
            // If it's a 0, we check it.
            if (maparray[i][j] == 0)
            {
                // If it's the leftmost 0, we keep it.
                if (columnChecker(maparray, j, i, 0, root->children[a+2]) == 1)
                {
                    section2[k].x = j;
                    section2[k].y = i;
                    k++;
                }
            }
        }
    }

    // Finding where the minimum connection distances are.
    connections = findShortest(maparray, section1, section2, 1);

    // Making sure there are connections.
    if (connections[0]->y == -1)
    {
        // Error behavior.
        return maparray;
    }

    // Finding the number of possible connections.
    while(connections[counter]->x != -1)
    {
        // Counting, lol.
        counter++;
    }

    // Counter is the number of connections now, so let's choose one of them randomly.
    randomnumber = rand() % counter;

    // Now that we know where they are going to connect, we just connect them.
    i = connections[randomnumber]->y + 1;
    while (maparray[i][connections[randomnumber]->x] != 0)
    {
        // Making this tile a zero, and moving over a tile.
        maparray[i][connections[randomnumber]->x] = 0;
        i++;
    }

    // Returning the connected rooms!
    free(connections);
    return maparray;
}


// A function to check a whole row to see if that is the furthest 0 in whichever direction.
int rowChecker(int** maparray, int x, int y, int direction, node* root)
{
    // Declaring some variables.
    int i;

    // If it's in the left wall, it has to be farthest left.
    if (x == root->currentcell->leftwall && direction == 0)
    {
        // Returning false.
        return 1;
    }

    // If it's in the right wall, it has to be farthest right.
    if (x == root->currentcell->rightwall && direction == 1)
    {
        // Returning false.
        return 1;
    }

    // If direction is 0, we're looking to see if this is the leftmost zero in this row.
    if (direction == 0)
    {
        // Checking everything to the left.
        for (i = x-1; i >= root->currentcell->leftwall; i--)
        {
            // Looking for another 0.
            if (maparray[y][i] == 0)
            {
                // Returning false if we find one.
                return 0;
            }
        }

        // Returning true if we never do.
        return 1;
    }

    // If direction is 1, we're looking to see if this is the rightmost zero in this row.
    else if (direction == 1)
    {
        // Checking everything to the right.
        for (i = x+1; i <= root->currentcell->rightwall; i++)
        {
            // Looking for another 0.
            if (maparray[y][i] == 0)
            {
                // Returning false if we find one.
                return 0;
            }
        }

        // Returning true if we never do.
        return 1;
    }

    // If direction isn't a 1 or a 0, we're returning invalid.
    else
    {
        return -1;
    }
}


// A function to check an entire column to see if a certain 0 is the farthest up or down.
int columnChecker(int** maparray, int x, int y, int direction, node* root)
{

    // Declaring some variables.
    int i;

    // If it's in the top wall, it has to be farthest up.
    if (y == root->currentcell->topwall && direction == 0)
    {
        // Returning true.
        return 1;
    }

    // If it's in the bottom wall, it has to be farthest down.
    if (y == root->currentcell->bottomwall && direction == 1)
    {
        // Returning true.
        return 1;
    }

    // If direction is 0, we're looking to see if this is the highest zero in this column.
    if (direction == 0)
    {
        // Checking everything to the up.
        for (i = y-1; i >= root->currentcell->topwall; i--)
        {
            // Looking for another 0.
            if (maparray[i][x] == 0)
            {
                // Returning false if we find one.
                return 0;
            }
        }

        // Returning true if we never do.
        return 1;
    }

    // If direction is 1, we're looking to see if this is the bottom zero in this column.
    else if (direction == 1)
    {
        // Checking everything to the right.
        for (i = y+1; i <= root->currentcell->bottomwall; i++)
        {
            // Looking for another 0.
            if (maparray[i][x] == 0)
            {
                // Returning false if we find one.
                return 0;
            }
        }

        // Returning true if we never do.
        return 1;
    }

    // If direction isn't a 1 or a 0, we're returning invalid.
    else
    {
        return -1;
    }
}


// A function to find the shortest possible connection. hV decides horizontal (0) or vertical (1).
point** findShortest(int** maparray, point a[], point b[], int hV)
{
    // Declaring some variables.
    int i, j, k = 0, minDistance = MAPSIZE, aLength = 0, bLength = 0, counter = 1;
    point** connections;

    // Finding the length of the first list to avoid unnecessarily long loops.
    while (a[aLength].x != -1)
    {
        // Counting the length.
        aLength++;
    }

    // Finding the length of the second list.
    while (b[bLength].x != -1)
    {
        // Counting the length.
        bLength++;
    }

    // If hV is 0, we're doing horizontal.
    if (hV == 0)
    {
        // Comparing each of them, and we're finding the closest one we can connect.
        for (i = 0; i < aLength; i++)
        {
            // Checking them against each other.
            for (j = 0; j < bLength; j++)
            {
                // If they have the same y coordinate, we check the distance.
                if (a[i].y == b[j].y)
                {
                    // We subtract the x values to find the distance.
                    if (b[j].x - a[i].x < minDistance)
                    {
                        // We keep that distance if it's the new minimum.
                        minDistance = b[j].x - a[i].x;
                    }
                }
            }
        }

        // Going back through and counting the ones with the minimum value.
        for (i = 0; i < aLength; i++)
        {
            // We're looking through all of them again to count the minimum distance points now.
            for (j = 0; j < bLength; j++)
            {
                // Checking that they match up again
                if (b[j].y == a[i].y)
                {
                    // Checking to see if it's the minimum distance.
                    if (b[j].x - a[i].x == minDistance)
                    {
                        // Counting it to allocate the right amount of memory.
                        counter++;
                    }
                }
            }
        }

        // Allocating the connections array, then filling it.
        connections = (point**)malloc(sizeof(point*) * counter);
        for (i = 0; i < counter; i++)
        {
            connections[i] = (point*)malloc(sizeof(point));
        }

        // Setting the end to -1 by default to detect the end later.
        connections[counter-1]->x = -1;
        connections[counter-1]->y = -1;

        // Now filling the array with the connections themselves. Here we go looping again!
        for (i = 0; i < aLength; i++)
        {
            // Last time looking through both arrays like this.
            for (j = 0; j < bLength; j++)
            {
                // Checking that they line up one more time.
                if (b[j].y == a[i].y)
                {
                    // Finding the ones with the minimum distance.
                    if (b[j].x - a[i].x == minDistance)
                    {
                        // Adding them to the connections array.
                        connections[k]->x = a[i].x;
                        connections[k]->y = a[i].y;

                        // Also, incrementing k.
                        k++;
                    }
                }
            }
        }
    }

    // If hV is 1, we're doing vertical.
    else if (hV == 1)
    {
        // Looping through both lists.
        for (i = 0; i < aLength; i++)
        {
            // We're going to compare the lists.
            for (j = 0; j < bLength; j++)
            {
                // Checking if they share an x coordinate.
                if (a[i].x == b[j].x)
                {
                    // We subtract the y values to find the distance.
                    if (b[j].y - a[i].y < minDistance)
                    {
                        // We keep the index and the distance if the distance is minimum.
                        minDistance = b[j].y - a[i].y;
                    }
                }
            }
        }

        // Counting the ones with the minimum distance.
        for (i = 0; i < aLength; i++)
        {
            // Second time through, but counting this time.
            for (j = 0; j < bLength; j++)
            {
                // Making sure they're lined up still.
                if (a[i].x == b[j].x)
                {
                    // If they're the minimum distance
                    if (b[j].y - a[i].y == minDistance)
                    {
                        // Counting it.
                        counter++;
                    }
                }
            }
        }

        // Allocating the space for the connections.
        connections = (point**)malloc(sizeof(point*)*counter);
        for (i = 0; i < counter; i++)
        {
            connections[i] = (point*)malloc(sizeof(point));
        }

        // Initializing the last part of the array to find its length later.
        connections[counter-1]->x = -1;
        connections[counter-1]->y = -1;

        // Looping through one more time.
        for (i = 0; i < aLength; i++)
        {
            // I'm saving the values this time.
            for (j = 0; j < bLength; j++)
            {
                // Making sure they have a matching x value.
                if (a[i].x == b[j].x)
                {
                    // Making sure they have the minimum distance.
                    if (b[j].y - a[i].y == minDistance)
                    {
                        // We're adding this one to the connections and incrementing k.
                        connections[k]->x = a[i].x;
                        connections[k]->y = a[i].y;
                        k++;
                    }
                }
            }
        }
    }

    // We're returning invalid if it's not one of those.
    else
    {
        // Returning invalid.
        return NULL;
    }

    // Checking if the minimum index is a valid index.
    if (connections[0]->x == -1 && connections[0]->y == -1)
    {
printf("%d, %d\n", connections[0]->x, connections[0]->y);

        // Printing an error so I can see it.
        printf("No connections could be made in the findShortest function.\n");

        // Returning invalid.
        return connections;
    }

    // Returning the connections.
    return connections;
}




