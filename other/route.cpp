#include "route.h"
#include "lib_record.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <time.h>
#include <sys/timeb.h>
using namespace std;

#define N 600
#define INF 10000

unsigned long get_time(){
	struct timeb now;
	ftime(&now);
	static int ms = now.millitm;
    static unsigned long s = now.time;

	int out_ms = now.millitm - ms;
	unsigned long out_s = now.time - s;

	if (out_ms < 0)
	{
		out_ms += 1000;
		out_s -= 1;
	}
	return out_ms+out_s*1000;
	
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
	int n;
public:
	int src,dst;
    bool in[N];//必经点集中是否包含该点
	Demand(){
		n=0;
		fill_n(in,N,false);
	}
	void read(char *demand){
		src = atoi(strtok(demand,","));
		dst = atoi(strtok(NULL,","));
		char* inc = strtok(NULL,",");
		inc = strtok(inc,"|");
		set(atoi(inc));
		while((inc = strtok(NULL,"|"))!=NULL){
			set(atoi(inc));
		}
	}
	int count() const{
		return n;
	} 
	void set(int i){
		if(!in[i]){
			n++;
			in[i]=true;
		}
	}
    void clear(int i){
        if(in[i]){
            n--;
            in[i]=false;
        }
    }
};

class Graph{
public:
	int max;
	bool visit[N];
    Node *head[N];

	Graph(){
		max=0;
		init();
		memset(head,0,sizeof(head));
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
	void init(){
		fill_n(visit,N,false);
	}
	void addEdge(int edge,int src,int dst,int cost){
		Node *node = newNode(dst,edge,cost);
		if(head[src]==NULL){
			head[src] = newNode(src,-1,0);
		}
		max=src>max?src:max;
		max=dst>max?dst:max;
		Node *tmp = head[src];
		node->next = tmp->next;
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
	void print(){
        for(int i=0;i<N;i++){
			if(head[i]){
				cout<<"----Node"<<i<<"-----------------------\n";
				Node *p=head[i]->next;
				while(p){
					cout<<"Node: "<<p->edge<<"\tDest: "<<p->dest<<"\tCost: "<<p->cost<<endl;
					p=p->next;
				}
			}
		}
	}
};

class Path{
private:
	int n;
public:
    int cost;
	bool has[N];
	int edge[N];
	int node[N];
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
		if(has[v])return;
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
		if(n==0)return;
		for(int i=0;i<n;i++){
			cout<<node[i]<<" ";
		}
		cout<<endl;
	}
};

struct heap{
	int v,cost;
	heap(int d,int c){v=d,cost=c;}
	bool operator<(const heap&a)const{
		return cost<a.cost;
	}
};

void dijkstra(Graph& g,int src,int dst){
	int n=g.max+1;
	int path[n];
	int cost[n];
	fill_n(path,n,-1);
	fill_n(cost,n,INF);
	cost[src]=0;
	priority_queue<heap>q;
	q.push(heap(src,0));
	while(!q.empty()){
		heap min=q.top();q.pop();
		Node* tmp = g.head[min.v];
		while((tmp=tmp->next)){
			int& v=tmp->dest;
			int d=min.cost+tmp->cost;
			if(cost[v]>d){
				cost[v]=d;
				path[v]=min.v;
				q.push(heap(v,d));
			}
		}
	}
	int u=src;
	cout<<cost[u]<<endl;
	while(u!=src){
		cout<<u<<" ";
		u=path[u];
	}
	cout<<endl;
}


//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand)
{
	Graph g;
	g.read(topo,edge_num);
	Demand dm;
	dm.read(demand);
	int n=dm.src;
	cout<<dijkstra(g,dm,n)<<endl;

	// for (int i = 0; i < g_path.n; i++)
        // record_result(g_path.edge[i]);
}
