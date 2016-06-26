#define _CRT_SECURE_NO_WARNINGS 1
 
#include<iostream>
#include"Heap.h"
#include<string>
#include<fstream>
#include<time.h>
#include<assert.h>
using namespace std;

template<class T>
struct HaffmanTreeNode
{
  HaffmanTreeNode<T>* _left;
  HaffmanTreeNode<T>* _right;
  T _weight;

  HaffmanTreeNode(const T& weight):_left(NULL)
	                              ,_right(NULL)
				                  ,_weight(weight)
  {}
};

template<class T>
class HaffmanTree
{
	typedef HaffmanTreeNode <T> Node;
public:

	HaffmanTree(const T* a,size_t n,const T& invalid)
	{
		_root=_HaffmanTree(a,n,invalid);
	}

	Node* GetrootNode()
	{
	  return _root;
	}


	~HaffmanTree()
	{
	 _Destory(_root);
	}


	void _Destory(Node* root)
	{
	if(root==NULL)
		return;
	_Destory(root->_left);
	_Destory(root->_right);

	delete root;
	}

protected:

	Node*  _HaffmanTree(const T* a,size_t n,const T& invalid)
	{
	   struct Nodecompare
		{
			bool operator()(const Node* l,const Node* r)
				{
				
					return l->_weight < r->_weight;
				}
		};

		Heap<Node*,Nodecompare>minHeap;
       //建小堆,方便每次都取最小的两个数字
		for(size_t i=0;i<n;i++)
		{
		  if(a[i] != invalid)
		   {
	          minHeap.Push (new Node(a[i]));
		   }
		}

		while(minHeap.Size()>1)
		{
			Node* left=minHeap.Top();
			minHeap.Pop (left);
			Node* right=minHeap.Top();
			minHeap.Pop(right);
			Node* parent=new Node(left->_weight+right->_weight);
			parent->_left=left;
			parent->_right=right;
			minHeap.Push(parent);
			_root=minHeap.Top ();
		}
		return _root;
	}


protected:
	Node* _root;
};

//1统计字符出现的次数
//2构建哈弗曼树
//3生成哈弗曼编码
//文件压缩

typedef unsigned long long longType;
struct charInfo
{
	unsigned char _ch;//字符
	longType _count;//字符出现的次数
	string _code;//生成的哈弗曼编码

	charInfo():_ch(0)
		      ,_count(0)
	{}
	charInfo(const longType count)
		:_ch(0)
		,_count(count)
		                        
	{}

	bool operator!=(const charInfo& info)const
	{
		 return _count!=info._count ;
	}

    charInfo operator+(const charInfo& info)const
	{
		return charInfo(_count+info._count);
	}

	bool operator<(const charInfo& info)const
	{
		return _count<info._count;
	}
};

//文件压缩
class FileCompress
{
	
public:
	FileCompress()
	{
		for(size_t i=0;i<256;i++)
		{
		_infos[i]._ch=i;
		_infos[i]._count=0;
		}
	}
public:
	//文件压缩
	void Compress(const char* filename)
	{
		//以二进制的方式进行读，否则以普通文本读的方式编译器会会对它进行处理
		FILE* fOut=fopen(filename,"rb");
		assert(fOut);

		//1统计文件中字符出现的次数
		char ch=fgetc(fOut);
		while(!feof(fOut))
		{		
			_infos[(unsigned char)ch]._count++;
			ch=fgetc(fOut);
		}

	//2构建哈弗曼树
	charInfo invalid(0);
	HaffmanTree<charInfo>tree(_infos,256,invalid);
	//3生成哈弗曼编码（以递归的方式）
	string code;
	GenerateHaffmanCode(tree.GetrootNode(),code);
	//4读源文件字符进行压缩，将哈弗曼编码写进对应的比特位
	string Compressfilename=filename;
	Compressfilename+=".compress";
	FILE*fin=fopen(Compressfilename.c_str(),"wb");
	//将文件指针挪到文件头部
	fseek(fOut,0,SEEK_SET);
	ch=fgetc(fOut);
	char value=0;
	int pos=0;
	while(!feof(fOut))
	{
		  string &code=_infos[(unsigned char)ch]._code;
		  for(size_t i=0;i<code.size() ;i++)
		  {
			
			  if(code[i]=='1')
			  {
			   value|=1;
			  }
			/*  else
			  {}	*/  
			  //当满8位的时候将哈弗曼编码写进对应的文件，然后继续读取后续的哈弗曼编码
			  if(++pos==8)
			  {
			  fputc(value,fin);
			  value=0;
			  pos=0;
			  }
			   value=value<<1;
		   
		  }
			 ch=fgetc(fOut);
		 }
		  

		  //若最后几位哈弗曼编码不满8位，则需要进行处理，写配置文件
		  if(pos!=0)
		  {
			  value<<=(7-pos);
			  fputc(value,fin);
		  }


		//写配置文件
		  string configfilename=filename;
		  configfilename+=".config";
		  FILE* finconfig=fopen(configfilename.c_str(),"wb");
		  assert(finconfig);
		  char buffer[128];
		  string str;
		  for(size_t i=0;i<256;++i)
		  {
			  if(_infos[i]._count>0)
			  {
		       str+=_infos[i]._ch;
			   str+=",";
			   str+=_itoa(_infos[i]._count,buffer,10);
			   //itoa把整数_count转化成字符存入buffer中，10进制的形式
			   str+='\n';
			  }
			  fputs(str.c_str(),finconfig);
			  str.clear();//循环写str的形式
		  }
		  fclose(fOut);
		  fclose(fin);
		  fclose(finconfig);

	}


	//解压缩
	void unCompress(const char* filename)
	{
		 string configfilename=filename;
		 configfilename+=".config";
		 FILE* foutconfig=fopen(configfilename.c_str(),"rb");
		 assert(foutconfig);
		 string str;
		 longType charCount=0;
		 while(Readline(foutconfig,str))
		 {
			  if(str.empty())
			  {
			   str+='\n';
			  }
			  //将配置文件中保存的字符格式转化为次数：a,4;b,3;c,2;d,1
			  else
			  {
			  _infos[(unsigned char)str[0]]._count=atoi(str.substr(2).c_str());
			  str.clear();
			  }
		 }

		//重建哈弗曼树
		 charInfo invalid(0);
		 HaffmanTree<charInfo >tree(_infos,256,invalid);
		 //读取压缩文件进行还原
		 charCount=tree.GetrootNode()->_weight._count ;
		 string Compressfilename=filename;
		 Compressfilename+=".compress";
		 FILE* fout=fopen(Compressfilename.c_str(),"rb");
		 assert(fout);
		 string Uncompressfilename=filename;
		 Uncompressfilename +=".uncompress";
		 FILE* fin=fopen( Uncompressfilename.c_str(),"wb");
		 assert(fin);
		 char ch=fgetc(fout);
		HaffmanTreeNode<charInfo>*root=tree.GetrootNode();
		HaffmanTreeNode<charInfo>*cur=root;
		int pos=7;		
		while(1)
		{
			if(ch& (1<<pos))
			{
				cur=cur->_right;
			}
			else
			{
				cur=cur->_left;
			}

			if(pos--==0)
			{
				pos=7;
				ch=fgetc(fout);
			}

			if((cur->_left==NULL)&&(cur->_right==NULL))
			{
				fputc(cur->_weight._ch,fin);

				if(--charCount==0)
				{
					 break;
				}
				cur=root;
			}
		}

		fclose(fin);
	}

protected:

	void GenerateHaffmanCode(HaffmanTreeNode<charInfo>* root,string code)
	{
		if(root==NULL)
		{
		 return;
		}

		if(root->_left)
		{
		GenerateHaffmanCode(root->_left ,code+'0');
		}

		if(root->_right)
		{
		GenerateHaffmanCode(root->_right ,code+'1');
		}

		if((root->_left ==NULL) && (root->_right==NULL))
		{
		  _infos[root->_weight._ch ]._code=code;
		  return;
		}

	}

	bool Readline(FILE* fout,string& str)
	{
		char ch=fgetc(fout);
		if(feof(fout))
		{
			 return false;
		}

		while(!feof(fout) && ch!='\n')
		{
			 str+=ch;
			 ch=fgetc(fout);
		}
			return true;
	}

protected:
	charInfo _infos[256];
};

void HaffmanTreeTest()
{
	int a[]={2,3,5,7,8};
	size_t n=sizeof(a)/sizeof(a[0]);
    HaffmanTree<int>(a,n,0);

}

void compressTest()
{
 FileCompress fc;
 double start_time=clock();
 //char *filename="file.txt";
 fc.Compress("dawenjian.txt"); 
double finish_time=clock();
 cout<<"压缩时间是："<<finish_time-start_time<<"ms"<<endl;

}

void uncompressTest()
{

	FileCompress fc;
	
	double start_time=clock();
	fc.unCompress ("dawenjian.txt");
	double finish_time=clock();
	cout<<"解压缩时间是："<<finish_time-start_time<<"ms"<<endl;

}
