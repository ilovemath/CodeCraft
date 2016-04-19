#include "route.h"
#include "lib_record.h"
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

#define N 600
#define INF 10000

clock_t get_time(){
	static clock_t start=clock();
	return (clock()-start)/1000;
}

struct Node{
    int dest;
    int cost;
    int edge;
    Node* next;
};

Node* newNode(int dest,int edge,int cost){
	Node* node = new Node;
	node->dest = dest;
	node->edge = edge;
	node->cost = cost;
	node->next = NULL;
	return node;
}

class Demand{
	int n,nSize;
	int inSet[50];
public:
	int src,dst;
    bool has[N];// whether i is a must pass
	// fake must pass, supose a is a must pass,
	// if b is the only in-node of a, and c is 
	// the only in-node of b,then both b and c 
	// are fake must pass.
	// this variable must be processed by Graph::pre
	bool fake[N];
	Demand(){
		n=0;
		nSize=0;
		fill_n(has,N,false);
		fill_n(fake,N,false);
	}
	void read(char *demand){
		src = atoi(strtok(demand,","));
		dst = atoi(strtok(NULL,","));
		char* inc = strtok(NULL,",");
		inc = strtok(inc,"|");
		int ss = atoi(inc);
		add(ss);
		set(ss);		
		while((inc = strtok(NULL,"|"))!=NULL){
			ss = atoi(inc);
			add(ss);
			set(ss);
		}
	}
	void dec(){
		n--;
	}
	int count() const{
		return n;
	}
	int size() const{
		return nSize;
	}
	
	int* begin(){
		return inSet;
	}
	int* end(){
		return inSet+nSize;
	}
	void add(int i){
		inSet[nSize++]=i;
	}
	void set(int i){
		if(!has[i]){
			has[i]=true;
			n++;
		}
	}
    void clear(int i){
        if(has[i]){
            has[i]=false;
            n--;
        }
    }
	void reset(){
		for(int i=0;i<nSize;i++){
			has[inSet[i]]=true;
		}
	}
	void print(){
		cout<<"Demand:";
		for(int i=0;i<nSize;i++){
			cout<<inSet[i]<<" ";
		}cout<<endl;
	}
};

class Graph{
private:
	Demand *dm;
	int indeg[N];
	int outdeg[N];
	int innode[N];
public:
	int max;
	
    Node *head[N];
	bool visited[N];

	Graph(){
		max=0;
		reset();
		dm=NULL;
		memset(head,0,sizeof(head));
		memset(indeg,0,sizeof(indeg));
		memset(outdeg,0,sizeof(outdeg));
	}
	~Graph(){
        for(int i=0;i<N;i++){
			if(head[i]){
				Node*tmp=head[i]->next;
				while(tmp){
					head[i]->next=tmp->next;
					delete tmp;
				    tmp = head[i]->next;
				}
				delete head[i];
			}
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
	void addEdge(int edge,int src,int dst,int cost){
		Node *node = newNode(dst,edge,cost);
		max=src>max?src:max;
		max=dst>max?dst:max;
		if(head[src]==NULL){
			head[src] = newNode(src,-1,0);
		}
		Node *tmp = head[src];
		while(tmp->next){// remove duplicate edge
			if(tmp->next->dest==dst){
				if(tmp->next->cost>cost){
					tmp->next->cost=cost;
					tmp->next->edge=edge;
				}
				return;
			}
			tmp=tmp->next;
		}
		indeg[dst]++;
		outdeg[src]++;
		if(indeg[dst]==1)
			innode[dst]=src;
		//node->next = tmp->next;
		tmp->next = node;
	}
	int getEdge(int src,int dst){
		Node* tmp=head[src];
		if(!tmp)return -1;
		tmp=tmp->next;
		while(tmp&&tmp->dest!=dst){
			tmp=tmp->next;
		}
		if(!tmp)return -1;
		return tmp->edge;
	}
	int getCost(int src,int dst){
		Node* tmp=head[src];
		if(!tmp)return -1;
		tmp=tmp->next;
		while(tmp&&tmp->dest!=dst){
			tmp=tmp->next;
		}
		if(!tmp)return -1;
		return tmp->cost;		
	}
	void read(char **topo,int size,bool out=true){
		int id,src,dst,cst;
		for(int i = 0; i < size; i++){
			id = atoi(strtok(topo[i],","));
			src = atoi(strtok(NULL,","));
			dst = atoi(strtok(NULL,","));
			cst = atoi(strtok(NULL,","));
			if(out)
				addEdge(id,src,dst,cst);
			else
				addEdge(id,dst,src,cst);
		}
	}
	void write(const char *fname,Demand& dm){
		ofstream f(fname);
		f<<"data;"<<endl;
		f<<"param M := 30000;"<<endl;
		f<<"param n :="<<max<<";"<<endl;
		f<<"param s :="<<dm.src<<";"<<endl;
		f<<"param t :="<<dm.dst<<";"<<endl;
		f<<"set P :=";
		for(int* it=dm.begin();it!=dm.end();it++){
			f<<" "<<*it;
		}f<<";"<<endl;
		f<<"param : A : c :="<<endl;
		for(int i=0;i<=max;i++){
			if(head[i]){
				Node*tmp=head[i]->next;
				while(tmp){
					f<<i<<" "<<tmp->dest<<" "<<tmp->cost<<endl;
					tmp=tmp->next;
				}
			}
		}
		f<<";\nend;"<<endl;
	}
	
	void del(int src,bool &delm){//delete the node that have a zero in degree
		if(delm)return;
		if(dm&&dm->has[src]){
			delm=true;
			return;
		}
		if(!head[src])return;
		Node*tmp=head[src]->next;
		while(tmp){
			int dst=tmp->dest;
			head[src]->next=tmp->next;
			delete tmp;
			tmp = head[src]->next;
			//cout<<"del "<<src<<"->"<<dst<<endl;
			indeg[dst]--;
			outdeg[src]--;
			if(indeg[dst]==0){
				del(dst,delm);
			}				
		}
	}
	// exist solution:	return true;
	// no solution:		return false;
	bool prepare(Demand& d){
		dm=&d;
		bool ff=false;
		for(int i=0;i<N;i++){
			if(!head[i])continue;
			if(indeg[i]==0&&dm->src!=i){
				del(i,ff);
				if(ff){cout<<"no solution!"<<endl;return false;}
			}
		}
		for(int* i=dm->begin();i!=dm->end();i++){
			if(indeg[*i]==1){
				int m=*i;
				int &n=innode[m];
				while(indeg[n]==1){
					if(outdeg[n]!=1){
						Node*tmp=head[n];
						while(tmp->next){
							int dst=tmp->next->dest;
							if(dst!=m){
								Node*t=tmp->next;
								tmp->next=t->next;
								delete t;
								//cout<<"del "<<n<<"->"<<dst<<endl;
								outdeg[n]--;
								indeg[dst]--;
								if(indeg[dst]==0){
									bool ff=false;
									del(dst,ff);
									if(ff){cout<<"no solution!"<<endl;return false;}
								}
							}else{
								tmp=tmp->next;						
							}
						}
						if(outdeg[n]!=1)cout<<"outdeg error:"<<n<<" cur:"<<outdeg[n]<<endl;
					}
					m=n;
					n=innode[n];
				}			
			}
		}
		return true;
	}
	void printd(){
		for(int i=0;i<=max;i++)
		{
			if(head[i]){
				cout<<i<<"'s in  degree:"<<indeg[i]<<endl;
				cout<<i<<"'s out degree:"<<outdeg[i]<<endl;
			}
		}
	}
	void print(){
		cout<<"Graph:"<<endl;
        for(int i=0;i<N;i++){
			if(head[i]){
				cout<<i<<"->";
				Node *p=head[i];
				while((p=p->next)){
					cout<<p->dest<<"("<<p->edge<<","<<p->cost<<")";
					if(p->next)cout<<"->";
				}
				cout<<endl;
			}
		}
	}
};

void verify(Graph& g,Demand& dm,int* path,int size){
	cout<<"begin verifing..."<<endl;
	int cost=0,cnt=0;
	g.reset();
	if(path[size-1]!=dm.dst){
		cout<<"last node error!"<<endl;
		for(int i=0;i<size;i++)
			cout<<path[i]<<" ";
		cout<<endl;
		return;
	}
	for(int i=0;i<size-1;i++){
		int n=path[i];
		int m=path[i+1];
		int c=g.getCost(n,m);
		if(c==-1){
			cout<<"edge not exist:"<<n<<"->"<<m<<endl;
			return;
		}
		if(g.visited[n]){
			cout<<"repass node:"<<n<<endl;
			return;
		}
		cost+=c;
		g.visit(n);
		if(dm.has[n])
			cnt++;
	}
	if(dm.count()!=cnt){
		cout<<"not all must pass node is passed!"<<endl;
		cout<<"unpass node is:"<<endl;
		for(int i=0;i<N;i++){
			if(!g.visited[i]&&dm.has[i])cout<<i<<" ";
		}
		cout<<endl;
		return;
	}
	cout<<"cost:"<<cost<<endl<<"path:"<<endl;
	for(int i=0;i<size-1;i++){
		string c="";
		if(dm.has[path[i]]){c="*";}
		cout<<path[i]<<c<<"("<<g.getCost(path[i],path[i+1])<<")->";
	}
	cout<<path[size-1]<<endl<<"verifing succed!"<<endl;
	
}

class Path{
private:
	int n;
public:
    int cost;
	bool has[N];
	int node[N];
	int edge[N];
	Path(int cst=0){
        n = 0;
        cost = cst;
		fill_n(has,N,false);
	}
	bool operator<(const Path& other){
		return cost<other.cost;
	}
	bool operator>(const Path& other){
		return cost>other.cost;
	}
	void copy(const Path& other){
		n = other.n;
		cost = other.cost;
		memcpy(node,other.node,other.size()*sizeof(int));
		memcpy(edge,other.edge,other.size()*sizeof(int));		
	}
	Path& operator=(const Path& other){
		if(this!=&other){
			n = other.n;
			cost = other.cost;
			memcpy(has,other.has,sizeof(has));
			memcpy(edge,other.edge,other.size()*sizeof(int));
			memcpy(node,other.node,other.size()*sizeof(int));
		}
		return *this;
	}
	Path& operator+=(const Path& other){
		memcpy(edge+n,other.edge,other.size()*sizeof(int));
		memcpy(node+n,other.node,other.size()*sizeof(int));
		n+=other.size();
		cost+=other.cost;
		for(int i=0;i<N;i++){
			if(other.has[i])
				has[i]=true;
		}
		return *this;
	}
	Path& operator-=(const Path& other){	
		n-=other.size();
		cost-=other.cost;
		for(int i=0;i<N;i++){
			if(other.has[i])
				has[i]=false;
		}
		return *this;
	}
	int* back(){
		return node+n-1;
	}
	void icost(int cst){
		cost+=cst;
	}
	void dcost(int cst){
		cost-=cst;
	}
	int size() const{
		return n;
	}
	void push(int v,int e){
		edge[n] = e;
		node[n++] = v;
		has[v] = true;
	}
	void pop(){
		if(n<=0)return;
		has[node[--n]]=false;
	}
	bool clash(const Path& other){
		if(n==0)return false;
		if(this==&other)return true;
		for(int i=0;i<=other.size();i++){
			if(has[other.node[i]])return true;
		}
		return false;
	}
	void print(){
		cout<<"Path:";
		if(n==0)return;
		for(int i=0;i<n;i++){
			cout<<node[i]<<" ";
		}
		cout<<endl;
	}
};

struct node{
	int v,cost;
	node(int d,int c){v=d,cost=c;}
	bool operator<(const node&a)const{
		return cost>a.cost;
	}
};

//g:graph
//dm: demand
//pt: reverse shortest path excluded src 
//chg: whether change graph's states
//src: begin
//dst: end
//	if dst is not given,find the nearest must pass node
//	if dst is -2,find the nearest must pass node or destination

int dijkstra(Graph& g,Demand& dm,stack<int>&pt,bool chg, int src,int dst=-1){
	int n=g.max+1;
	int path[n];
	int cost[n];
	bool visit[n];
	fill_n(path,n,-1);
	fill_n(cost,n,INF);
	fill_n(visit,n,false);
	int save=src;
	priority_queue<node>q;
	q.push(node(src,0));
	while(!q.empty()){
		node min=q.top();q.pop();
		int& u=min.v;
		if(visit[u])
			continue;
		//termination: 
		//1. u is dst; 
		//2. u is destination
		//3. u is a must pass or and not visited
		if(u==dst||u==dm.dst||(!g.visited[u]&&dm.has[u])){
			save=u;
			break;
		}
		visit[u]=true;
		Node* tmp = g.head[u];
		while((tmp=tmp->next)){// for each u's child
			int& v=tmp->dest;
			if(v!=dst){// v is not dst
				// such points are skiped:
				if(g.visited[v])continue;	//skip visited
				if(v==dm.src)continue;  //skip source
				if(-2!=dst){//if dst is -2, both destination and must pass should not be skiped
					if(v==dm.dst)//skip destination
						continue;
					if(-1!=dst){//if dst is given, skip must pass point
						if(dm.has[v])
							continue;
					}
				}
			}
			int cst=min.cost+tmp->cost;
			if(cost[v]>cst){
				cost[v]=cst;
				path[v]=u;
				q.push(node(v,cst));
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
				pt.push(u);
				u=path[u];
			}
		}else{
			while(u!=src){
				pt.push(u);
				u=path[u];
			}
		}
		return cost[save];
	}
}

void backtrack(Graph &g, Demand &dm, Path &g_path, Path &path, int &src, int count){
    if(src==dm.dst){
        if(dm.count()==count)
		{
			if(g_path > path){
				g_path=path;
			}
		}
        return; 
    }
    if(g.head[src]){
        if(path > g_path)return;
        Node *tmp = g.head[src];
        while((tmp=tmp->next)!=NULL){
			int& dest = tmp->dest;
			int& cost = tmp->cost;
			int& edge = tmp->edge;
			if(dest==dm.src)continue;
            if(g.visited[dest])continue;
			g.visit(dest);
			path.icost(cost);
			path.push(dest,edge);
			backtrack(g,dm,g_path,path,dest,count+dm.has[dest]);
            path.pop();
			path.dcost(cost);
			g.clear(dest);
        }
    }
}

void back_track(Graph& g,Demand& dm){
	Path g_path(INF),path;
	backtrack(g,dm,g_path,path,dm.src,0);
	for (int i = 0; i < g_path.size(); i++)
		record_result(g_path.edge[i]);
	//verify(g,dm,g_path.node,g_path.size());
}

struct Path2{
	int cost;
	vector<int> path;
	void push(int v){
		path.push_back(v);
	}
	int pop(){
		int v=path.back();
		path.pop_back();
		return v;
	}
	int&back(){
		return path.back();
	}
	int&front(){
		return path.front();
	}
	int size() const{
		return path.size();
	}
	bool empty(){
		return path.empty();
	}
	Path2& operator=(const Path2& other){
		if(this!=&other){
			cost = other.cost;
			vector<int>tmp(other.path);
			path.swap(tmp);
		}
		return *this;
	}
	friend Path2 operator+(const Path2& p1, const Path2& p2){
		Path2 tmp;
		tmp.cost=p1.cost+p2.cost;
		for(int i=0;i<p1.size();i++){
			tmp.push(p1.path[i]);
		}
		for(int i=0;i<p2.size();i++){
			tmp.push(p2.path[i]);
		}
		return tmp;
	}
	bool operator<(const Path2& other){
		return cost<other.cost;
	}
	bool operator>(const Path2& other){
		return cost>other.cost;
	}
	void print(){
		cout<<"Path2:";
		for(unsigned int i=0;i<path.size();i++){
			cout<<path[i]<<" ";
		}
		cout<<endl;
	}
};

struct piece{
	int cost;// v's cost;
	vector<int>path;
	void push(int i){
		path.push_back(i);
	}
	int pop(){
		int i=path.back();
		path.pop_back();
		return i;
	}
	int size(){
		return path.size();
	}
	bool empty(){
		return path.empty();
	}
	int& back(){
		return path.back();
	}
	bool operator<(const piece&a)const{
		return cost>a.cost;
	}
};

void greedy_with_backtrack(Graph& g, Demand& dm, Path2& path, Path2& local, int count){

	if(local>path)
		return;
	if(get_time()>15000){
		 return;
	}
	int cost;
	//for local paths: (m is a must pass node)
	//src->n1->n2->...->m
	//	...
	//src->n1'->n2'->...->m'
	priority_queue<piece>q;
	//(cost,n1,n2...m)...(cost',n1',...m')
	stack<int>s;//m...n2,n1
	stack<int>v;//m...m'
	// find all possible m with dijkstra
	while((cost=dijkstra(g,dm,s,false,local.back()))!=-1){
		piece p;
		p.cost=cost;
		while(!s.empty()){
			p.push(s.top());s.pop();
		}//now p.back() is m
		// visit m to find another m
		g.visit(p.back());
		//save m to clear it afterwards
		v.push(p.back());
		q.push(p);
	}
	if(q.empty()){// none m is found
		// if(dm.count()==count+1){
			// for(int i=0;i<N;i++)
				// if(!g.visited[i]&&dm.has[i])cout<<i<<endl;
			// local.print();			
		// }
		if(dm.count()==count){// all m is contained
			stack<int>s;
			int cost=dijkstra(g,dm,s,false,local.back(),dm.dst);
			if(cost==-1)return;// no feasible path
			local.cost+=cost;
			// cout<<"cost:"<<local.cost<<endl;
			if(local<path){
				path=local;
				while(!s.empty()){
					path.push(s.top());
					s.pop();
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
		piece p=q.top();q.pop();
		for(int i=0;i<p.size();i++){
			int n=p.path[i];
			local.push(n);
			g.visit(n);
		}
		local.cost+=p.cost;
		greedy_with_backtrack(g,dm,path,local,count+1);
		local.cost-=p.cost;
		for(int i=0;i<p.size();i++){
			g.clear(local.pop());
		}
	}
}

int bfs(Graph& g, Demand& dm,stack<int>&pt,int src,bool chg=true){
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
		if(visit[u])
			continue;
		// termination:
		// u is a must pass point and not visited
		if(!g.visited[u]&&dm.has[u]){
			save=u;
			break;
		}
		visit[u]=true;
		Node* tmp = g.head[u];
		while((tmp=tmp->next)){// for each u's child
			int& v=tmp->dest;
			// such points are skiped:
			if(g.visited[v])continue;	//skip visited
			if(v==dm.src)continue;  //skip source
			if(v==dm.dst)continue;  //skip destination
			if(visit[v])continue;
			q.push(v);
			path[v]=u;
			cost[v]=cost[u]+tmp->cost;
		}
	}
	if(-1==path[save]){// no path
		return -1;
	}else{
		int u=save;
		//cout<<dm.has[u]<<endl;
		if(chg){
			while(u!=src){
				g.visit(u);
				pt.push(u);
				u=path[u];
			}
		}else{
			while(u!=src){
				pt.push(u);
				u=path[u];
			}
		}
		return cost[save];
	}
}

void greedy_with_bfs(Graph& g, Demand& dm, Path2& path){
	int cst,cnt=0;
	stack<int>s;
	Path2 local;
	local.cost=0;
	local.push(dm.src);
	while((cst=bfs(g,dm,s,local.back()))!=-1){
		cnt++;
		local.cost+=cst;
		while(!s.empty()){
			local.push(s.top());s.pop();
		}
	}
	if(dm.count()==cnt){
		cst=dijkstra(g,dm,s,false,path.back(),dm.dst);
		if(cst!=-1){
			local.cost+=cst;
			while(!s.empty()){
				local.push(s.top());s.pop();
			}
			if(local<path)path=local;
		}
	}
	g.reset();
}

int dfs(Graph& g, Demand& dm,Path2& path,Path2& local,int src,int depth){
	if(depth>=10)return -1;
	if(!g.visited[src]&&dm.has[src]){
		path=local;
		g.visit(src);
		return local.cost;
	}
	if(g.head[src]){
        Node *tmp = g.head[src];
        while((tmp=tmp->next)){
			int& dst = tmp->dest;
			int& cst = tmp->cost;
			if(dst==dm.src)continue;
			if(dst==dm.dst)continue;
            if(g.visited[dst])continue;
			if(!dm.has[dst])
				g.visit(dst);
			local.cost+=cst;
			local.push(dst);
			//cout<<"dst:"<<dst<<endl;
			int cost=dfs(g,dm,path,local,dst,depth+1);
			if(cost!=-1)return cost;
			//cout<<dst<<":"<<cost<<endl;
            local.pop();
			local.cost-=cst;
			g.clear(dst);
        }
    }
	return -1;
}

void greedy_with_dfs(Graph& g, Demand& dm, Path2& path){
	int cost=0,cst,cnt=0;
	Path2 tmp,local;
	tmp.push(dm.src);
	while((cst=dfs(g,dm,tmp,local,tmp.back(),0))!=-1){
		cnt++;
		cost+=cst;
		int*p=&tmp.front();
		for(int i=0;i<tmp.size();i++){
			path.push(p[i]);
		}
		local.path.clear();
		if(dm.count()==cnt)break;
	}
	if(dm.count()==cnt){
		stack<int>s;
		cst=dijkstra(g,dm,s,false,path.back(),dm.dst);
		if(cst!=-1){
			cost+=cst;
			if(cost<path.cost)
				while(!s.empty()){
					path.push(s.top());s.pop();
				}
		}else{
			cout<<"no solution!"<<endl;
		}
		
	}
	g.reset();
}

void greedy_with_part(Graph& g, Demand& dm, Path2& path){
	for(int *i=dm.begin();i!=dm.end();i++){
		int cst,m=*i;
		int count=0;
		g.visit(m);
		stack<int>s;
		Path2 back;
		back.cost=0;
		back.push(m);
		while((cst=dijkstra(g,dm,s,true,back.back(),-2))!=-1){
			count++;
			back.cost+=cst;
			while(!s.empty()){
				back.push(s.top());s.pop();
			}
			if(back.back()==dm.dst)break;
		}
		if(back.back()==dm.dst){
			Path2 front;
			front.cost=0;
			front.push(dm.src);
			while((cst=dijkstra(g,dm,s,true,front.back()))!=-1){
				count++;
				front.cost+=cst;
				while(!s.empty()){
					front.push(s.top());s.pop();
				}
			}
			if(dm.count()==count){
				cst=dijkstra(g,dm,s,false,front.back(),m);
				if(cst!=-1){
					front.cost+=cst;
					if(front.cost+back.cost<path.cost){						
						while(!s.empty()){
							if(s.top()==m)break;
							front.push(s.top());s.pop();
						}
						path=front+back;
					}
				 }//else{cout<<"in count: cst=-1"<<endl;}
			}//else{cout<<"dm.count()!=count"<<endl;}
		}//else{cout<<"back.back()!=dm.dst"<<endl;}
		g.reset();
	}
}

#define R() (rand()%3)
void greedy_with_mix(Graph& g, Demand& dm,Path2& path){
	int cst,cnt=0;
	Path2 local;
	local.cost=0;
	local.push(dm.src);
	stack<int>s;
	while(true){
		if(R()==0){
			cst=bfs(g,dm,s,local.back());
		}else{
			cst=dijkstra(g,dm,s,true,local.back());
		}
		if(cst==-1)break;
		cnt++;
		local.cost+=cst;
		while(!s.empty()){
			local.push(s.top());s.pop();
		}		
	}
	if(dm.count()==cnt){
		cst=dijkstra(g,dm,s,false,local.back(),dm.dst);
		if(cst!=-1){
			local.cost+=cst;
			while(!s.empty()){
				local.push(s.top());s.pop();
			}
			if(local<path)path=local;
		}else{
			local.cost=INF;
		}
	}else{
		path.cost=INF;
	}
	g.reset();
}

void greedy(Graph& g,Demand& dm){
	Path2 path,local;
	path.cost=INF;
	local.cost=0;
	path.push(dm.src);
	local.push(dm.src);	
	greedy_with_bfs(g,dm,path);
	greedy_with_part(g,dm,path);
	greedy_with_backtrack(g,dm,path,local,0);
	//verify(g,dm,&path.front(),path.size());
	for(int i=0;i<path.size()-1;i++)
		record_result(g.getEdge(path.path[i],path.path[i+1]));
}

void model(const char* fname){
	ofstream f(fname);
	f<<"param n,integer,>=0;"<<endl;
	f<<"param s,integer,>=0;"<<endl;
	f<<"param t,integer,>=0;"<<endl;
	f<<"param M,integer,>=0;"<<endl;

	f<<"set V := 0..n; "<<endl;
	f<<"set P within V;"<<endl;

	f<<"set A within V cross V;"<<endl;

	f<<"param c{(i, j) in A},integer,>=1,<=20;"<<endl;

	f<<"var x{(i, j) in A}, binary;"<<endl;
	f<<"var pi{i in V},>=0;"<<endl;

	f<<"minimize obj : sum {(u,v) in A} c[u,v]*x[u,v];"<<endl;

	f<<"s.t."<<endl;

	f<<"second1{v in V: v = s}: (sum{(i,v) in A} x[i,v]) - (sum{(v,j) in A} x[v,j])=-1; "<<endl;
	f<<"second2{v in V: v = t}: (sum{(i,v) in A} x[i,v]) - (sum{(v,j) in A} x[v,j]) = 1;"<<endl;
	f<<"second3{v in V: v <> s and v <> t}: (sum{(i,v) in A} x[i,v]) - (sum{(v,j) in A} x[v,j]) = 0; "<<endl;

	f<<"third{v in P}: sum{(u,v) in A} x[u,v]=1; "<<endl;
	f<<"thirteenth{(u,v) in A}: (pi[v] - pi[u])<=c[u,v]+M*(1-x[u,v]);"<<endl;
	f<<"fourteenth{(u,v) in A}: (pi[u] - pi[v])>=c[u,v]-M*(1-x[u,v]);"<<endl;
	f<<"#fifteenth: pi[s]=0; "<<endl;                                                     

	f<<"solve;"<<endl;
	//f<<"display  sum {(u,v) in A} c[u,v]*x[u,v];"<<endl;
	f<<"display {(i,j) in A} x[i,j];"<<endl;
	f<<"end;"<<endl;
	
}

void glpk(Graph& g,Demand& dm){
	g.write("data",dm);
	model("model");
	
	glp_tran *tran;
    glp_prob *mip;
    
    glp_smcp parm1;    
    glp_init_smcp(&parm1);
    parm1.meth = GLP_DUAL;

    glp_iocp parm2;
    glp_init_iocp(&parm2);
    parm2.br_tech = GLP_BR_PCH;
    parm2.bt_tech = GLP_BT_BPH;
    parm2.pp_tech = GLP_PP_NONE;
    parm2.fp_heur = GLP_ON;
    parm2.gmi_cuts = GLP_ON;
    parm2.mir_cuts = GLP_ON;


    //set output off
    glp_term_out(GLP_OFF);  // disable the output info
    //create probleme
    mip = glp_create_prob();
    tran = glp_mpl_alloc_wksp();  // allocates the MathProg translator workspace.

    //generate problem from file
    if (glp_mpl_read_model(tran, "model", 1) != 0 ||
        glp_mpl_read_data(tran, "data") != 0 ||  
        glp_mpl_generate(tran, NULL) != 0)

    {
        cout << "Error on translating model/data or generating mode" << endl;
        glp_mpl_free_wksp(tran);  // frees all the memory allocated to the translator workspace
        glp_delete_prob(mip);
		remove("data");
		remove("model");
        return;
    }

    glp_mpl_build_prob(tran, mip);  // obtains all necessary information from the translator workspace and stores it in the specified problem object prob

    glp_simplex(mip, &parm1);  // solve LP problem with the primal or dual simplex method

    glp_intopt(mip, &parm2);  // solve MIP problem with the branch-and-cut method

    if (glp_mpl_postsolve(tran, mip, GLP_MIP) != 0)   // copies the solution from the specified problem object prob to the translator workspace

         cout <<"Error on postsolving model" << endl;

	int pt[N];
	for (int i = 1; i <=  glp_get_num_cols(mip) ; i++)
    {
        string data = glp_get_col_name(mip, i);
        if(data[0] == 'x'&&glp_mip_col_val(mip, i)==1)
        {
			int n1 = atoi(data.substr(data.find('[') + 1, data.find(',')  - data.find('[') - 1).c_str());
			int n2 = atoi(data.substr(data.find(',') + 1, data.find(']')  - data.find(',') - 1).c_str());
			pt[n1]=n2;
        }
    }
	vector<int> path;
	int u=dm.src;
	while(u!=dm.dst){
		path.push_back(u);
		u=pt[u];
	}path.push_back(u);
	
	verify(g,dm,&path.front(),path.size());
	for(int i=0;i<(int)path.size()-1;i++)
		record_result(g.getEdge(path[i],path[i+1]));
    //free problem
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);
	remove("data");
	remove("model");
	
}

void findall(Graph& g,Demand& dm,int src){
	int cnt=0;
	stack<int>s;
	vector<int>path;
	g.visit(src);
	while(dijkstra(g,dm,s,true,src)!=-1){
		while(s.size()>1){
			s.pop();
		}cnt++;
		path.push_back(s.top());s.pop();
		for(int j=0;j<(int)path.size();j++){
			g.visit(path[j]);
		}
		g.visit(src);
	}
	cout<<src<<"("<<cnt<<"):";
	for(int i=0;i<(int)path.size();i++){
		cout<<path[i]<<" ";
	}cout<<endl;
	g.reset();
}


//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand)
{
	Demand dm;
	dm.read(demand);
	Graph g;
	g.read(topo,edge_num);
	if(!g.prepare(dm)){return;}
	// glpk(g,dm);
	 // return;
	if(edge_num<100){
		back_track(g,dm);
	}else{
		greedy(g,dm);
	}
}
