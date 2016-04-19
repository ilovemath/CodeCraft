
#include "route.h"
#include "lib_record.h"
#include <stdio.h>
#include <string.h>
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
    int n,size;
	int src,dst;
    bool has[N];//必经点集中是否包含该点
	int node[N];
    void init(){
		n = 0;
		size = 0;
        memset(has,false,N*sizeof(bool));
    }
    void add(int i){
        if(!has[i]){
            node[size++]=i;
			n++;
            has[i]=true;
        }
    }
	void set(int i){
		if(!has[i]){
			n++;
			has[i]=true;
		}
	}
    void clear(int i){
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
				while(tmp){
					head[i]->next=tmp->next;
					free(tmp);
				    tmp = head[i]->next;
				}
				free(head[i]);
			}
		}
	}
	void addEdge(int edge,int src,int dst,int cost){
		Node *node = newNode(dst,edge,cost);
		if(head[src]==NULL){
			head[src] = newNode(src,0,0);
		}
		Node *tmp = head[src];
//		while(tmp->next && cost > tmp->next->cost)
		// if(!in.has[dst]){
			// //printf("%d\n",dst);
			// while(tmp->next && in.has[tmp->next->dest])
				// tmp=tmp->next;			
		// }
		node->next = tmp->next;
		tmp->next = node;
	}
	void print(){
        for(int i=0;i<N;i++){
			if(head[i]){
				printf("----Node %d-----------------------\n",i);
				Node *p=head[i]->next;
				while(p){
					printf("Edge: %d  Dest: %d  Cost: %d\n",p->edge,p->dest,p->cost);
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
	Path(int cst=0){
        n = 0;
        cost = cst;
		fill_n(has,N,false);
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

void backtrack(const Graph &g, int src, Include &in, Path &g_path, Path &path, bool &use_opt){

    if(src==in.dst){
        if(in.n==0 && g_path > path){
			g_path=path;
        }
        return; 
    }
	if(use_opt){
		//printf("%d\n",get_time());
		if(get_time()>8000)return;
		// int d = path.cost+cost[src][dest];
		// //printf("###src:%d\tdst:%d\t%d\n",src,dest,d);
		// if(d>INF||d>g_path.cost)return;
		// if(path.cost>200)return;
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
            in.clear(dst);
            backtrack(g,dst,in,g_path,path,use_opt);
            path.remove();
            path.unvisit(dst);
			path.dec(cst);
            if(flg)
                in.set(dst);
        }
    }
}


class Path2{
private:
	int n;
public:
    int cost;
	bool has[N];
	int edge[N];
	int node[N];
	Path2(int cst=0){
        n = 0;
        cost = cst;
		fill_n(has,N,false);
	}
	bool operator<(const Path2& other){
		return cost<other.cost;
	}
	bool operator>(const Path2& other){
		return cost>other.cost;
	}
	Path2& operator=(const Path2& other){
		if(this!=&other){
			n = other.n;
			cost = other.cost;
			memcpy(has,other.has,sizeof(has));
			memcpy(edge,other.edge,other.size()*sizeof(int));
			memcpy(node,other.node,other.size()*sizeof(int));
		}
		return *this;
	}
	Path2& operator+=(const Path2& other){
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
	Path2& operator-=(const Path2& other){	
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
		//if(has[v])return;//为了节省时间，省掉判断
		edge[n] = e;
		node[n++] = v;
		has[v] = true;
	}
	void pop(){
		//if(n<=0)return;//为了节省时间，省掉判断
		has[node[--n]]=false;
	}
	bool clash(const Path2& other){
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

void getAllPath(vector< vector<Path2> >& res,Path2& path,Graph& g,Include& in,int node){
	Node* tmp=g.head[node];
	if(tmp){
		while((tmp=tmp->next)!=NULL){
			int dst = tmp->dest;
			int cst = tmp->cost;
			if(path.has[dst])
				continue;
			path.icost(cst);
			path.push(dst,tmp->edge);
			if(in.has[dst]||dst==in.dst){
				res[dst].push_back(path);
			}else{
				getAllPath(res,path,g,in,dst);
			}
			path.dcost(cst);
			path.pop();
		}
	}
}

void getPaths(vector< vector<Path2> >& res,Path2& path,Graph& g,Include& in,int node){
	vector<vector<vector<Path2> > > m(N,vector<vector<Path2> >(N,vector<Path2> ()));
	for(int i=0;i<=in.src;i++){
		Path2 path;
		int &n=in.node[i];
		vector< vector<Path2> > res(N,vector<Path2>());
		getAllPath(res,path,g,in,n);
		m[n]=res;
	}
}

void traverse(vector<vector<vector<Path2> > >& res,Path2& path,Include& in,int src){
	if(src==in.dst){
		path.print();
		cout<<path.cost<<endl;
	}
	else if(in.n==0){
		vector<Path2>&paths=res[src][in.dst];
		if(!paths.empty()){
			for(unsigned int i=0;i<paths.size();i++){
				if(path.clash(paths[i]))continue;
				path+=paths[i];
				traverse(res,path,in,in.dst);
				path-=paths[i];
			}
		}	
	}else{
		for(int i=0;i<in.size;i++){
			int dst = in.node[i];
			if(!in.has[dst])continue;
			in.clear(dst);
			//cout<<src<<"->"<<dst<<",";
			vector<Path2>&paths=res[src][dst];
			if(!paths.empty()){
				for(unsigned int i=0;i<paths.size();i++){
					if(path.clash(paths[i]))continue;
					path+=paths[i];
					cout<<"\npath: ";path.print();
					cout<<"paths:";paths[i].print();
					traverse(res,path,in,dst);
					path-=paths[i];
				}
			}
			in.set(dst);
		}
	}
}

//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand)
{
	// bool use_opt = false;	
	// if(edge_num>100)
	// {
		// get_time();
		// use_opt=true;
	// }
	Graph g;
	int id,src,dst,cst;
	for(int i = 0; i < edge_num; i++){
		id = atoi(strtok(topo[i],","));
		src = atoi(strtok(NULL,","));
		dst = atoi(strtok(NULL,","));
		cst = atoi(strtok(NULL,","));
		g.addEdge(id,src,dst,cst);
	}
    Include in;
    in.init();
    in.src = atoi(strtok(demand,","));
    in.dst = atoi(strtok(NULL,","));
    char* inc = strtok(NULL,",");
    inc = strtok(inc,"|");
    in.add(atoi(inc));
    while((inc = strtok(NULL,"|"))!=NULL){
        in.add(atoi(inc));
    }
	in.node[in.size]=in.src;
	vector<vector<vector<Path2> > >m(N,vector<vector<Path2> >(N,vector<Path2> ()));
	for(int i=0;i<=in.src;i++){
		Path2 path;
		int &n=in.node[i];
		vector< vector<Path2> > res(N,vector<Path2>());
		getAllPath(res,path,g,in,n);
		m[n]=res;
	}
	cout<<"\n----------------------------------------"<<endl;
	Path2 path;
	traverse(m,path,in,in.src);
    // Path g_path(INF),path(0);
    //backtrack(out,include.src,include,g_path,path,use_opt);
    // for (int i = 0; i < g_path.n; i++)
        // record_result(g_path.edge[i]);
	
}

