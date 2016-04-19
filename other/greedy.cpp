
int dijk(Graph& g,Demand& dm,stack<int>& s,int src){
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
		while(u!=src){
			s.push(u);
			u=path[u];
		}
		return cost[save];
	}
}

void greedy(Graph& g,Demand& dm){
	vector<int> path;
	int cost=INF,count=0;
	for(int *it=dm.begin();it!=dm.end();it++){
		int next=*it;
		int&src=dm.src;
		int i,cst,cnt=1;
		g.visit(src);
		vector<int> local;
		local.push_back(src);
		cst=dijkstra(g,dm,local,true,src,next);
		// cout<<"must:"<<next<<" "<<"cost:"<<cst<<endl;
		// g.reset();
		// continue;
		if(cst==-1)continue;
		while((i=dijkstra(g,dm,local,true,next))!=-1){
			cnt++;
			cst += i;
			next = local.back();
		}
		if(dm.count()==cnt){
			cst += dijkstra(g,dm,local,true,next,dm.dst);
			if(cst<cost){
				cost=cst;
				count=cnt;
				path.swap(local);
			}
		}else{
			if(cnt>count){
				count=cnt;
				path.swap(local);
			}
			if(cnt==count){				
				if(cst>cost){
					cost=cst;
					count=cnt;
					path.swap(local);
				}
			}
		}
		g.reset();
	}
	if(dm.count()!=count){
		// insert the uncontained must pass into the path
		//insert(g,dm,path);
		return;
	}
	for (int i = 0; i < (int)path.size()-1; i++){
		record_result(g.getEdge(path[i],path[i+1]));
	}
}
