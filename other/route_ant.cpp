#include "route.h"
#include "lib_record.h"

#include <map>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
using namespace std;

const int N=600;//最大节点数
const int INF=100000;//一个很大的数，代表无限
const int TIMELIMIT=8000;//退出时间

clock_t now(){
	static int start=clock();
	return (clock()-start)/1000;
}

class Demand{
public:
	int src,dst;
    bool has[N];//是否是必经点
	vector<int> in;//必经点集合
	Demand(){
		fill_n(has,N,false);
	}
	void Read(char *demand){
		src = atoi(strtok(demand,","));
		dst = atoi(strtok(NULL,","));
		char* inc = strtok(NULL,",");
		inc = strtok(inc,"|");
		int ss = atoi(inc);
		has[ss]=true;
		in.push_back(ss);
		while((inc = strtok(NULL,"|"))!=NULL){
			ss = atoi(inc);
			has[ss]=true;
			in.push_back(ss);
		}
	}
	int count() const{
		return in.size();
	}
	void Print(){
		cout<<"Demand:";
		for(auto &v:in){
			cout<<v<<" ";
		}cout<<endl;
	}
} dm;

struct Node{
	int edge;
	int cost;
	double prob;
	Node(){}
	Node(int e,int c){
		edge=e;
		cost=c;
		prob=1.0;
	}
};

class Graph{
public:
	Graph(void){
		max=0;
		reset();
	}
public:
	int max;
	bool visited[N];
	map<int,Node>head[N];
public:
	void Read(char** topo,int size){
		int id,src,dst,cst;
		for(int i = 0; i < size; i++){
			id = atoi(strtok(topo[i],","));
			src = atoi(strtok(NULL,","));
			dst = atoi(strtok(NULL,","));
			cst = atoi(strtok(NULL,","));
			auto &h=head[src];
			if(h.find(dst)!=h.end()){
				if(cst>h[dst].cost)
					continue;
				h[dst].edge=id;
				h[dst].cost=cst;
			}else{
				if(dm.dst==src)continue;
				if(dm.src==dst)continue;
				if(src>max)max=src;
				if(dst>max)max=dst;
				Node n(id,cst);
				head[src][dst]=n;
			}
		}
	}
	void Print(){
		for(int i=0;i<N;i++){
			if(head[i].empty())
				continue;
			cout<<i<<": ";
			for(auto &v:head[i]){
				cout<<v.first<<"("<<v.second.edge<<","<<v.second.cost<<") ";
			}
			cout<<endl;
		}
	}
	void visit(int i){
		visited[i]=true;
	}
	void clear(int i){
		visited[i]=false;
	}
	void reset(){
		fill_n(visited,N,false);		
	}
} g;

class Path{
public:
	Path(){}
	Path(int cst){
		cost=cst;
	}
public:
	int cost;
	vector<int>path;
	void reset(){
		cost=0;
		path.clear();
	}
	void push(int i){
		path.push_back(i);
	}
	void pop(){
		path.pop_back();
	}
	int& back(){
		return path.back();
	}
	int size() const{
		return path.size();
	}
	bool empty() const{
		return path.empty();
	}
	int operator[](unsigned int i){
		if(i>=path.size())
			return -1;
		else
			return path[i];
	}
	bool operator<(const Path& other){
		return cost<other.cost;
	}
	bool operator>(const Path& other){
		return cost>other.cost;
	}
	Path& operator=(Path& other){
		if(this!=&other){
			cost=other.cost;
			path=other.path;
		}
		return *this;
	}
	void Print(){
		if(path.empty()){
			cout<<"No Path Found!"<<endl;
		}else{
			for(auto&v:path){
				string c="";
				if(dm.has[v])c="*";
				if(v!=path.back())
					cout<<v<<c<<"->";
			}
			int &v=path.back();
			string c="";
			if(dm.has[v])c="*";
			cout<<v<<c<<endl;
		}
	}
	void Output(){
		if(path.empty()){
			cout<<"No Path Found!"<<endl;
		}else{
			for(auto it=path.begin();it!=path.end()-1;it++){
				record_result(g.head[*it][*(it+1)].edge);
			}
		}
	}
	void Verify(){
		cout<<"begin verify..."<<endl;
		bool wrong=false;
		int cost=0,cnt=0;
		int visit[N];
		fill_n(visit,N,0);
		if(path.empty()){
			cout<<"No path found!"<<endl;
			return;
		}
		if(path.back()!=dm.dst){
			cout<<"last node wrong"<<endl<<"current path is:";
			for(auto n:path){
				string c="";
				if(dm.has[n])c="*";
				if(n!=path.back())
					cout<<n<<c<<"->";
			}cout<<path.back()<<endl;
			wrong=true;
		}
		for(unsigned int i=1;i<path.size();i++){
			int &src=path[i-1];
			int &dst=path[i];
			auto &head=g.head[src];
			if(head.find(dst)==head.end()){
				cout<<"edge not exist:"<<src<<"->"<<dst<<endl;
				wrong=true;
			}
			cost+=head[dst].cost;
			if(visit[src]){
				cout<<"repass node:"<<src<<endl;
				wrong=true;
			}
			visit[src]=1;
			if(dm.has[src])
				cnt++;
		}
		if(dm.has[path.back()])cnt++;
		if(dm.count()!=cnt){
			cout<<"missing must pass node"<<endl;
			cout<<"total:"<<dm.count()<<endl;
			cout<<"unpass:"<<endl;
			for(int i=0;i<N;i++){
				if(!visit[i]&&dm.has[i])cout<<i<<" ";
			}
			cout<<endl;
			wrong=true;
		}
		if(wrong){
			cout<<"verify failed!"<<endl;
			return;
		}
		cout<<"best cost:"<<cost<<endl<<"shortest path:"<<endl;
		for(unsigned int i=1;i<path.size();i++){
			int &src=path[i-1];
			int &dst=path[i];
			string c="";
			if(dm.has[src]){c="*";}
			cout<<src<<c<<"("<<g.head[src][dst].cost<<")->";
		}
		cout<<path.back()<<endl<<"verify succeed!"<<endl;
	}

};

void backtrack(Path& best,Path& local,int count){
	if(now()>9000)return;
	if(local.empty()){
		local.push(dm.src);
	}
	if(local>best)return;
	int src=local.back();
	if(src==dm.dst){
		if(dm.count()==count){
			if(local<best){
				best=local;
			}
		}
	}else if(!g.head[src].empty()){
		for(auto &v:g.head[src]){
			int dst=v.first;
			int cst=v.second.cost;
			if(dst==dm.src)
				continue;
			if(g.visited[dst])
				continue;
			g.visit(dst);
			local.push(dst);
			local.cost+=cst;
			backtrack(best,local,count+dm.has[dst]);
			local.cost-=cst;
			local.pop();
			g.clear(dst);
		}
	}
}

struct heapnode{
	int v,cost;
	heapnode(int d,int c){v=d,cost=c;}
	bool operator<(const heapnode&a)const{
		return cost>a.cost;
	}
};

int dijkstra(stack<int>&s,bool chg,int src,int dst=-1){
	int n=g.max+1;
	int path[n];
	int cost[n];
	bool visit[n];
	fill_n(path,n,-1);
	fill_n(cost,n,INF);
	fill_n(visit,n,false);
	int save=src;
	priority_queue<heapnode>q;
	q.push(heapnode(src,0));
	while(!q.empty()){
		heapnode min=q.top();q.pop();
		int& u=min.v;
		if(visit[u])
			continue;
		//termination: 
		//1. u is dst; 
		//2. u is non-first destination or source
		//3. u is an unvisited must pass point
		if((u==dst)||(!q.empty()&&u==dm.dst)
			||(!q.empty()&&u==dm.src)
			||(!g.visited[u]&&dm.has[u])){
			save=u;
			break;
		}
		visit[u]=true;
		auto&head=g.head[u];
		if(head.empty())continue;
		for(auto& e:head){
			int v=e.first;
			if(v!=dst){// v is not dst
				// such points are skiped:
				if(g.visited[v])continue;	//skip visited
				if(v==dm.src)continue;  	//skip source
				if(v==dm.dst)continue;		//skip destination					
				if(-1!=dst){//if dst is given, skip must pass point
					if(dm.has[v])
						continue;
				}
			}
			int cst=min.cost+e.second.cost;
			if(cost[v]>cst){
				cost[v]=cst;
				path[v]=u;
				q.push(heapnode(v,cst));
			}
		}
	}
	if(-1==path[save]){// no path
		return -1;
	}else{		
		int u=save;
		if(chg){
			while(u!=src){
				g.visit(u);
				s.push(u);
				u=path[u];
			}
		}else{
			while(u!=src){
				s.push(u);
				u=path[u];
			}
		}
		return cost[save];
	}
}

int bfs(stack<int>&s,bool chg,int src){
	int n=g.max+1;
	int path[n];
	int cost[n];
	bool visit[n];
	fill_n(path,n,-1);
	fill_n(cost,n,INF);
	fill_n(visit,n,false);
	int save=src;
	queue<int>q;
	q.push(src);
	cost[src]=0;
	while(!q.empty()){
		int u=q.front();q.pop();
		if(visit[u])continue;
		// termination:
		// u is a must pass point and not visited
		if(!g.visited[u]&&dm.has[u]){
			save=u;
			break;
		}
		visit[u]=true;
		auto&head=g.head[u];
		if(head.empty())continue;
		for(auto& e:head){
			int v=e.first;
			// such points are skiped:
			if(g.visited[v])continue;	//skip visited
			if(v==dm.src)continue;  //skip source
			if(v==dm.dst)continue;  //skip destination
			if(visit[v])continue;
			q.push(v);
			path[v]=u;
			cost[v]=cost[u]+e.second.cost;
		}
	}
	if(-1==path[save]){// no path
		return -1;
	}else{		
		int u=save;
		if(chg){
			while(u!=src){
				g.visit(u);
				s.push(u);
				u=path[u];
			}
		}else{
			while(u!=src){
				s.push(u);
				u=path[u];
			}
		}
		return cost[save];
	}
}

void greedy(Path& best,Path& local,int count){
	if(now()>TIMELIMIT)return;
	if(local>best)return;
	if(local.empty()){
		local.push(dm.src);
	}
	int cost;
	queue<Path>q;
	stack<int>s;
	stack<int>v;
	while((cost=dijkstra(s,false,local.back()))!=-1){
		Path p(cost);
		while(!s.empty()){
			p.push(s.top());s.pop();
		}
		g.visit(p.back());
		//save m to clear it afterwards
		v.push(p.back());
		q.push(p);
	}
	if(q.empty()){// none m is found
		if(dm.count()==count){// all m is contained
			stack<int>s;
			int cost=dijkstra(s,false,local.back(),dm.dst);
			if(cost==-1){return;}// no feasible path
			local.cost+=cost;
			if(local<best){
				best=local;
				while(!s.empty()){
					best.push(s.top());s.pop();
				}
			}
			local.cost-=cost;
			return;
		}
		return;// no feasible path
	}
	while(!v.empty()){// unvisit ms
		g.clear(v.top());v.pop();
	}
	while(!q.empty()){
		Path p=q.front();q.pop();
		for(auto&v:p.path){
			local.push(v);
			g.visit(v);
		}
		local.cost+=p.cost;
		greedy(best,local,count+1);
		local.cost-=p.cost;
		for(int i=0;i<p.size();i++){
			g.clear(local.back());local.pop();
		}
	}
}

void greedy1(Path& best){
	int cst,cnt=0;
	stack<int>s;
	Path local(0);
	local.push(dm.src);
	while((cst=dijkstra(s,true,local.back()))!=-1){
		cnt++;
		local.cost+=cst;
		while(!s.empty()){
			local.push(s.top());s.pop();
		}
	}
	if(dm.count()==cnt){
		cst=dijkstra(s,false,local.back(),dm.dst);
		if(cst!=-1){
			local.cost+=cst;
			while(!s.empty()){
				local.push(s.top());s.pop();
			}
			if(local<best)best=local;
		}
	}
	g.reset();
}

void greedy2(Path& best){
	int cst,cnt=0;
	stack<int>s;
	Path local(0);
	local.push(dm.src);
	while((cst=bfs(s,true,local.back()))!=-1){
		cnt++;
		local.cost+=cst;
		while(!s.empty()){
			local.push(s.top());s.pop();
		}
	}
	if(dm.count()==cnt){
		cst=dijkstra(s,false,local.back(),dm.dst);
		if(cst!=-1){
			local.cost+=cst;
			while(!s.empty()){
				local.push(s.top());s.pop();
			}
			if(local<best)best=local;
		}
	}
	g.reset();
}

void greedy3(Path& best){
	int cst,cnt=0;
	stack<int>s;
	Path local(0);
	local.push(dm.src);
	g.visit(dm.src);
	while(true){
		if(rand()%10>5)
			cst=bfs(s,true,local.back());
		else{
			cst=dijkstra(s,true,local.back());			
		}
		if(cst==-1)break;
		cnt++;
		local.cost+=cst;
		while(!s.empty()){
			local.push(s.top());s.pop();
		}
	}
	if(dm.count()==cnt){
		cst=dijkstra(s,false,local.back(),dm.dst);
		if(cst!=-1){
			local.cost+=cst;
			while(!s.empty()){
				local.push(s.top());s.pop();
			}
			if(local<best)best=local;
		}
	}
	g.reset();
}


//蚂蚁算法参数
const double ALPHA=1.5;	//启发因子，信息素重要程度
const double BETA=0.5; 	//期望因子，节点花费重要程度
const double ROU=0.6;  	//信息素挥发因子(这里表示残余量)

const int ANT_COUNT=30;//蚂蚁数量


class Ant{	
	double rnd(double x){
		return x*rand()/(RAND_MAX+1.0);
	}

private:
	int cur;//当前所在节点
	bool stop;//搜索是否结束
public:
	Ant(void){}
	~Ant(void){}
public:
	Path path;//蚂蚁走的路径
	int mustpass;//包含的必经节点
	
public:
	void Init(){
		stop=false;
		mustpass=0;
		cur = dm.src;
		g.reset();
		g.visit(cur);
		path.reset();
		path.push(cur);
	}
	
	int Next(){
		if(g.head[cur].empty())
			return -1;//无路可走
		double total=0;//信息素总和
		int allow[8];//蚂蚁当前可走路径
		double prob[8];//蚂蚁当前可走路径的概率
		fill_n(prob,8,0);
		fill_n(allow,8,-1);
		int cnt=0;
		for(auto &v:g.head[cur]){
			allow[cnt]=v.first;
			prob[cnt]=pow(v.second.prob,ALPHA)*pow(1.0/v.second.cost,BETA);
			total+=prob[cnt++];
		}
		int selected=-1;
		if(total>0){//轮盘赌选择
			double tmp=rnd(total);
			for(int i=0;i<cnt;i++){
				tmp-=prob[i];
				if(tmp<0&&!g.visited[allow[i]]){
					selected=allow[i];
					break;
				}
			}
		}
		if(selected==-1){//防止信息素过小产生误差
			for(int i=0;i<cnt;i++){
				if(!g.visited[allow[i]]){
					selected=allow[i];
					break;
				}
			}
		}
		return selected;
	}
	
	void Move(){
		int next = Next();
		if(next==-1||next==dm.dst||next==dm.src){
			stop=true;
			return;
		}
		path.cost+=g.head[cur][next].cost;
		cur = next;
		path.push(cur);
		g.visit(cur);
		if(dm.has[cur]){
			mustpass++;
			if(mustpass==dm.count()){
				stop=true;
				stack<int>s;
				int cst=dijkstra(s,true,path.back(),dm.dst);
				if(cst!=-1){
					path.cost+=cst;
					while(!s.empty()){
						path.push(s.top());s.pop();
					}
				}
				else{
					cout<<"no path to dest!"<<endl;
				}
			}
		}
	}
	
	void Search(){
		Init();
		while(!stop){
			Move();
		};
	}
	
	bool Success(){
		return (path.back()==dm.dst)&&(mustpass==dm.count());
	}
};

class Problem{
private:
	int cost;//某蚂蚁所求路径权值
	int count;//蚂蚁所求路径权值连续相同次数
public:
	Problem(void){
		count=0;
		best.path.cost=INF;		
	}
	~Problem(void){}
public:
	Ant best;
	Ant ants[ANT_COUNT];
public:
	void UpdateTrial(){//更新环境信息素
		//信息素挥发
		for(int i=0;i<N;i++){
			auto &head=g.head[i];
			if(head.empty())continue;
			for(auto &e:head){
				e.second.prob*=ROU;
			}
		}
		
		//信息素更新
		//只更新包含必经点最多的
		int id=-1;
		int nm=0;
		for(int i=0;i<ANT_COUNT;i++){
			Ant& ant=ants[i];
			if(ant.mustpass>nm){
				id=i;nm=ant.mustpass;
			}
		}
		if(id==-1)return;
		Ant& ant=ants[id];
		for(int j=1;j<ant.path.size();j++){
			int src=ant.path[j-1];
			int dst=ant.path[j];			
			g.head[src][dst].prob += 10.0 / ant.path.cost;
		}
	}
	
	bool canStop(){//如果蚂蚁0连续20次权值相同,则认为算法已收敛
		if(cost==ants[0].path.cost){
			count++;
		}else{
			count=0;
			cost=ants[0].path.cost;
		}
		if(count>20)return true;
		else return false;
	}
	
	void Search(){
		//迭代至时间耗尽
		while(now()<5000){
			//每只蚂蚁搜索一遍
			for(int j=0;j<ANT_COUNT;j++){
				ants[j].Search();
				// ants[j].path.Print();
				// cout<<ants[j].mustpass<<endl;
			}
			//保存最佳结果
			for(int j=0;j<ANT_COUNT;j++){
				if(ants[j].Success()&&ants[j].path<best.path){
					best=ants[j];
				}
			}
			UpdateTrial();
			if(canStop())break;
		}
		g.reset();
	}
	
	void Result(Path& path){
		if(best.path<path){
			path=best.path;
		}
	}

};



//你要完成的功能总入口
void search_route(char *topo[5000], int num, char *demand){
	srand(time(0));
	dm.Read(demand);
	g.Read(topo,num);
	Path local(0),best(INF);
	Problem m;
	m.Search();
	m.Result(best);
	if(num<100)
		backtrack(best,local,0);
	else{
		greedy1(best);
		greedy2(best);
		greedy3(best);
		greedy(best,local,0);
	}
	best.Output();
}
