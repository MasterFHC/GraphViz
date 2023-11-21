#include <iostream>
#include <cmath>
#include <fstream>
#include <windows.h>
#include <random>
#include "SimpleGraph.h"

using std::cout;	using std::endl;    using std::vector;
using std::random_device;               using std::mt19937;

/*随机数生成摘自https://zhuanlan.zhihu.com/p/442008589*/
random_device seed;//硬件生成随机数种子
mt19937 engine(seed());//利用种子生成随机数引擎
/*end of random seed generation*/

void Welcome();
void ReadGraph();
void Initialize();
void GraphReady();
void CalcRepel();
void CalcAttract();
void MoveNode();
bool IsStatic();
void ExertPertubation();
double Dist(double x1,double y1,double x2, double y2);
void ExertForce(size_t id, double dx, double dy);
void Debug(size_t round);
//MyGraph:
struct ComplexNode{
    double x,y;
    double dx,dy;
    bool visible;
    vector<size_t> affiliated;
};
struct ComplexEdge{
    size_t start,end;
};
struct ComplexGraph{
    vector<ComplexNode> nodes;
    vector<ComplexEdge> edges;
};

//Critical Variables
double nodeCnt=0,edgeCnt=0;
ComplexGraph Graph;
SimpleGraph PaintBoard;

//Critical Consts
double pi=3.141592653589793;
double InitRadius=1;
double kRepel=3*(1e-3);
double kAttract=10*(1e-3);
double kPertubation=10*(1e-2);
double minMovement=1*(1e-5);
double minProtection=1*(1e-5);//minimal dist
char *name=(char*)"64wheel";

// Main method
int main() {
    Welcome();
    ReadGraph();
    Initialize();
    bool IsPertubationDone=false;
    for(int i=0;i>=0;i++){
        CalcRepel();
        CalcAttract();
        if(IsStatic()){
            if(IsPertubationDone){
                cout<<"Done with "<<i<<" iterations!\n";
                break;
            }
            else{
                ExertPertubation();
                IsPertubationDone=true;
            }
        }
//        Sleep(30);
        MoveNode();
        GraphReady();
        DrawGraph(PaintBoard);
    }
    GraphReady();
    DrawGraph(PaintBoard);
    Debug(1);
    return 0;
}

/* Prints a message to the console welcoming the user and
 * describing the program. */
void Welcome() {
    cout << "Welcome to CS106L GraphViz!" << endl;
    cout << "This program uses a force-directed graph layout algorithm" << endl;
    cout << "to render sleek, snazzy pictures of various graphs." << endl;
    cout << "Entered ReadGraph?" << endl;
    return;
}
void ReadGraph(){
    cout<<"Entered ReadGraph\n";
    std::ifstream ifs;
    ifs.open(name,std::ios::in);
    if(!ifs.is_open()){
        cout<<"Open failed!\n";
        return;
    }
    ifs>>nodeCnt;
    while(!ifs.eof()){
        std::size_t startGet,endGet;
        ifs>>startGet>>endGet;
        Graph.edges.push_back({startGet,endGet});
        edgeCnt++;
    }
    edgeCnt--;
    Graph.edges.pop_back();//remove trash data
    ifs.close();
    for(int i=0;i<edgeCnt;i++) cout<<Graph.edges[i].start<<' '<<Graph.edges[i].end<<'\n';
}
void Initialize(){
    InitGraphVisualizer(PaintBoard);
    ComplexNode newNode= *new ComplexNode;
    for(int i=0;i<nodeCnt;i++) Graph.nodes.push_back(newNode);
    for(int i=0;i<edgeCnt;i++){//store the edges
        size_t from=Graph.edges[i].start,to=Graph.edges[i].end;
        Graph.nodes[from].affiliated.push_back(to);
        Graph.nodes[to].affiliated.push_back(from);
    }
    for(int i=0;i<nodeCnt;i++){
        double init_x,init_y;
        init_x=InitRadius*cos(i*2*pi/nodeCnt);
        init_y=InitRadius*sin(i*2*pi/nodeCnt);
        Graph.nodes[i].x=init_x;
        Graph.nodes[i].y=init_y;
        Graph.nodes[i].visible=true;
    }
}
void GraphReady(){
    while(!PaintBoard.nodes.empty()) PaintBoard.nodes.pop_back();
    while(!PaintBoard.edges.empty()) PaintBoard.edges.pop_back();

    for(int i=0;i<nodeCnt;i++){
        if(Graph.nodes[i].visible){
            PaintBoard.nodes.push_back({Graph.nodes[i].x,Graph.nodes[i].y});
        }
    }
    for(int i=0;i<edgeCnt;i++){
        PaintBoard.edges.push_back({Graph.edges[i].start,Graph.edges[i].end});
    }
}
void CalcRepel(){
    for(int i=0;i<nodeCnt;i++){
        double x0=Graph.nodes[i].x,y0=Graph.nodes[i].y;
        for(int j=i+1;j<nodeCnt;j++){
            double x1=Graph.nodes[j].x,y1=Graph.nodes[j].y;
            double FRepel=kRepel/Dist(x0,y0,x1,y1);
            double theta=atan2(y1-y0,x1-x0);
         ExertForce(i,(-1.0)*FRepel*cos(theta),(-1.0)*FRepel*sin(theta));
         ExertForce(j,(1.0)*FRepel*cos(theta),(1.0)*FRepel*sin(theta));
        }
    }
}
void CalcAttract(){
    for(int i=0;i<edgeCnt;i++){
        size_t node0=Graph.edges[i].start,node1=Graph.edges[i].end;
        double x0=Graph.nodes[node0].x,y0=Graph.nodes[node0].y;
        double x1=Graph.nodes[node1].x,y1=Graph.nodes[node1].y;
        double FAttract=kAttract*Dist(x0,y0,x1,y1)*Dist(x0,y0,x1,y1);
        double theta=atan2(y1-y0,x1-x0);
        ExertForce(node0,(1.0)*FAttract*cos(theta),(1.0)*FAttract*sin(theta));
        ExertForce(node1,(-1.0)*FAttract*cos(theta),(-1.0)*FAttract*sin(theta));
    }
}
void MoveNode(){
    for(int i=0;i<nodeCnt;i++){
        Graph.nodes[i].x+=Graph.nodes[i].dx;
        Graph.nodes[i].y+=Graph.nodes[i].dy;
        Graph.nodes[i].dx=0;
        Graph.nodes[i].dy=0;
    }
}
double Dist(double x1,double y1,double x2, double y2){
    return std::max(minProtection,sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)));
}
void ExertForce(size_t id, double dx, double dy){
    Graph.nodes[id].dx+=dx;
    Graph.nodes[id].dy+=dy;
}
bool IsStatic(){
    double OverallMovement=0;
    for(int i=0;i<nodeCnt;i++){
        OverallMovement+=Dist(0,0,Graph.nodes[i].dx,Graph.nodes[i].dy);
    }
    if(OverallMovement<nodeCnt*minMovement) return true;
    else return false;
}
void ExertPertubation(){
    /*随机数生成摘自https://zhuanlan.zhihu.com/p/442008589*/
    int min = 1,max = 36000;
    std::uniform_int_distribution<> distrib(min, max);
    for(int i=0;i<nodeCnt;i++){
        double randomAngle = distrib(engine)/(100.0);
        double randomMagnifier = distrib(engine)/(36000.0)+0.5;//value of 0.5~1.5
        double FPertubation=kPertubation*randomMagnifier;
        ExertForce(i,FPertubation*cos(randomAngle),FPertubation*sin(randomAngle));
    }
    

    /*end of random seed generation*/
}
void Debug(size_t round){
    cout<<"round: "<<round<<'\n';
    for(int i=0;i<nodeCnt;i++){
        cout<<"id="<<i<<" x="<<Graph.nodes[i].x<<" y="<<Graph.nodes[i].y<<'\n';
    }
}
/*
文件流：https://blog.csdn.net/cpp_learner/article/details/104180901
随机数：https://zhuanlan.zhihu.com/p/442008589
*/
