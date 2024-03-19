#define FILE_EXTENSION ".txt"
#include<fstream>
#include<string>
#include<cstring>
#include<cctype>
#include<vector>
#include<iostream>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;
int txtFileCount = 0;
const int ALPHABET_SIZE = 26;

// string parser: output vector of strings (words) after parsing
vector<string> word_parse(vector<string> tmp_string)
{
    vector<string> parse_string;
    for (auto &word : tmp_string)
    {
        string new_str;
        for (auto &ch : word)
        {
            if (isalpha(ch))
                new_str.push_back(ch);
        }
        parse_string.emplace_back(new_str);
    }
    return parse_string;
}

vector<string> split(const string &str, const string &delim)
{
    vector<string> res;
    if ("" == str)
        return res;

    char *strs = new char[str.length() + 1];
    strcpy(strs, str.c_str());

    char *d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char *p = strtok(strs, d);
    while (p)
    {
        string s = p;
        res.push_back(s);
        p = strtok(NULL, d);
    }

    return res;
}

// trie node
struct TrieNode
{
    TrieNode *children[ALPHABET_SIZE];
    bool isEndOfWord;
};

struct TrieNode *getNode()
{
    struct TrieNode *pNode = new TrieNode();

    pNode->isEndOfWord = false;

    for (int i = 0; i < ALPHABET_SIZE; i++)
        pNode->children[i] = NULL;

    return pNode;
}

void insert(struct TrieNode *root, const string &key)
{
    TrieNode *pCrawl = root;
    for (char ch : key) 
    {
        int index = ch - 'a';
        
        if (!pCrawl->children[index])
            pCrawl->children[index] = getNode();

        pCrawl = pCrawl->children[index];
    }

    // mark the last node as leaf
    pCrawl->isEndOfWord = true;
}

bool prefixSearch(TrieNode *root, const string &prefix) {
    TrieNode* pCrawl = root;
    for (char ch : prefix) {
        int index = tolower(ch) - 'a';

        if (!pCrawl->children[index])
            return false;

        pCrawl = pCrawl->children[index];
    }

    // If the loop completes, the prefix is found
    return true;
}

bool wildcardSearch(TrieNode* node, const string& pattern, int index, string currentWord) {
    if (index == pattern.length()) {
        // Check if the current word is present in the Trie
        return (node != nullptr) && (node->isEndOfWord);
    }

    char currentChar = pattern[index];
    // If the current character is '*', try matching the pattern with each child
    if (currentChar == '*') {
        for (int i = 0; i < ALPHABET_SIZE; ++i) {
            if ((node->children[i]) &&
            ((wildcardSearch(node->children[i], pattern, index, currentWord))||wildcardSearch(node->children[i], pattern, index + 1, currentWord))){
                    return true;
            }
        }
        return wildcardSearch(node, pattern, index + 1, currentWord);
    } else {
        // If the current character is not '*', try matching with the corresponding child
        int childIndex = currentChar - 'a';
        if (node->children[childIndex]) {
            return wildcardSearch(node->children[childIndex], pattern, index + 1, currentWord+currentChar);
        } else {
            return false;
        }
    }
}

bool fullWordSearch(TrieNode* root, const string& word) {
    TrieNode* pCrawl = root;
    for (char ch : word) {
        int index = ch - 'a';
        if (!pCrawl->children[index]) {
            // The current character is not present in the Trie
            return false;
        }

        pCrawl = pCrawl->children[index];
    }
    // Check if the last node represents the end of the word
    return (pCrawl != nullptr) && (pCrawl->isEndOfWord);
}


int main(int argc, char *argv[])
{
    // INPUT:
    // 1. data directory in data folder
    // 2. number of txt files
    // 3. output route
    string data_dir = argv[1] + string("/"); //data file path
    string query = string(argv[2]);           //query.txt
    string output = string(argv[3]);          //output.txt
    string fullDataDir = data_dir + "/";
    for (const auto & entry : fs::directory_iterator(data_dir)) {
        if (entry.path().extension() == ".txt") {
            txtFileCount++;
        }
    }
    // Read File & Parser Example
    string file, title_name, tmp;
    fstream fi;
    ofstream fout;
    vector<string> tmp_string;
    vector<string> directory(10000);
    // from data_dir get file ....
    vector<TrieNode*> tree(txtFileCount);
	vector<TrieNode*> treeReverse(txtFileCount);
    // eg : use 0.txt in data directory
    for (int i = 0; i < txtFileCount; ++i)
    {
        file = data_dir + to_string(i) + FILE_EXTENSION;
        fi.open(file, ios::in);
        tree[i] = getNode();
        treeReverse[i] = getNode();
        // GET TITLENAME
        getline(fi, title_name);
        // GET TITLENAME WORD ARRAY
        directory[i] = title_name;
        tmp_string = split(title_name, " ");
        vector<string> title = word_parse(tmp_string);
        for (auto &word : title)
        {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            insert(tree[i], word);
            reverse(word.begin(), word.end());
            insert(treeReverse[i], word);
            // cout << title_name << endl;
        }
        // GET CONTENT LINE BY LINE
        while (getline(fi, tmp))
        {
            // GET CONTENT WORD VECTOR
            tmp_string = split(tmp, " ");
            // PARSE CONTENT
            vector<string> content = word_parse(tmp_string);
            for (auto &word : content)
            {
                transform(word.begin(), word.end(), word.begin(), ::tolower);
                insert(tree[i], word);
                reverse(word.begin(), word.end());
                insert(treeReverse[i], word);
            }
        }
        // CLOSE FILE
        fi.close();
    }
    // query
    string whole_line;
    fi.open(query, ios::in);
    fout.open(output, ios::out);
    while(getline(fi, whole_line))
    {
        int now_pos = 0;
        int len = whole_line.size();
        int and_or_mode = 0; 
        bool total_result[10000] = {0};
        while(now_pos < len)
        {
            // and
            if(whole_line[now_pos] == '+'){
                now_pos += 2;
                and_or_mode = 1;
            }
			
            //or
            else if(whole_line[now_pos] == '/'){
                now_pos += 2;
                and_or_mode = 2;
            }
			else if(whole_line[now_pos] == '-'){
				now_pos += 2;
				and_or_mode = 3;
			}
            // full search
            else if(whole_line[now_pos] == '"'){
                int right_boundry;
                for(right_boundry = now_pos + 1; right_boundry < len; right_boundry++)
                    if(whole_line[right_boundry] == '"')
                        break;

                string test;
                test = whole_line.substr(now_pos + 1, right_boundry - now_pos - 1);
                now_pos = right_boundry + 2;
                transform(test.begin(), test.end(), test.begin(), ::tolower);
                bool result[10000] = {0};
                for(int i = 0; i < txtFileCount; i++){
                    if(fullWordSearch(tree[i], test))
                    {
                        result[i] = 1;
                    }
                }
                if(and_or_mode == 0){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i];
                }
                else if(and_or_mode == 1){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] && total_result[i];
                }
                else if(and_or_mode == 2){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] || total_result[i];
                }
				else if(and_or_mode == 3){
					for(int i = 0; i < txtFileCount; i++)
						total_result[i] = !result[i] && total_result[i];
				}
            }
			//wildcard search
			else if(whole_line[now_pos] == '<')
            {
                int right_boundry;
                for(right_boundry = now_pos + 1; right_boundry < len; right_boundry++)
                    if(whole_line[right_boundry] == '>')
                        break;

                string test;
                test = whole_line.substr(now_pos + 1, right_boundry - now_pos - 1);
                now_pos = right_boundry + 2;
                transform(test.begin(), test.end(), test.begin(), ::tolower);
                bool result[10000] = {0};
                for(int i = 0; i < txtFileCount; i++){
                    if(wildcardSearch(tree[i], test, 0, "")){
						result[i] = 1;
						//cout << i << endl;
					}
                }
                if(and_or_mode == 0){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i];
                }
                else if(and_or_mode == 1){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] && total_result[i];
                }
                else if(and_or_mode == 2){
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] || total_result[i];
                }
				else if(and_or_mode == 3){
					for(int i = 0; i < txtFileCount; i++)
						total_result[i] = !result[i] && total_result[i];
				}
            }
            // suffixSearch
            else if(whole_line[now_pos] == '*'){
                int right_boundry;
                for(right_boundry = now_pos + 1; right_boundry < len; right_boundry++)
                    if(whole_line[right_boundry] == '*')
                        break;

                string test;
                test = whole_line.substr(now_pos + 1, right_boundry - now_pos - 1);
                now_pos = right_boundry + 2;
                transform(test.begin(), test.end(), test.begin(), ::tolower);
                reverse(test.begin(), test.end());
                bool result[10000] = {0};
                for(int i = 0; i < txtFileCount; i++)
                {
                    if(prefixSearch(treeReverse[i], test))
                    {
                        result[i] = 1;
                        //cout << i << endl;
                    }

                }

                if(and_or_mode == 0)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i];
                }
                else if(and_or_mode == 1)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] && total_result[i];
                }
                else if(and_or_mode == 2)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] || total_result[i];
                }
				else if(and_or_mode == 3){
					for(int i = 0; i < txtFileCount; i++)
						total_result[i] = !result[i] && total_result[i];
				}
            }

            // prefix
            else
            {
                int right_boundry;
                for(right_boundry = now_pos; right_boundry < len; right_boundry++)
                    if( whole_line[right_boundry] == ' ' )
                        break;

                string test;
                test = whole_line.substr(now_pos, right_boundry - now_pos);
                now_pos = right_boundry + 1;
                transform(test.begin(), test.end(), test.begin(), ::tolower);
                bool result[10000] = {0};
                for(int i = 0; i < txtFileCount; i++)
                {
                    if(prefixSearch(tree[i],test))
                    {
                        result[i] = 1;
                        //cout << i << endl;
                    }

                }

                if(and_or_mode == 0)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i];
                }
                else if(and_or_mode == 1)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] && total_result[i];
                }
                else if(and_or_mode == 2)
                {
                    for(int i = 0; i < txtFileCount; i++)
                        total_result[i] = result[i] || total_result[i];
                }
				else if(and_or_mode == 3){
					for(int i = 0; i < txtFileCount; i++)
						total_result[i] = !result[i] && total_result[i];
				}
            }

        }

		int all_zero =1;
		for(int i = 0; i < txtFileCount; i++){
			if(total_result[i] == 1){
				fout<<directory[i]<<endl;
				all_zero = 0;
			}
		}
		if(all_zero == 1){
			fout<<"Not Found!"<<endl;
		}
	}
	fi.close();
	fout.close();
}