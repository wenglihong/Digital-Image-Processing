


#include <string> 
#include <vector> 

using namespace std; 

class HuffmanTree 
{ 
private: 
	typedef struct HuffmanNode 
	{ 
		int weight; 
		int lChild, rChild, parent; 
	} *pNode; pNode huffman; 
	int * hf; //”√¿¥≈≈–Ú 
	int _n,_m; 
	void SelectNode(int i,int *min1,int *min2); 
public: HuffmanTree(int * keys, int N); 
		vector<string> HuffmanCoding(int len);
		string Encode(vector<string> hfCode, vector<char> alphabet, string str); 
		string Decode(int len, vector<char> alphabet, string str); 

		~HuffmanTree(){};
};

