#include "route.h"
#include "glpk.h"
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
	// this variable must be processed by Map::pre
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

class Map{
private:
	Demand *dm;
	int indeg[N];
	int outdeg[N];
	int innode[N];
public:
	int max;
	
    Node *head[N];
	bool visited[N];

	Map(){
		max=0;
		reset();
		dm=NULL;
		memset(head,0,sizeof(head));
		memset(indeg,0,sizeof(indeg));
		memset(outdeg,0,sizeof(outdeg));
	}
	~Map(){
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
		cout<<"Map:"<<endl;
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

void verify(Map& g,Demand& dm,int* path,int size){
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

void data(const char *fname,Map& g,Demand& dm){
	ofstream f(fname);
	f<<"data;"<<endl;
	f<<"param n :="<<g.max<<";"<<endl;
	f<<"param s :="<<dm.src<<";"<<endl;
	f<<"param t :="<<dm.dst<<";"<<endl;
	f<<"set P :=";
	for(int* it=dm.begin();it!=dm.end();it++){
		f<<" "<<*it;
	}f<<";"<<endl;
	f<<"param : A : c :="<<endl;
	for(int i=0;i<=g.max;i++){
		if(g.head[i]){
			Node*tmp=g.head[i]->next;
			while(tmp){
				f<<i<<" "<<tmp->dest<<" "<<tmp->cost<<endl;
				tmp=tmp->next;
			}
		}
	}
	f<<";\nend;"<<endl;
}

void model(const char* fname){
	ofstream f(fname);
	f<<"param n,integer,>=0;"<<endl;
	f<<"param s,integer,>=0;"<<endl;
	f<<"param t,integer,>=0;"<<endl;

	f<<"set V := 0..n; "<<endl;
	f<<"set P within V;"<<endl;

	f<<"set A within V cross V;"<<endl;

	f<<"param c{(i, j) in A},integer,>=1,<=20;"<<endl;

	f<<"var x{(i, j) in A}, binary;"<<endl;
	f<<"var pi{i in V},>=0;"<<endl;

	f<<"minimize obj : sum {(u,v) in A} c[u,v]*x[u,v];"<<endl;

	f<<"s.t."<<endl;

	f<<"second1: (sum{(i,s) in A} x[i,s]) - (sum{(s,j) in A} x[s,j])=-1; "<<endl;
	f<<"second2: (sum{(i,t) in A} x[i,t]) - (sum{(t,j) in A} x[t,j]) = 1;"<<endl;
	f<<"second3{v in V: v <> s and v <> t}: (sum{(i,v) in A} x[i,v]) - (sum{(v,j) in A} x[v,j]) = 0; "<<endl;

	f<<"third{v in P}: sum{(u,v) in A} x[u,v]=1; "<<endl;
	
	f<<"solve;"<<endl;
	//f<<"display  sum {(u,v) in A} c[u,v]*x[u,v];"<<endl;
	f<<"display {(i,j) in A} x[i,j];"<<endl;
	f<<"end;"<<endl;
	
}

void glpk(Map& g,Demand& dm){
	const char*dat="data";
	const char*mod="model";
	model(mod);
	data(dat,g,dm);
	
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
    if (glp_mpl_read_model(tran, mod, 1) != 0 ||
        glp_mpl_read_data(tran, dat) != 0 ||  
        glp_mpl_generate(tran, NULL) != 0)

    {
        cout << "Error on translating model/data or generating mode" << endl;
        glp_mpl_free_wksp(tran);  // frees all the memory allocated to the translator workspace
        glp_delete_prob(mip);
		remove(mod);
		remove(dat);
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
	
	//verify(g,dm,&path.front(),path.size());
	for(int i=0;i<(int)path.size()-1;i++)
		record_result(g.getEdge(path[i],path[i+1]));
    //free problem
    glp_mpl_free_wksp(tran);
    glp_delete_prob(mip);
	remove(dat);
	remove(mod);
	
}

//你要完成的功能总入口
void search_route(char *topo[5000], int edge_num, char *demand)
{
	Demand dm;
	dm.read(demand);
	Map g;
	g.read(topo,edge_num);
	if(!g.prepare(dm)){return;}
	glpk(g,dm);	
}
