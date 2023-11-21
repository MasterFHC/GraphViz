/*
Extensions:
    -Adjusted Parameters:
        -kRepel=3*(1e-3)
        -kAttract=7*(1e-3)
    -Random Pertubation:
        -a pertubation on every node every 10 rounds
        -kPertubation=5*(1e-3)
    -Recessing Force:
        -forces exerted will decrease overtime
        -the multiplier starts at 4.0
        -slowly decrease to 1.0 after 500 rounds
    -Stop iteration automatically once the graph is almost static
        -minEnergy=4*(1e-5)
    -Velocity system
        -kFriction=0.5
    -A way to control "explosion" i.e. the fact that my algorithm performs badly on "64wheel" (it still performs badly)
        -explotionControl=2.0
-General attractive force
    -A better initial positioning of nodes (DID NOT WORK AS INTENDED, SO I REMOVED IT FROM THE CODE)
    -User interface
    -An idiot-proof input system

Extentions I did not do:
    -Penalty for crossing edges
    -Penalty for low resolution

Bad performance:
    -32wheel
    -64wheel
    -heawood
    -tesseract
    -tietze
*/

#include <iostream>
#include <cmath>
#include <fstream>
#include <windows.h>
#include <random>
#include "SimpleGraph.h"

using std::cin;
using std::cout;
using std::endl;
using std::mt19937;
using std::random_device;
using std::string;
using std::vector;

/*全图浏览系统*/
string MultiGraph[35] = {"2line", "3grid", "5clique", "5grid", "8wheel", "10clique", "10grid", "10line", "30clique",
                         "30cycle", "31binary-tree", "32wheel", "50line", "60cycle", "63binary-tree", "64wheel",
                         "127binary-tree", "bull", "cube", "desargues", "dodecahedron", "doodad-1", "doodad-2",
                         "doodad-3", "durer", "heawood", "icosahedron", "mobius-kantor", "moser-spindle", "octahedron",
                         "petersen", "star", "tesseract", "tietze", "triangle"};
/*以上为测试内容*/

/*
    随机数生成摘自https://zhuanlan.zhihu.com/p/442008589
    文件流：https://blog.csdn.net/cpp_learner/article/details/104180901
    随机数：https://zhuanlan.zhihu.com/p/442008589
*/
random_device seed;     // 硬件生成随机数种子
mt19937 engine(seed()); // 利用种子生成随机数引擎
/*end of random seed generation*/

void Welcome();
void Reboot(); // multigraph
int Inquiry();
void ReadGraph();
void Initialize(double nodeCnt);
void GraphReady();
void CalcRepel();
void CalcAttract();
void MoveNode();
bool IsStatic();
void ExertPertubation();
void Recession(double round);
double Dist(double x1, double y1, double x2, double y2);
void ExertForce(size_t id, double dx, double dy);
void Debug(size_t round);
// MyGraph:
struct ComplexNode
{
    double x, y;
    double dx, dy;
    double vx, vy;
    bool visible;
    double affiliated;
    double weight;
    vector<double> attraction_amplifier;
};
struct ComplexEdge
{
    size_t start, end;
};
struct ComplexGraph
{
    vector<ComplexNode> nodes;
    vector<ComplexEdge> edges;
};

// Critical Variables
double nodeCnt = 0, edgeCnt = 0;
ComplexGraph Graph;
SimpleGraph PaintBoard;

// Critical Consts
const double pi = 3.141592653589793;
const double e = 2.718281828459045;
double InitRadius = 1;
double kRepel = 3 * (1e-3);
double kAttract = 7 * (1e-3);
double kPertubation = 5 * (1e-3);
// double kPertubation=0.0;
double kRecession = 1; // parameter that recesses as iteration goes on
double kFriction = 0.5;
double minEnergy = 4 * (1e-5);
double minProtection = 1 * (1e-5); // minimal dist
string name = "64wheel";
bool MultiGraphEnabled = 1;
bool InquiryEnabled = 0;

// Main method
int main()
{
    Welcome();
    for (int k = 0; k < 35; k++)
    {
        if (MultiGraphEnabled)
            name = MultiGraph[k];
        Reboot();
        int totalRunningTime = 0;
        if (InquiryEnabled)
        {
            totalRunningTime = Inquiry();
            if (totalRunningTime == -1)
            {
                k--;
                continue;
            }
        }
        else
            totalRunningTime = 10;
        ReadGraph();
        Initialize(nodeCnt);
        bool IsPertubationDone = false;
        clock_t startTime = clock();
        for (int i = 0; i >= 0; i++)
        {
            double elapsedTime = double(clock() - startTime) / CLOCKS_PER_SEC;
            if (elapsedTime > totalRunningTime)
            {
                cout << "Time limit reached!" << endl;
                break;
            }
            Recession(i);
            if (i % 10 == 1)
                ExertPertubation();
            CalcRepel();
            CalcAttract();
            MoveNode();
            GraphReady();
            DrawGraph(PaintBoard);
            if (IsStatic())
            {
                if (IsPertubationDone)
                {
                    cout << "Done with " << i << " iterations!" << endl;
                    break;
                }
                else
                {
                    ExertPertubation();
                    IsPertubationDone = true;
                }
            }
        }
        Sleep(1000);
        if (!MultiGraphEnabled and InquiryEnabled)
        {
            cout << "continue? [y/n]" << endl;
            string isContinue;
            cin >> isContinue;
            if (isContinue == "n")
            {
                cin.clear();
                break;
            }
            cin.clear();
            system("cls");
        }
    }
    cout << "End of the program!" << endl;
    return 0;
}

/* Prints a message to the console welcoming the user and
 * describing the program. */
void Welcome()
{
    cout << "---This is a dummy GraphViz program designed by FHC---" << endl;
}

/*Prepare for another round of Graph visualization*/
void Reboot()
{
    Graph.nodes.clear();
    Graph.edges.clear();
    PaintBoard.nodes.clear();
    PaintBoard.edges.clear();
    nodeCnt = 0, edgeCnt = 0;
}

/*inquire for the name and the time the user wants*/
int Inquiry()
{
    bool isNameValid = false;
    string input_name;
    cout << "Which graph to visualize? (Input a string)" << endl;
    cin >> input_name;
    for (int k = 0; k < 35; k++)
    {
        if (MultiGraph[k] == input_name)
        {
            isNameValid = true;
            name = input_name;
            break;
        }
    }
    if (!isNameValid)
    {
        cout << "Invalid Graph Name!" << endl;
        return -1;
    }
    int input_time;
    cout << "How many seconds do you want to run the visualizer? (Input an integer)" << endl;
    if (cin >> input_time)
    {
        return input_time;
    }
    else
    {
        cout << "Invalid Running Time!" << endl;
        cin.clear();
        return -1;
    }
}

/*Read the information of the graph from the input file*/
void ReadGraph()
{
    cout << "Drawing Graph: " << name << endl;
    std::ifstream ifs;
    ifs.open(name, std::ios::in);
    if (!ifs.is_open())
    {
        cout << "Open failed!" << endl;
        return;
    }
    ifs >> nodeCnt;
    while (!ifs.eof())
    {
        size_t startGet, endGet;
        ifs >> startGet >> endGet;
        Graph.edges.push_back({startGet, endGet});
        edgeCnt++;
    }
    edgeCnt--;
    Graph.edges.pop_back(); // remove trash data
    ifs.close();
}

/*Pre-calculate all the information and parameter needed*/
void Initialize(double nodeCnt)
{
    double dist[(int)nodeCnt][(int)nodeCnt];
    size_t degreeCnt[(int)nodeCnt + 1];
    InitGraphVisualizer(PaintBoard);
    ComplexNode newNode = *new ComplexNode;
    for (int i = 0; i < nodeCnt; i++)
        Graph.nodes.push_back(newNode), Graph.nodes[i].affiliated = 0;
    for (int i = 0; i < nodeCnt; i++)
        for (int j = 0; j < nodeCnt; j++)
            dist[i][j] = nodeCnt;
    for (int i = 0; i < edgeCnt; i++)
    { // store the edges
        size_t from = Graph.edges[i].start, to = Graph.edges[i].end;
        Graph.nodes[from].affiliated++;
        Graph.nodes[to].affiliated++;
        dist[from][to] = 1;
        dist[to][from] = 1;
    }
    for (int i = 0; i < nodeCnt + 1; i++)
        degreeCnt[i] = 0;
    for (int i = 0; i < nodeCnt; i++)
        degreeCnt[(int)Graph.nodes[i].affiliated]++;
    vector<size_t> degreeSame[(int)nodeCnt + 1];
    for (int i = 0; i < nodeCnt; i++)
        degreeSame[(int)Graph.nodes[i].affiliated].push_back(i);
    /*  A better initial positioning of nodes - (Failed attempt)*/

//        double degreeVariety=0,deltaRadius=0,radius=0;
//        bool isFirstDegree=true;
//        for(int i=0;i<nodeCnt+1;i++){
//            if(degreeCnt[i]==0) continue;
//            degreeVariety++;
//            cout<<"degree="<<i<<':';
//            for(int j=0;j<degreeCnt[i];j++)
//                cout<<degreeSame[i][j]<<' ';
//            cout<<endl;
//        }
//        // cout<<degreeVariety<<endl;
//        deltaRadius=InitRadius/degreeVariety;
//        for(int i=nodeCnt;i>=0;i--){
//            if(degreeCnt[i]==0) continue;
//            if(isFirstDegree and degreeCnt[i]!=1) radius+=deltaRadius;
//            for(int j=0;j<degreeCnt[i];j++){
//                double init_x,init_y;
//                size_t id=degreeSame[i][j];
//                init_x=radius*cos(j*2*pi/degreeCnt[i]);
//                init_y=radius*sin(j*2*pi/degreeCnt[i]);
//                Graph.nodes[id].x=init_x;
//                Graph.nodes[id].y=init_y;
//                Graph.nodes[id].visible=true;
//                Graph.nodes[id].weight=(1.0+Graph.nodes[id].affiliated/(10.0));
//            }
//            isFirstDegree=false;
//            radius+=deltaRadius;
//        }

    for (int i = 0; i < nodeCnt; i++) // Generate the initial distribution of the nodes
    {
        double init_x, init_y;
        init_x = InitRadius * cos(i * 2 * pi / nodeCnt);
        init_y = InitRadius * sin(i * 2 * pi / nodeCnt);
        Graph.nodes[i].x = init_x;
        Graph.nodes[i].y = init_y;
        Graph.nodes[i].visible = true;
        Graph.nodes[i].weight = (1.0 + Graph.nodes[i].affiliated / (10.0));
    }
    for (int k = 0; k < nodeCnt; k++) // Floyd-Warshell: Calculate distance of the nodes on the graph
    {
        for (int i = 0; i < nodeCnt; i++)
        {
            for (int j = 0; j < nodeCnt; j++)
            {
                if (i == j)
                    continue;
                dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }

    for (int i = 0; i < nodeCnt; i++) // Implement the amplifier
    {
        for (int j = 0; j < nodeCnt; j++)
        {
            if (i == j)
                Graph.nodes[i].attraction_amplifier.push_back(0);
            else
                Graph.nodes[i].attraction_amplifier.push_back(pow(5.0, (-1.0) * (dist[i][j] - 1)));
        }
    }
}

/*push all the nodes and edges to the paintboard and get ready to display on the screen*/
void GraphReady()
{
    while (!PaintBoard.nodes.empty())
        PaintBoard.nodes.pop_back();
    while (!PaintBoard.edges.empty())
        PaintBoard.edges.pop_back();

    for (int i = 0; i < nodeCnt; i++)
    {
        if (Graph.nodes[i].visible)
        {
            PaintBoard.nodes.push_back({Graph.nodes[i].x, Graph.nodes[i].y});
        }
    }
    for (int i = 0; i < edgeCnt; i++)
    {
        PaintBoard.edges.push_back({Graph.edges[i].start, Graph.edges[i].end});
    }
}

/*Repel Forces*/
void CalcRepel()
{
    for (int i = 0; i < nodeCnt; i++)
    {
        double x0 = Graph.nodes[i].x, y0 = Graph.nodes[i].y;
        for (int j = i + 1; j < nodeCnt; j++)
        {
            double x1 = Graph.nodes[j].x, y1 = Graph.nodes[j].y;
            double FRepel = kRepel / Dist(x0, y0, x1, y1);
            double theta = atan2(y1 - y0, x1 - x0);
            ExertForce(i, (-1.0) * FRepel * cos(theta) * Graph.nodes[j].weight, (-1.0) * FRepel * sin(theta) * Graph.nodes[j].weight);
            ExertForce(j, (1.0) * FRepel * cos(theta) * Graph.nodes[i].weight, (1.0) * FRepel * sin(theta) * Graph.nodes[i].weight);
        }
    }
}

/*Attractive Forces*/
void CalcAttract()
{
    for (int i = 0; i < nodeCnt; i++)
    {
        for (int j = i + 1; j < nodeCnt; j++)
        {
            double kAmplifier = Graph.nodes[i].attraction_amplifier[j];
            double x0 = Graph.nodes[i].x, y0 = Graph.nodes[i].y;
            double x1 = Graph.nodes[j].x, y1 = Graph.nodes[j].y;
            double FAttract = kAmplifier * kAttract * Dist(x0, y0, x1, y1) * Dist(x0, y0, x1, y1);
            double theta = atan2(y1 - y0, x1 - x0);
            ExertForce(i, (1.0) * FAttract * cos(theta), (1.0) * FAttract * sin(theta));
            ExertForce(j, (-1.0) * FAttract * cos(theta), (-1.0) * FAttract * sin(theta));
        }
    }
}

/*Implement the difference of the velocity*/
void MoveNode()
{
    double explotionControl = 2.0;
    for (int i = 0; i < nodeCnt; i++)
    {
        Graph.nodes[i].vx += Graph.nodes[i].dx;
        Graph.nodes[i].vy += Graph.nodes[i].dy;
        double abs_vx = abs(Graph.nodes[i].vx), abs_vy = abs(Graph.nodes[i].vy);
        if (abs_vx > 5)
            Graph.nodes[i].vx = Graph.nodes[i].vx / abs_vx * explotionControl;
        if (abs_vy > 5)
            Graph.nodes[i].vy = Graph.nodes[i].vy / abs_vy * explotionControl;
    }
    double base_vx = Graph.nodes[0].vx, base_vy = Graph.nodes[0].vy;
    for (int i = 0; i < nodeCnt; i++)
    {
        Graph.nodes[i].vx -= base_vx;
        Graph.nodes[i].vy -= base_vy;
        Graph.nodes[i].x += Graph.nodes[i].vx;
        Graph.nodes[i].y += Graph.nodes[i].vy;
        Graph.nodes[i].dx = 0;
        Graph.nodes[i].dy = 0;
        Graph.nodes[i].vx *= kFriction;
        Graph.nodes[i].vy *= kFriction;
    }
}

/*Tool: Calculate the distance between (x1,y1) and (x2,y2)*/
double Dist(double x1, double y1, double x2, double y2)
{
    double returnDist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return returnDist > minProtection ? returnDist : minProtection;
}

/*Tool: Implement Force to the certain node*/
void ExertForce(size_t id, double dx, double dy)
{
    Graph.nodes[id].dx += dx * (kRecession + 1);
    Graph.nodes[id].dy += dy * (kRecession + 1);
}

/*Tool: Tell if the iteration is ready to stop*/
bool IsStatic()
{
    double OverallEnergy = 0;
    for (int i = 0; i < nodeCnt; i++)
    {
        OverallEnergy += sqrt(Graph.nodes[i].vx * Graph.nodes[i].vx + Graph.nodes[i].vy * Graph.nodes[i].vy);
    }
    if (OverallEnergy < nodeCnt * minEnergy)
        return true;
    else
        return false;
}

/*Tool: Exert Pertubation to all nodes*/
void ExertPertubation()
{
    /*随机数生成摘自https://zhuanlan.zhihu.com/p/442008589*/
    int min = 1, max = 36000;
    std::uniform_int_distribution<> distrib(min, max);
    for (int i = 0; i < nodeCnt; i++)
    {
        double randomAngle = distrib(engine) / (100.0);
        double randomMagnifier = distrib(engine) / (36000.0) + 0.5; // value of 0.5~1.5
        double FPertubation = kPertubation * randomMagnifier;
        ExertForce(i, FPertubation * cos(randomAngle) * kRecession, FPertubation * sin(randomAngle) * kRecession);
    }
    /*end of random seed generation*/
}

/*Tool: A recession function to calculate kRecession*/
void Recession(double round)
{
    const double startRecession = 4;
    const double limRound = 500.0;
    kRecession = startRecession * pow(e, (-1.0) * log(startRecession) * round / limRound);
}

/*Tool: Print the position(velocity) of all the nodes*/
void Debug(size_t round)
{
    cout << "round: " << round << '\n';
    for (int i = 0; i < nodeCnt; i++)
    {
        cout << "id=" << i << " dx=" << Graph.nodes[i].dx << " dy=" << Graph.nodes[i].dy << endl;
    }
}
