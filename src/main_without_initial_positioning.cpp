/*
Extensions:
    -Adjusted Parameters:
        -kRepel=
        -kAttract=
    -Random Pertubation:
        -a pertubation on every node every 5 rounds
    -Recessing Force
    -Stop iteration automatically once the graph is almost static
    -Velocity system
    -A way to control "explosion" i.e. the fact that my algorithm perform badly on "64wheel"
    -General attractive force
To-do:
    -Penalty for crossing edges
    -User interface
    -A better initial positioning of nodes
    -An idiot-proof input system
Bad performance:
    -doodads
    -50line
    -heawood
    -mobius-kantor
    -petersen
    -tietze
*/
#include <iostream>
#include <cmath>
#include <fstream>
#include <windows.h>
#include <random>
#include "SimpleGraph.h"

using std::cout;	using std::endl;    using std::vector;      using std::string;
using std::random_device;               using std::mt19937;

/*全图浏览系统*/
string MultiGraph[35]={"2line","3grid","5clique","5grid","8wheel","10clique","10grid","10line","30clique",
                    "30cycle","31binary-tree","32wheel","50line","60cycle","63binary-tree","64wheel",
                    "127binary-tree","bull","cube","desargues","dodecahedron","doodad-1","doodad-2",
                    "doodad-3","durer","heawood","icosahedron","mobius-kantor","moser-spindle","octahedron",
                    "petersen","star","tesseract","tietze","triangle"};
/*以上为测试内容*/

/*随机数生成摘自https://zhuanlan.zhihu.com/p/442008589*/
random_device seed;//硬件生成随机数种子
mt19937 engine(seed());//利用种子生成随机数引擎
/*end of random seed generation*/

void Welcome();
void Reboot();//multigraph
void ReadGraph();
void Initialize(double nodeCnt);
void GraphReady();
void CalcRepel();
void CalcAttract();
void MoveNode();
bool IsStatic();
void ExertPertubation();
void Recession(double round);
double Dist(double x1,double y1,double x2, double y2);
void ExertForce(size_t id, double dx, double dy);
void Debug(size_t round);
//MyGraph:
struct ComplexNode{
    double x,y;
    double dx,dy;
    double vx,vy;
    bool visible;
    vector<size_t> affiliated;
    vector<double> attraction_amplifier;
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
const double pi=3.141592653589793;
const double e=2.718281828459045;
double InitRadius=1;
double kRepel=3*(1e-3);
double kAttract=7*(1e-3);
double kPertubation=1*(1e-2);
//double kPertubation=0.0;
double kRecession=1;//parameter that recesses as iteration goes on
double kFriction=0.5;
double minEnergy=1*(1e-5);
double minProtection=1*(1e-5);//minimal dist
string name="10line";
bool MultiGraphEnabled=1;
bool SlowMotionEnabled=1;

// Main method
int main() {
    Welcome();
    for(int k=0;k<35;k++){
        if(MultiGraphEnabled) name=MultiGraph[k];
        Reboot();
        ReadGraph();
        Initialize(nodeCnt);
        bool IsPertubationDone=false;
        for(int i=0;i>=0;i++){
            if(!MultiGraphEnabled and SlowMotionEnabled) Sleep(10);
            Recession(i);
            if(i%5==1) ExertPertubation();
            CalcRepel();
            CalcAttract();
//            if(i%1000==0) cout<<"now at "<<i<<"th iteration!"<<endl;
            MoveNode();
            GraphReady();
            DrawGraph(PaintBoard);
            if( IsStatic()){
                if(IsPertubationDone){
                    cout<<"Done with "<<i<<" iterations!"<<endl;
                    break;
                }
                else{
                    ExertPertubation();
                    IsPertubationDone=true;
                }
            }
        }
        GraphReady();
        DrawGraph(PaintBoard);
        Sleep(1000);
    }
    
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
void Reboot(){
    Graph.nodes.clear();
    Graph.edges.clear();
    PaintBoard.nodes.clear();
    PaintBoard.edges.clear();
    nodeCnt=0,edgeCnt=0;
}
void ReadGraph(){
    cout<<"Drawing Graph: "<<name<<endl;
    std::ifstream ifs;
    ifs.open(name,std::ios::in);
    if(!ifs.is_open()){
        cout<<"Open failed!"<<endl;
        return;
    }
    ifs>>nodeCnt;
    while(!ifs.eof()){
        size_t startGet,endGet;
        ifs>>startGet>>endGet;
        Graph.edges.push_back({startGet,endGet});
        edgeCnt++;
    }
    edgeCnt--;
    Graph.edges.pop_back();//remove trash data
    ifs.close();
//    for(int i=0;i<edgeCnt;i++) cout<<Graph.edges[i].start<<' '<<Graph.edges[i].end<<'\n';
}
void Initialize(double nodeCnt){
    double dist[(int)nodeCnt][(int)nodeCnt];
    InitGraphVisualizer(PaintBoard);
    ComplexNode newNode= *new ComplexNode;
    for(int i=0;i<nodeCnt;i++) Graph.nodes.push_back(newNode);
    for(int i=0;i<nodeCnt;i++)
        for(int j=0;j<nodeCnt;j++)
            dist[i][j]=nodeCnt;
    for(int i=0;i<edgeCnt;i++){//store the edges
        size_t from=Graph.edges[i].start,to=Graph.edges[i].end;
        Graph.nodes[from].affiliated.push_back(to);
        Graph.nodes[to].affiliated.push_back(from);
        dist[from][to]=1;
        dist[to][from]=1;
    }
    for(int i=0;i<nodeCnt;i++){
        double init_x,init_y;
        init_x=InitRadius*cos(i*2*pi/nodeCnt);
        init_y=InitRadius*sin(i*2*pi/nodeCnt);
        Graph.nodes[i].x=init_x;
        Graph.nodes[i].y=init_y;
        Graph.nodes[i].visible=true;
    }
    
    for(int k=0;k<nodeCnt;k++){
        for(int i=0;i<nodeCnt;i++){
            for(int j=0;j<nodeCnt;j++){
                if(i==j) continue;
                dist[i][j]=std::min(dist[i][j],dist[i][k]+dist[k][j]);
            }
        }
    }
    
    for(int i=0;i<nodeCnt;i++){
        for(int j=0;j<nodeCnt;j++){
            if(i==j) Graph.nodes[i].attraction_amplifier.push_back(0);
            else Graph.nodes[i].attraction_amplifier.push_back(pow(5.0,(-1.0)*(dist[i][j]-1)));
        }
        // for(int j=0;j<nodeCnt;j++) cout<<Graph.nodes[i].attraction_amplifier[j]<<' ';
        // cout<<endl;
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
    for(int i=0;i<nodeCnt;i++){
        for(int j=i+1;j<nodeCnt;j++){
             double kAmplifier=Graph.nodes[i].attraction_amplifier[j];
//            double kAmplifier=1.0;
            double x0=Graph.nodes[i].x,y0=Graph.nodes[i].y;
            double x1=Graph.nodes[j].x,y1=Graph.nodes[j].y;
            double FAttract=kAmplifier*kAttract*Dist(x0,y0,x1,y1)*Dist(x0,y0,x1,y1);
            double theta=atan2(y1-y0,x1-x0);
            ExertForce(i,(1.0)*FAttract*cos(theta),(1.0)*FAttract*sin(theta));
            ExertForce(j,(-1.0)*FAttract*cos(theta),(-1.0)*FAttract*sin(theta));
        }
    }
}
void MoveNode(){
    double explotionControl=2.0;
    for(int i=0;i<nodeCnt;i++){
        Graph.nodes[i].vx+=Graph.nodes[i].dx;
        Graph.nodes[i].vy+=Graph.nodes[i].dy;
        double abs_vx=abs(Graph.nodes[i].vx),abs_vy=abs(Graph.nodes[i].vy);
        if(abs_vx>5) Graph.nodes[i].vx=Graph.nodes[i].vx/abs_vx*explotionControl;
        if(abs_vy>5) Graph.nodes[i].vy=Graph.nodes[i].vy/abs_vy*explotionControl;
        Graph.nodes[i].x+=Graph.nodes[i].vx;
        Graph.nodes[i].y+=Graph.nodes[i].vy;
        Graph.nodes[i].dx=0;
        Graph.nodes[i].dy=0;
        Graph.nodes[i].vx*=kFriction;
        Graph.nodes[i].vy*=kFriction;
    }
}
double Dist(double x1,double y1,double x2, double y2){
    double returnDist=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    return returnDist>minProtection?returnDist:minProtection;
}
void ExertForce(size_t id, double dx, double dy){
    Graph.nodes[id].dx+=dx*(kRecession+1);
    Graph.nodes[id].dy+=dy*(kRecession+1);
}
bool IsStatic(){
    double OverallEnergy=0;
    for(int i=0;i<nodeCnt;i++){
        OverallEnergy+=sqrt(Graph.nodes[i].vx*Graph.nodes[i].vx+Graph.nodes[i].vy*Graph.nodes[i].vy);
    }
    if(OverallEnergy<nodeCnt*minEnergy) return true;
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
        ExertForce(i,FPertubation*cos(randomAngle)*kRecession,FPertubation*sin(randomAngle)*kRecession);
    }
    /*end of random seed generation*/
}
void Recession(double round){
    const double startRecession=3;
    const double limRound=500.0;
    kRecession=startRecession*pow(e,(-1.0)*log(startRecession)*round/limRound);
}
void Debug(size_t round){
    cout<<"round: "<<round<<'\n';
    for(int i=0;i<nodeCnt;i++){
        cout<<"id="<<i<<" dx="<<Graph.nodes[i].dx<<" dy="<<Graph.nodes[i].dy<<endl;
    }
}
/*
文件流：https://blog.csdn.net/cpp_learner/article/details/104180901
随机数：https://zhuanlan.zhihu.com/p/442008589
*/
