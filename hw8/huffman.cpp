 
#include<iostream>
#include<fstream>
#include<vector>
#include<string.h>
#include<math.h>
using namespace std;
//tree node
class node{
	public:
		node(char a,int b){
			data=a;
			count=b;
			left=NULL;
			right=NULL;
			code="";
		}
		node(node* n1,node* n2){
			left=n1;
			right=n2;
			data=left->data;
			count=left->count+right->count;
			code="";
		}
		unsigned char data;
		node* left,* right;
		int count;
		string code;
};
//sort the data by frenquency
void mergesort(node* data[], int len){
	node** a = data;
	node** b = new node*[len];
	for (int seg = 1; seg < len; seg += seg) {
		for (int start = 0; start < len; start += seg + seg) {
			int low = start, mid = min(start + seg, len), high = min(start + seg + seg, len);
			int k = low;
			int start1 = low, end1 = mid;
			int start2 = mid, end2 = high;
			while (start1 < end1 && start2 < end2)
				b[k++] = a[start1]->count <= a[start2]->count ? a[start1++] : a[start2++];
			while (start1 < end1)
				b[k++] = a[start1++];
			while (start2 < end2)
				b[k++] = a[start2++];
		}
		node** temp = a;
		a = b;
		b = temp;
	}
	if (a != data) {
		for (int i = 0; i < len; i++)
			b[i] = a[i];
		b = a;
	}
	delete[] b;
}
void huffman(node* &root,vector<node*>& table,vector<node*>& nodes,ifstream& in){
	//read data and count the frequency
	char c;
	int count[256]={0};
	in.seekg(0,in.end);
	int size=in.tellg();
	in.seekg(0,in.beg);
	for(int i=0;i<size;in.seekg(++i,in.beg))
		++count[in.peek()];
	for(int i=0;i<256;i++){
		if(count[i]){
			node* temp=new node((char)i,count[i]);
			nodes.push_back(temp);
		}
	}
	//build data table
	for(int i=0;i<nodes.size();i++)
		table.push_back(nodes[i]);
	//build tree
	while(nodes.size()>1){
		mergesort(&nodes[0],nodes.size());
		node* temp1=nodes[0];
		node* temp2=nodes[1];
		if(temp2->data<temp1->data)
			swap(temp1,temp2);
		node* temp=new node(temp1,temp2);
		nodes[1]=temp;
		nodes.erase(nodes.begin());
	}
	root=nodes[0];
}
//build huffman tree code
void huffmantable(node* root){
	if(root->left!=NULL && root->right!=NULL){
		root->left->code=root->code+"0";
		root->right->code=root->code+"1";
		huffmantable(root->left);
		huffmantable(root->right);
	}
}
//when get 8 bite than out
int idx=7;
int push_buf(int bit,ofstream& out){
    static unsigned char bufcode=0;
    bufcode |= bit<<idx;
    idx--;
    if(idx<0){
    	out.put(bufcode);
        idx=7;
        bufcode=0;
    }
}

void compress(int size,vector<node*> table,ifstream& in,ofstream& out){
	in.seekg(0,in.end);
	int filesize=in.tellg(); // file size
	int cfilesize=0; // compressed file size
	int cdatasize=0; // compressed rate
	for(int i=0;i<table.size();i++){		
		cfilesize+=(2+table[i]->code.length());//originally file header size, 2 represents character and blank A000
		cdatasize+=table[i]->count*table[i]->code.length();//encode data size
	}
	if(cdatasize%8!=0)
		++cfilesize; // add left bit
	cfilesize+=cdatasize/8; // bit to byte
	cfilesize=cfilesize+8+log10(filesize)+log10(table.size());
	cfilesize+=log10(cfilesize);
	int c_rate=((double)cfilesize/filesize)*100;
	if(c_rate>=100) 
		++cfilesize;
	out<<filesize<<" "<<cfilesize<<" "<<c_rate<<" "<<table.size()<<" ";
	cout<<"\nFile Byte:"<<filesize<<endl;
	cout<<"Compressed File Byte:"<<cfilesize<<endl;
	cout<<"Compression Rate:"<<c_rate<<"%"<<endl;
	for(int i=0;i<table.size();i++){
		out.put(table[i]->data);
		out<<table[i]->code<<" ";
	}
	unsigned char c;
	in.seekg(0,in.beg);
	while(in.get((char&)c)){
		for(int i=0;i<table.size();i++){
			if(table[i]->data==c){
				for(int j=0;j<table[i]->code.length();j++){	
					push_buf(table[i]->code[j]-'0',out);
				}
				++i;
				break;				
			}
		}
	}
	// add 0 to complete 1 byte for the left 
	if(idx!=7)
		do{
			push_buf(0,out);
		}while(--idx);	
}
void decodetable(int n,node* root,ifstream& in){
	node* cur;
	string s;
	unsigned char c;
	while(n--){
		in.get((char&)c);
		in>>s;
		cur=root;
		for(int i=0;i<s.length();i++){
			if(s[i]-'0'==1){
				if(cur->right==NULL)
					cur->right=new node(0,0);
				cur=cur->right;
			}
			else{
				if(cur->left==NULL)
					cur->left=new node(0,0);
				cur=cur->left;
			}
		}
		cur->data=c;
		in.get((char&)c);
	}
}
void uncompressed(ifstream& in,ofstream& out){
	node* root=NULL;
	int o,n; //o: file bytes, n: table size
	in>>o;
	in>>n;
	in>>n;
	in>>n;
	unsigned char c;
	root=new node(0,0);
	node* cur;
	in.get((char&)c);
	decodetable(n,root,in);
	int code[8],idx=8;
	while(o--){
		cur=root;
		while(1){
		    if(idx==8){
				in.get((char&)c);
		    	idx=0;		    	
		        for(int i=0;i<8;i++){ // every timme get first bit and save to code[]
					n=(c>=128);
					c=c<<1;
					code[i]=n;
				}
		    }
		    if(code[idx++]==1)
		    	cur=cur->right;
		    else
		    	cur=cur->left;
		    if(cur->left==NULL && cur->right==NULL)
				break;
		}
		out<<cur->data;
	}
}

// huffman c input.txt output.txt
// huffman u compressed.txt unncompressed.txt
int main(int argc, char* argv[]){
	ifstream in;
	ofstream out;
	vector<node*> nodes;
	vector<node*> table;
	node* root=NULL;
	if(argc!=4||(strcmp(argv[1],"c")!=0&&strcmp(argv[1],"u")!=0)){
		cout<<"input error\n";
		return 0;
	}
	else if(strcmp(argv[1],"c")==0){
		in.open(argv[2],ios::binary); //input.txtx
		huffman(root,table,nodes,in);
		huffmantable(root);
		out.open(argv[3],ios::binary); //output.txt
		cout<<"-----Huffman encoding table-----\t"<<endl;
		for(int i=0;i<table.size();i++)
			cout<<table[i]->data<<" "<<table[i]->code<<endl;
			compress(root->count,table,in,out);
			cout<<"\n-----compress finished-----"<<endl;
		}
	else if(strcmp(argv[1],"u")==0){
		in.open(argv[2],ios::binary);
		out.open(argv[3],ios::binary);
		uncompressed(in,out);
		cout<<"\n-----uncompress finished-----"<<endl;
	}
	in.close();
	out.close();	
	return 0;
}

