
#ifndef LALR_CORE_LL1_H
#define LALR_CORE_LL1_H

#include <iostream>
#include <ctype.h>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include<list>
#include<set>
#include "./json//single_include//nlohmann//json.hpp"
using json = nlohmann::json;


using namespace std;

// 一个文法句子
class Grammar {
    friend class LL1;
public:
    Grammar(const string& in);
    // 分割终结符、非终结符、产生式集合、左部非终结符名字，返回分割是否成功
    bool analyze_grammar_string();
private:
    // 产生式
    string prod;

    // 文法左部非终结符
    char left_VN;

    // 候选式集合
    set<string> candidates;

    // 该文法中涉及到的非终结符集合
    set<char> Vn;

    // 终结符
    set<char> Vt;
    string cut(int i, int j) {
        return string(prod.begin() + i, prod.begin() + j);
    }
    friend bool operator == (const Grammar& a, const char& c) {
        return a.left_VN == c;
    }
    // 该文法是否合法
    bool isValid;

};

class LL1 {
private:
    // 文法G
    vector<Grammar>Grammars;

    // 非终结符
    set<char> VN;

    // 终结符
    set<char> VT;

    // 暂时存储终结符
    set<char> VT_SAVE;

    // first集
    map<char, set<char> > FIRST;

    // follow集
    map<char, set<char> > FOLLOW;

    // 分析表
    map<pair<char, char>, string> M;

    // 求first集
    set<char> first(const string& s);

    // 求follow集
    void follow();

    // 分析栈
    vector<char> parse;

    // 表达式输入栈
    vector<char> indata;

    // 求分析表
    void table();

    // 记录分析步骤
    vector<string>Steps;

    // 非终结符，用list容器与前面的非终结符区分开来
    list<string> _vn;

    // 对应终结符的产生式，用list容器与前面的产生式vector区分开来
    list<list<string>> ListVn;

    // 所有的非终结符集合
    // 便于消除左递归与提取做公因子时选取使用
    set<char> upLetter={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};

    // 当前处理得字符
    char ch;

    //单个文法
    string str = "";

    //暂存输入的所有文法
    char buf[1024];

    // 用来标记位置
    char *p;

    // 是否分析成功标志，web网页端画分析树用
    bool success=true;

    // 消除左递归和提取左公因子后的新的文法
    vector<string>new_grammars;

public:
    bool addProd(const Grammar& prod); // 添加产生式
    void build(); // 求first集、follow集、分析表

    // LL1文法分析过程
    void parser();

    // 入口函数
    string run(string grammar, string expression);

    // 从文件中读取文法
    void file_input(string path);

    // web网页端输入文法与表达式
    void web_input(string grammar,string expression);

    // 终端控制台中输出结果
    void terminal_output();

    // web网页端输出分析结果
    string web_output();

    // 初始化,维护非终结符集合
    void init(string grammars);

    // 依次读取字符
    int Getchar();

    // 依次扫描字符
    int scan();

    // 消除左递归和提取左公因子
    string left_ecursion_and_common_factors(string grammars);

    //消除左递归
    void remove_left_ecursion();

    // 提取左公因子
    void extract_common_factors();
};


#endif //LALR_CORE_LL1_H
