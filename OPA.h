
#ifndef LALR_CORE_OPA_H
#define LALR_CORE_OPA_H



#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include<vector>
#include <cstring>
#include "./json//single_include//nlohmann//json.hpp"
using json = nlohmann::json;

using namespace std;





int test_s();                          //字串分析
int is_T(char c);
int index(char c);
string stack_p(int j,int k,char *s);      //打印
void firstvt(char c);                //求FIRSTVT集
void lastvt(char c);                 //求LASTVT集
bool build_table();						//生成算符文法优先关系表
string run(string grammars,string expression);
void web_input(string gg,string expression);
bool judge_ag();
void web_output();
int handle_grammars();

#endif //LALR_CORE_OPA_H
