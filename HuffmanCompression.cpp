#include <fstream>
#include <string>
using namespace std;

//Store the Huffman code of all ASCIII characters
string huffcode[256];

/**
 * Define the node type of the Huffman tree
 *
 */
struct Node {
    int value;
    int frequence;

    Node *left, *right, *next;
    Node():value(-1),frequence(0),left(NULL),right(NULL),next(NULL) {}
};

//Define the array of the generated nodes through combining
Node allNodes[255];
//Stores the numbers of generated nodes
int arrL = 0;

/**
 * Build the Huffman tree and return the root node
 *
 * head: The chain of all occurred characters sorted in the order
 * of incremental frequency
 * length: The length of the chain
 */
Node HuffmanTree(Node *head, int length) {
    //If only two nodes left, combine them and return
    if (length == 2) {
        Node n;
        n.left = head;
        n.right = head->next;
        return n;
    }

    //Generate a new node through combining the first two nodes
    int ll = arrL;
    ++arrL;
    allNodes[ll].frequence = head->frequence + head->next->frequence;
    allNodes[ll].left = head;
    allNodes[ll].right = head->next;
    //Resort the chain in the order of incremental frequency
    if (allNodes[ll].frequence <= head->next->next->frequence) {
        allNodes[ll].next = head->next->next;
        head = &allNodes[ll];
    }
    else {
        head = head->next->next;
        Node *l = head;
        Node *r = head->next;

        while (true) {
            if (r == NULL) {
                l->next = &allNodes[ll];
                break;
            }
            else if (allNodes[ll].frequence > r->frequence) {
                l = r;
                r = r->next;
            }
            else {
                l->next = &allNodes[ll];
                allNodes[ll].next = r;
                break;
            }
        }
    }
    //The chain will decrease one in every time
    return HuffmanTree(head, length-1);
}

/**
 * Generate the Huffman code for all characters according
 * to the Huffman tree
 *
 * p: The root node pointer of the Huffman tree
 * s: The binary string that should be inserted in the code
 */
void Coding(Node *p, string s) {
    if (p->left == NULL) {
        huffcode[p->value] = s;
    }
    else {
        Coding(p->left, s+"0");
        Coding(p->right, s+"1");
    }
}

/**
 * The n power of 2
 *
 * n: The exponent
 */
inline int PowerTwo(int n) {
    int r = 1;
    return r<<n;
}

/**
 * Turn the integer to the huffman code
 *
 */
inline string IntToHuffman(int a) {
    return huffcode[a];
}

/**
 * Turn the ASCIII to the 8-bit binary
 *
 */
string IntToByte(int a) {
    string s;
    while (a != 0) {
        s.insert(s.begin(), a%2+'0');
        a /= 2;
    }
    if (s.length() < 8)
        s.insert(s.begin(), 8-s.length(), '0');
    return s;
}

/**
 * Turn the 8-bit binary to the ASCIII
 *
 */
char ByteToAsc(string s) {
    int sum = 0;
    for (int i=0; i<8; ++i) {
        if (s[i] == '1') sum+=PowerTwo(7-i);
    }
    char c = sum;
    return c;
}

/**
 * Find the character that has the least frequency and is greater than zero
 *
 */
int Minimal(int f[]) {
    int m=0;
    int index=0;
    for (int i=0; i<256; ++i) {
        if (f[i] > 0 && m == 0) {
            m = f[i];
            index = i;
        }
        else if (m > f[i] && f[i] > 0) {
            m = f[i];
            index = i;
        }
    }
    return index;
}

/**
 * Compress the file, and decompress the generated file at the same time
 *
 */
int Compress(char* inputfile, char* outfile, char* outfile2) {
    ifstream in(inputfile);
    ofstream out(outfile, ios::binary);//Write the file in the binary mode

    int freq[256];
    for (int i=0; i<256; ++i) freq[i]=0;
    char c;
    //Compute the frequency of all characters
    while (in.get(c)) {
        int asc = c;
        ++freq[asc];
    }

    //Generate the chain of all occurred characters
    Node leafNodes[256];
    int leafLength = 0;
    for (int i=0; i<256; ++i) {
        int m = Minimal(freq);
        if (freq[m] > 0) {
            leafNodes[i].value = m;
            leafNodes[i].frequence = freq[m];
            freq[m] = 0;
            if (i > 0) leafNodes[i-1].next = &leafNodes[i];
        }
        else {
            leafLength = i;
            break;
        }
    }

    //Generate the Huffman tree
    Node n = HuffmanTree(&leafNodes[0], leafLength);
    //Generate the Huffman code
    Coding(&n, "");

    //Generate the compressed file in which every 8 binary are
    //turned into an ASCIII
    in.clear();
    in.seekg(ios::beg);
    string buffer;
    int redundant = 0;
    while (in.get(c)) {
        int a = (unsigned char)c;
        buffer += IntToHuffman(a);
        while (buffer.length() > 7) {
            out<<ByteToAsc(buffer.substr(0, 8));
            buffer.erase(0, 8);
        }
    }
    //If the length of binary string is not a multiple of 8, add some zeros
    if (buffer.length() > 0) {
        redundant = 8-buffer.length();
        buffer.append(redundant, '0');
        out<<ByteToAsc(buffer.substr(0, 8));
        buffer.erase(0, 8);
    }

    in.close();
    out.close();

    //Decompress the compressed file
    ifstream in2(outfile, ios::binary);
    ofstream out2(outfile2);

    while (in2.get(c)) {
        int a = (unsigned char)c;
        buffer += IntToByte(a);
    }

    //Abandon the redundant zeros at the end of the file
    buffer.erase(buffer.length()-redundant, redundant);
    //Traverse the Huffman tree to find the corresponding Huffman code
    Node *head = &n;
    Node *index = head;
    for (int i=0; i<=buffer.length(); ++i) {
        if (index->value >= 0) {
            out2<<(char)index->value;
            index = &n;
            if(i != buffer.length()) --i;
        }
        else if (buffer[i] == '0') index = index->left;
        else if (buffer[i] == '1') index = index->right;
    }
    in2.close();
    out2.close();
    return 0;
}

int main () {
    Compress("Aesop_Fables.txt", "Compress1.out", "Decompress1.txt");
    Compress("graph.txt", "Compress2.out", "Decompress2.txt");
    return 0;
}
