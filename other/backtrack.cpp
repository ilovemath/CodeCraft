#include <stdio.h>
#include <string.h>
#include "lib_record.h"
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <iostream>
#include <vector>
using namespace std;

#define N 600
#define INF 10000

struct Node{
    int dest;
    int cost;
    int edge;
    Node* next;
};

Node* newNode(int dest,int edge,int cost){
	Node* node = (Node*) malloc(sizeof(Node));
	node->dest = dest;
	node->edge = edge;
	node->cost = cost;
	node->next = NULL;
	return node;	
}


struct Include{//必经点
    int n;
    bool has[N];//必经点集中是否包含该点
    void init(){
		n=0;
        memset(has,false,N*sizeof(bool));
    }
    void add(int i){
        if(!has[i]){
            n++;
            has[i]=true;
        }
    }
    void remove(int i){
        if(has[i]){
            n--;
            has[i]=false;
        }
    }
};


class Graph{
public:
    Node *head[N];

	Graph(){
		memset(head,0,sizeof(head));	
	}
	~Graph(){
        for(int i=0;i<N;i++){
			if(head[i]){			
				Node*tmp=head[i]->next;
				free(head[i]);
				while(tmp){
					head[i]->next=tmp->next;
					free(tmp);
				    tmp = head[i]->next;
				}
			}	
		}	
	}
	void addEdge(int edge,int src,int dst,int cost){
		Node *node = newNode(dst,edge,cost);
		if(head[src]==NULL){
			head[src] = newNode(src,0,0);
		}
		Node *tmp = head[src];
		node->next = tmp->next;
		tmp->next = node;
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

class Path{//路径
public:
    int n;
    int cost;
    int edge[N];
	bool has[N];
	Path(int cst){
        n = 0;
        cost = cst;
		memset(has,false,sizeof(has));
	}
	bool operator<(const Path &other){
		return cost<other.cost;
	}
	bool operator>(const Path &other){
		return cost>other.cost;
	}
	Path& operator=(const Path &other){
        n = other.n;
		cost = other.cost;
        memcpy(edge,other.edge,other.n*sizeof(int));
		return *this;
	}
	void inc(int cst){
		cost+=cst;
	}
	void dec(int cst){
		cost-=cst;
	}
	void visit(int i){
		has[i]=true;
	}
	void unvisit(int i){
		has[i]=false;		
	}
    void add(int i){
        edge[n++]=i;
    }
    void remove(){
		if(n>0){
			n--;
		}
    }
};


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


void backtrack(const Graph &g, const int &src, const int &dest, Include &in, Path &g_path, Path &path, bool &use_opt){

    if(src==dest){
        if(in.n==0 && g_path > path){
			g_path=path;
        }
        return; 
    }
	if(use_opt){
		if(get_time()>8000)return;
	}
    if(g.head[src]){
        if(g_path < path)
            return;
        Node *tmp = g.head[src];
        while((tmp=tmp->next)!=NULL){
            int dst = tmp->dest;
			int cst = tmp->cost;
            if(path.has[dst])
                continue;
			path.inc(cst);
            path.visit(dst);
            path.add(tmp->edge);
            bool flg = in.has[dst];
            in.remove(dst);
            backtrack(g,dst,dest,in,g_path,path,use_opt);
            path.remove();
            path.unvisit(dst);
			path.dec(cst);
            if(flg)
                in.add(dst);
        }
    }
}

//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand)
{
	bool use_opt = false;	
	if(edge_num>100)
	{
		get_time();
		use_opt=true;
	}
	Graph g;
	int id,s,d,c;
	for(int i = 0; i < edge_num; i++){
		id = atoi(strtok(topo[i],","));
		s = atoi(strtok(NULL,","));
		d = atoi(strtok(NULL,","));
		c = atoi(strtok(NULL,","));
		g.addEdge(id,s,d,c);
	}
	char* inc;
    int src,dst;
    Include include;
    include.init();
    src = atoi(strtok(demand,","));
    dst = atoi(strtok(NULL,","));
    inc = strtok(NULL,",");
    inc = strtok(inc,"|");
    include.add(atoi(inc));
    while((inc = strtok(NULL,"|"))!=NULL){
        include.add(atoi(inc));
    }
    Path g_path(INF),path(0);
    backtrack(g,src,dst,include,g_path,path,use_opt);
    for (int i = 0; i < g_path.n; i++)
        record_result(g_path.edge[i]);
	//printf("%d\n",g_path.cost);
	cout<<g_path.cost<<endl;
}

