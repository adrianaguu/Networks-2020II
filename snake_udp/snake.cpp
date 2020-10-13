#include <iostream>

#include <vector>
#include <ctype.h>
using namespace std;

bool gameover;

const int width = 30;

const int height = 30;

char matrix[width][height];


vector<int> nTail(14);

int points = 100;

enum eDirecton
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
}; // Controls

eDirecton dir;

void Setup()
{
    gameover = false;

    dir = STOP;


    nTail[0] = rand() % (width - 7) + 7;
    nTail[1] = rand() % (height - 7) + 7;

    for (int i = 2; i < 14; i += 2)
    {
        nTail[i] = nTail[i - 2] - 1;
        nTail[i + 1] = nTail[1];
    }

    for (int i = 0; i < height; i++)
    {

        for (int j = 0; j < width; j++)
        {
            if (j == 0 || j == width - 1 || i == 0 || i == height - 1)

                matrix[i][j] = '#';

            else
            {
                matrix[i][j] = ' ';
            }
        }
    }

    matrix[nTail[0]][nTail[1]] = 'A';

    for (int i = 2; i < 14; i += 2)
    {
        matrix[nTail[i]][nTail[i + 1]] = '*';
    }
}

void Draw()
{

    for (int i = 0; i < height; i++)
    {

        for (int j = 0; j < width; j++)
        {

            cout << matrix[i][j];
        }
        cout << endl;
    }

    cout << "POINTS: " << points << endl;
}

void Input()
{
    char c;
    cin >> c;

    switch (c)
    {

    case 'a':

        dir = LEFT;

        break;

    case 'd':

        dir = RIGHT;

        break;

    case 'w':

        dir = UP;

        break;

    case 's':

        dir = DOWN;

        break;

    case 'x':

        gameover = true;

        break;
    }
}

void algorithm()
{
    int prev0 = nTail[0];
    int prev1 = nTail[1];
    int last0 = nTail[12];
    int last1 = nTail[13];

    switch (dir)
    {

    case LEFT:

        prev1--;

        break;

    case RIGHT:

        prev1++;

        break;

    case UP:

        prev0--;

        break;

    case DOWN:

        prev0++;

        break;

    default:

        break;
    }

    if (isalpha(matrix[prev0][prev1]))
    {
        points += 5;
    }

    else if (matrix[prev0][prev1] == '#' || matrix[prev0][prev1] == '*')
    {
        points -= 1;
    }

    else
    {
        nTail.pop_back();
        nTail.pop_back();

        nTail.insert(nTail.begin(), prev1);
        nTail.insert(nTail.begin(), prev0);

        matrix[nTail[0]][nTail[1]] = 'A';
        matrix[last0][last1] = ' ';

        for (int i = 2; i < 14; i += 2)
        {
            matrix[nTail[i]][nTail[i + 1]] = '*';
        }
    }
}

int main()
{

    Setup();

    while (!gameover)
    {

        Draw();

        Input();

        algorithm();
    }

    return 0;
}
