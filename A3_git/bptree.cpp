#include <bits/stdc++.h>
using namespace std;

#define ll long long
#define ORDER 3

struct BPNode{
	vector< ll > key;
	vector< BPNode* > child_ptr;
	ll n;
	BPNode *buffer;			//buffer stores new node which is returned from lower nodes on splitting
	BPNode *parent;
	bool isleaf;
	vector< ll > count;		//stored only at leaves
};

BPNode *root = NULL;			//ROOT of tree

////////////////////////////////////////////////////////////////////////////////////

vector< string > split(string line)
{
	vector< string > res;
	string str = "";
	for(ll i = 0; i < line.size(); i++)
	{
		if(line[i] != ' ')
			str += line[i];
		else
		{
			res.push_back(str);
			str = "";
		}
	}
	res.push_back(str);
	return res;
}

////////////////////////////////////////////////////////////////////////////////////
//create new node
BPNode* initialize()
{
	BPNode *new_node = new BPNode;
	new_node->key = vector< ll > (ORDER - 1, LONG_LONG_MAX);
	new_node->child_ptr = vector< BPNode* > (ORDER, NULL);
	new_node->parent = NULL;
	new_node->isleaf = false;
	new_node->n = 0;
	new_node->count = vector< ll > (ORDER - 1, 1);
	return new_node;
}

////////////////////////////////////////////////////////////////////////////////////
// find block to insert into
BPNode* findBlock(BPNode *root, ll num)
{
	if(root)
	{	
		if(root->isleaf == false)		//non leaf node threrfore recurse
		{
			ll lower_b = LONG_LONG_MIN , i = 0;
			while(i < ORDER )
			{			
				ll upper_b = root->key[i];
				if(i == 2)
					upper_b = LONG_LONG_MAX;
				//cout<<lower_b<<" "<<upper_b<<"\n";
				//getchar();
				if(root->key[i] == LONG_LONG_MAX)
				{
					root = root->child_ptr[i];
					break;
				}
				if(num >= lower_b && num <= upper_b)
				{
					root = root->child_ptr[i];
					break;
				}
				lower_b = root->key[i];

				i++;
			}
			return findBlock(root, num);
		}
		else 
			return root;
	}

	return NULL;	//block not found
}

////////////////////////////////////////////////////////////////////////////////////

BPNode* insert(BPNode *node, ll num)
{
	if(node->key[ORDER - 2] == LONG_LONG_MAX)		//free space in current node
	{
		if(num > node->key[0])		//num smaller than smallest key
		{
			node->key[1] = num;
			if(!(node->isleaf))
			{
				node->child_ptr[2] = node->buffer;
			}
		}
		else if(num == node->key[0])		//key already exists so just increment count
		{
			node->count[0] += 1;
		}	
		else		
		{
			node->key[1] = node->key[0];	//move first key to 2nd position.
			node->count[1] = node->count[0];	//move first count to 2nd position

			node->key[0] = num;
			node->count[0] = 1;

			if(!(node->isleaf))
			{
				node->child_ptr[2] = node->child_ptr[1];
				node->child_ptr[1] = node->buffer;
			}			
		}
	}
	else												//current node is full
	{
		BPNode *new_node = initialize();
		new_node->parent = node->parent;
		
		if(node->isleaf)		//leaf node was split
		{
			if(num == node->key[0])			//insert value is equal to first key in node
			{
				node->count[0] += 1;
				return root;
			}
			if(num == node->key[1])			//insert value is equal to second key in node
			{
				node->count[1] += 1;
				return root;
			}


			//current node is full and num is not already present	

			vector< pair<ll,ll> >temp;				//store <key , count>
			temp.push_back({node->key[0] , node->count[0]});
			temp.push_back({node->key[1] , node->count[1]});
			temp.push_back({num , 1});
			sort(temp.begin(), temp.end());

			if(node)			//cond
			{
				node->key[0] = temp[0].first;
				node->key[1] = temp[1].first;

				node->count[0] = temp[0].second;
				node->count[1] = temp[1].second;
			}

			if(new_node)
			{
				new_node->key[0] = temp[2].first;
				new_node->count[0] = temp[2].second;
				new_node->isleaf = true;		//since leaf is split, new node is also leaf
			}

			if(node && new_node)
			{
				new_node->child_ptr[2] = node->child_ptr[2];
				node->child_ptr[2] = new_node;
			}
						

			if(node->parent == NULL)		//create new root
			{
				root = initialize();
				root->key[0] = temp[1].first;
				root->child_ptr[0] = node;
				root->child_ptr[1] = new_node;
				node->parent = root;
				new_node->parent = root;
			}
			else							//not at root
			{
				new_node->parent->buffer = new_node;
				root = insert(node->parent, temp[1].first);
			}
			
		}
		else				//intermediate node has to be split
		{
			vector< BPNode* > temp_child_ptr = node->child_ptr;
			vector< ll > temp = node->key;
			
			temp.push_back(num);
			sort(temp.begin(), temp.end());

			ll index = upper_bound(temp.begin(), temp.end(), num) - temp.begin();
			temp_child_ptr.insert(temp_child_ptr.begin() + index, node->buffer);
			
			if(new_node!=NULL)
			{
				new_node->key[0] = temp[2];
				new_node->key[1] = LONG_LONG_MAX;
				new_node->child_ptr[0] = temp_child_ptr[2];
				new_node->child_ptr[1] = temp_child_ptr[3];
				new_node->child_ptr[2] = NULL;
			}

			new_node->child_ptr[0]->parent = new_node;
			new_node->child_ptr[1]->parent = new_node;

			if(node)
			{
				node->key[0] = temp[0];
				node->key[1] = LONG_LONG_MAX;
				node->child_ptr[0] = temp_child_ptr[0];
				node->child_ptr[1] = temp_child_ptr[1];
				node->child_ptr[2] = NULL;
				
				node->child_ptr[0]->parent = node;
				node->child_ptr[1]->parent = node;
			}

			if(node->parent == NULL)	//create new root		
			{
				root = initialize();
				root->key[0] = temp[1];
				root->child_ptr[0] = node;
				root->child_ptr[1] = new_node;
				node->parent = root;
				new_node->parent = root;
			}	
			else						//non-root node
			{	
				new_node->parent->buffer = new_node;
				root = insert(node->parent, temp[1]);
			}
		} 
	}
	return root;
}
////////////////////////////////////////////////////////////////////////////////////

bool find(BPNode *node, ll num)
{
	ll i = 0;
	while(i < ORDER - 1)
	{
		if(node->key[i] == num)
			return true;
		 i++;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////
ll count(BPNode *node, ll num)
{
	if(node)
	{
		ll c = 0,fwd_count , i = 0;
		while(i < ORDER - 1)
		{
			if(node->key[i] == num)
				c+= node->count[i];
			if(node->key[i] > num)
				break;
			if(node->key[i] == LONG_LONG_MAX)
			{
				fwd_count = count(node->child_ptr[2], num);		//count element in nodes after current node
				c += fwd_count;
			}

			i++;
		}
	
		fwd_count = count(node->child_ptr[2], num);		//recurse to next node
		c += fwd_count;
		return c;
	}

	return 0;

}

////////////////////////////////////////////////////////////////////////////////////
void range(BPNode *node, ll x, ll y, ll &c)
{
	if(node)
	{
		ll i = 0;
		while( i < ORDER - 1)
		{
			if(node->key[i] >= x && node->key[i] <= y)
				c+= node->count[i];
			if(node->key[i] > y)
				break;
			if(node->key[i] == LONG_LONG_MAX)
				range(node->child_ptr[2], x, y, c);

			i++;
		}
		range(node->child_ptr[2], x, y, c);
	}
}

////////////////////////////////////////////////////////////////////////////////////
void traverse(BPNode *root)
{
	if(root)
	{
		queue<BPNode *>q;
		q.push(root);
		while(!q.empty())
		{
			int n=q.size();
			while(n--)
			{
				BPNode *node = q.front();
				q.pop();

				for (int i = 0; i < node->key.size(); ++i)
					if(node->key[i] < LONG_LONG_MAX)
						cout<<"("<<node->key[i]<<","<<node->count[i]<<")|";
					else
						cout<<"_"<<"|";
						
				cout<<"   ";
				

				for (int i = 0; i < node->child_ptr.size(); ++i)
					if(!node->isleaf && node->child_ptr[i] != NULL)
						q.push(node->child_ptr[i]);
			}

			cout<<"\n\n";
		}

		cout << endl;
	}
}

////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{	
	ifstream fin;
	ofstream fout;
	fin.open(argv[1]);
	string line;
	ll x, y;
	fout.open("out.txt");
	while(fin)
	{
		getline(fin, line);
		vector< string > command = split(line);
		
		//cout << line <<"\n";
		try{
			if(command[0] == "INSERT")
			{
				x = stoll(command[1]);
				BPNode *node = findBlock(root, x);
				
				if(node)
				{
					//getchar();
					//cout << node->key[0] <<"\n";
					root = insert(node, x);
				}
				else{
					root = initialize();
					root->key[0] = x;
					root->isleaf = true;
				}
			}
			if(command[0] == "FIND")
			{
				x = stoll(command[1]);
				BPNode *node = findBlock(root, x);
				if(node == NULL)
				{
					//cout << "NO\n";
					fout << "NO\n";
				}
				else
				{
					if(find(node, x))
					{
						//cout << "YES\n";
						fout << "YES\n";
					}
					else
					{
						//cout << "NO\n";
						fout << "NO\n";
					}
				}
			}
			if(command[0] == "COUNT")
			{
				x = stoll(command[1]);
				BPNode *node = findBlock(root, x);
				if(node)
				{
					//cout << count(node, x) << "\n";
					fout << count(node, x) << "\n";
					
				}
				else
				{
					//cout << "0\n";
					fout << "0\n";
				}
			}
			if(command[0] == "RANGE")
			{
				x = stoll(command[1]);
				y = stoll(command[2]);
				BPNode *node = findBlock(root, x);
				if(node)
				{
					ll c = 0;
					range(node, x, y, c);
					//cout << c <<"\n";
					fout << c <<"\n";
					
				}
				else
				{
					//cout << "0\n";
					fout << "0\n";
				}
			}
			
		}
		catch(...)
		{
			cout << "Syntax Error\n";
			exit(0);
		}

	}
	//traverse(root);
	return 0;
}