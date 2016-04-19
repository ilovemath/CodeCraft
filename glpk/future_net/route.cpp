#include "route.h"
#include "glpk.h"
#include "lib_record.h"
#include <map>
#include <list>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define N 600
#define INF 10000

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
	Node(){}
	Node(int e,int c){
		edge=e;
		cost=c;
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
	auto begin(){
		return path.begin();
	}
	auto end(){
		return path.end();
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

class Solution{
private:
    glp_prob *mip;
	vector<Path>paths;
	
	int weight;
	int nodes;
private:	
	inline bool HasSubCycle(){return paths.size()>1;}
	inline int CountCycle(){return paths.size();}
public:
	void Init(){
		depth=0;
		best.weight=INF;
		nodes=0;
	}
	
	void data(const char *fname){
		ofstream f(fname);
		f<<"data;"<<endl;
		f<<"param n :="<<g.max<<";"<<endl;
		f<<"param s :="<<dm.src<<";"<<endl;
		f<<"param t :="<<dm.dst<<";"<<endl;
		f<<"set P :=";
		for(auto &v:dm.in){
			f<<" "<<v;
		}f<<";"<<endl;
		f<<"param : A : c :="<<endl;
		for(int i=0;i<=g.max;i++){
			if(g.head[i].empty())continue;
			for(auto&v:g.head[i]){
				f<<i<<""<<v.first<<" "<<v.second.cost<<endl;
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
		//f<<"display {(i,j) in A} x[i,j];"<<endl;
		f<<"end;"<<endl;
		
	}
	
	void Output(){
		
	}
	void Process(){
		weight=0;
		map<int,int>pt;
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
		int n=0;
		while(!pt.empty()){
			paths.resize(n+1);
			int s=(*pt.begin()).first;
			int n=(*pt.begin()).second;
			pt.erase(pt.begin);
			while(s!=n&&!pt.empty()){
				auto it=pt.find(n);
				if(it==pt.end()){
					cout<<"error!"<<endl;
					exit(1);
				}
				int c=(*it).first;
				n=(*it).second;
				pt.erase(it);
				paths[n].push(c);
			}
			n++;
		}
		weight=glp_get_row_prim(mip,glp_get_num_rows(mip));
	}
	
	void Compute(){
		const char*dat="data";
		const char*mod="model";
		glp_tran *tran;
		
		// disable the output info
		glp_term_out(GLP_OFF);  
		//create probleme
		mip = glp_create_prob();
		tran = glp_mpl_alloc_wksp();

		model(mod);
		data(dat,g,dm);		
		//generate problem from file
		if (glp_mpl_read_model(tran, mod, 1) != 0 ||
			glp_mpl_read_data(tran, dat) != 0 ||  
			glp_mpl_generate(tran, NULL) != 0){
			cout << "Error on translating model/data or generating mode" << endl;
			glp_mpl_free_wksp(tran);
			glp_delete_prob(mip);
			remove(mod);
			remove(dat);
			return;
		}

		glp_mpl_build_prob(tran, mip);

		glp_smcp parm1;    
		glp_init_smcp(&parm1);
		parm1.meth = GLP_DUAL;		
		glp_simplex(mip, &parm1);

		glp_iocp parm2;
		glp_init_iocp(&parm2);
		parm2.br_tech = GLP_BR_PCH;
		parm2.bt_tech = GLP_BT_BPH;
		parm2.pp_tech = GLP_PP_NONE;
		parm2.fp_heur = GLP_ON;
		parm2.gmi_cuts = GLP_ON;
		parm2.mir_cuts = GLP_ON;
		glp_intopt(mip, &parm2);
		if (glp_mpl_postsolve(tran, mip, GLP_MIP) != 0) 
			cout <<"Error on postsolving model" << endl;
		
		
		//free problem
		glp_mpl_free_wksp(tran);
		glp_delete_prob(mip);
		remove(dat);
		remove(mod);
	}
	bool Iterate(){
		bool suc=false;
		auto pt=paths.begin();
		int size=(*it).size();
		for(auto it=paths.begin()+1;it!=paths.end();it++){
			if((*it).size()<size){
				size=(*it).size();
				pt=it;
			}
		}
		
		//On parcourt les sommets
		for(auto it = (*pt).begin(); it != (*pt).end(); )
		{
			++nodes;

			int next;
			int start = (*it);
			++it;
			if(it == (*pt).end())
			{
				next = *(*pt).begin();
			}
			else
			{
				next = (*it);
			}
			ostringstream out;
			out << "x[" << start << "," << next << "]";

			glp_create_index(mip);
			int col = glp_find_col(mip, out.str().c_str());
			glp_set_col_bnds(mip, col, GLP_FX, 0, 0);
			glp_simplex(mip);
			Solution sol;
			sol.Process();
			if(sol.CountCycle() == 1)
			{
				sol.CheckSolution(graph);
				success = true;
			}


			if(glp_get_status(mip) == GLP_OPT //has solution
					&& sol.HasSubCycle())//if has sub cycle
				if(sol.Iterate(mip)
					success = true;

			glp_create_index(mip);
			col = glp_find_col(mip, out.str().c_str());
			glp_set_col_bnds(mip, col, GLP_LO, 0, 0);

		}


		return success;
		
	}
	void CheckSolution(){
		if(weight < best.weight)
		{
			best.weight = weight;
			best.paths.resize(1);
			best.paths[0].clear();
			for(list<int>::iterator it = paths[0].begin(); it != paths[0].end(); ++it)
				best.paths[0].push_back((*it));
		}
	}
};


void glpk(Map& g,Demand& dm){
	
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
