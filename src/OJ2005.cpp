#include <bits/stdc++.h>
using namespace std;
int a,b,c,d;
map <int,int> fore,rear;
void SolveValue(){
    for(int i=0;i<=1000;i++){
        for(int j=0;j<=1000;j++){
            int value=a*i*i+b*j*j,times;
            if(i==0 and j==0) times=1;
            else if((i==0 and j!=0) or (i!=0 and j==0)) times=2;
            else times=4;
            fore[value]+=times;
            cout<<"value="<<value<<" times="<<times<<" input="<<rear[value]<<endl;
        }
    }
    for(int i=0;i<=1000;i++){
        for(int j=0;j<=1000;j++){
            int value=c*i*i+d*j*j,times;
            if(i==0 and j==0) times=1;
            else if((i==0 and j!=0) or (i!=0 and j==0)) times=2;
            else times=4;
            rear[value]+=times;
            cout<<"value="<<value<<" times="<<times<<" input="<<rear[value]<<endl;
        }
    }
}
long long CountSolution(){
    long long cnt=0;
    for(map<int,int>::iterator it=fore.begin();it!=fore.end();it++){
        cnt+=fore[it->first]*rear[(-1)*(it->first)];
    }
    return cnt;
}
int main(){
    cin>>a>>b>>c>>d;
    SolveValue();
    cout<<CountSolution()<<endl;
    return 0;
}