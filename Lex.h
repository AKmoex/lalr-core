//
// Created by AKmoex on 2021/6/10.
//

#ifndef LALR_CORE_LEX_H
#define LALR_CORE_LEX_H


#include<iostream>
#include<stdio.h>
#include<cstring>
#include<iomanip>
#include <fstream>
#include<sstream>
#include<vector>

using namespace std;

// 单词结构体
// 包含属性：单词、种类、位置
struct WordInfo {
    string word;
    int category;
    pair<int, int>position;
};

class Lex {
public:
    // 判断字母
    bool is_letter(char x);

    // 判断数字
    bool is_number(char x);

    // 判断行内注释
    bool is_annotation();

    // 判断字符串
    bool is_string(char x);

    // 处理数字
    void handle_number();

    // 处理标识符或关键字
    void handle_letter();

    // 处理行内注释
    void handle_annotation();

    // 处理其他字符
    void handle_symbol();

    // 处理字符串
    bool handle_string();

    // 主处理程序
    void handle();

    // 将词法分析结果保存
    void save_info();

    // 入口函数
    string run(string words,string keywords,string delimiters,string method);

    // 初始化函数",",";","(",")","[","]","{","}"
    void init(string keywords = "do end for if printf scanf then while",string delimiters=", ; ( ) [ ] { }");

    // 终端控制台输入
    void terminal_input();

    // web网页端输入
    void web_input(string words);

    // web网页端输出
    string web_output();

    // 终端控制台输出
    void terminal_output();
private:
    // 记录位置
    int row = 1, col = 1;
    // 单词对应的种类
    int type, p = 0;
    // 暂存输入
    char instring[100];
    // 暂存输出
    string outtoken = "";
    string ci = "";
    string id = "";
    // 暂存 字符串
    string annotation="";
    // 保存分析完的单词信息
    vector<WordInfo> words;
    // 关键字,可自定义
    vector<string>k;
    // 分界符,可自定义
    vector<string> s1;
    // 算术运算符
    vector<string> s2 = { "+","-","*","/","++","--", };
    // 关系运算符
    vector<string> s3 = { ">","<",">=","<=","=","<>" };
};


#endif //LALR_CORE_LEX_H
