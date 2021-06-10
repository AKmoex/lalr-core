

#include "Lex.h"
// 判断字母
bool Lex::is_letter(char x) {
    return (x >= 'a' && x <= 'z' || x >= 'A' && x <= 'Z');
}

// 判断数字
bool Lex::is_number(char x) {
    return x >= '0' && x <= '9';
}

// 判断行内注释
bool Lex::is_annotation() {
    // 连续读取两个字符
    char ch1=instring[p];
    p++;
    char ch2=instring[p];
    p--;
    // 若连续两个 // 即为行内注释
    return ch1=='/'&&ch2=='/';
}

// 判断字符串
bool Lex::is_string(char x) {
    return x=='"';
}

// 处理数字
void Lex::handle_number() {
    type = 5;
    int i = 0;
    // 整数
    while (is_number(instring[p])) {
        ci += instring[p];
        p++;
    }
    //处理浮点数
    if(instring[p]=='.'){
        // 继续扫描数字
        ci+=instring[p];
        p++;
        while (is_number(instring[p])) {
            ci += instring[p];
            p++;
        }
        // 数字后面跟着字符则出错
        while (is_letter(instring[p]) || is_number(instring[p])) {
            ci += instring[p];
            p++;
            type = 9;
        }
    }
    else{
        //如果数字之后接着字符
        while (is_letter(instring[p]) || is_number(instring[p])) {
            ci += instring[p];
            p++;
            type = 9;
        }
    }
    outtoken = ci;
    col++;
}

// 处理标识符和关键字
void Lex::handle_letter() {
    int i = 0;
    // 读取连续的字母序列
    while (is_letter(instring[p]) || is_number(instring[p])) {
        id += instring[p];
        i++;
        p++;
    }

    // 处理得到的词
    for (i = 0; i < k.size(); i++) {
        if (id == k[i]) {
            type = 1;
            col++;
            outtoken = id;
            return;
        }
    }
    // 检查是否出错
    for (i = 0; i < id.length(); i++){
        if (!is_letter(id[i]) && !is_number(id[i])) {
            type = 9;
            outtoken = id;
            col++;
            return;
        }
    }
    // 其他情况均为标识符
    type = 6;
    col++;
    outtoken = id;
}

// 处理行内注释
void Lex::handle_annotation() {
    // 依次扫描字符直到出现\n换行符
    while(instring[p]!='\n'){
        // 将读取到的字符拼接起来
        annotation+=instring[p];
        p++;
    }
    // 将其标记为行内注释
    type=7;
    // 暂存输出内容
    outtoken = annotation;
    // 列数加一
    col++;
}

// 处理其他字符
void Lex::handle_symbol() {
    char ch1 = instring[p++];
    char ch2 = instring[p];
    type = 9;
    // 先处理双目符的种类
    switch (ch1) {
        case '+':
            if (ch2 == '+')
                type = 3; break;
        case '-':
            if (ch2 == '-')
                type = 3; break;
        case '>':
            if (ch2 == '=')
                type = 4; break;
        case '<':
            if (ch2 == '=' || ch2 == '>')
                type = 4; break;
    }
    // 单独处理双目符
    if (ch1 == '>' && ch2 == '=' || ch1 == '<' && ch2 == '=' || ch1 == '<' && ch2 == '>' || ch1 == '+' && ch2 == '+' || ch1 == '-' && ch2 == '-') {
        p++;
        // 将char字符转为字符串
        stringstream s1,s2;
        s1 << ch1;
        s2 << ch2;
        string str1 = s1.str();
        string str2 = s2.str();
        outtoken = str1 + str2;
        col++;
        return;
    }
    else {
        char chq[2];
        chq[0] = ch1;
        chq[1] = '\0';
        // 判断是否为分界符
        for (int i = 0; i < s1.size(); i++) {
            if (chq == s1[i]) {
                type = 2;
                break;
            }
        }
        // 判断是否为算数运算符
        for (int i = 0; i < s2.size(); i++) {
            if (chq == s2[i]) {
                type = 3;
                break;
            }
        }
        // 判断是否为关系运算符
        for (int i = 0; i < s3.size(); i++) {
            if (chq == s3[i]) {
                type = 4;
                break;
            }
        }
    }
    col++;
    outtoken = ch1;
}

// 处理字符串
bool Lex::handle_string(){
    // 先复制一份当前位置
    int p_s=p;
    string s="";
    p++;
    // 连续读取字符，直到出现"或换行符或全部读取完
    while(instring[p]!='\n'&&instring[p]!='"'&&instring[p]!='\0'){
        // 拼接上字符
        s+=instring[p];
        p++;
    }
    // 判断最后一个字符是否是“
    if(instring[p]=='"'){
        // type=8 为字符串
        type=8;
        col++;
        p++;
        // 暂存字符串内容
        outtoken="\""+s+"\"";
        return true;
    }
        // 不是则说明不是字符串
        // 回到起初的读取位置
    else{
        p=p_s;
        return false;
    }
}

// 主处理函数
void Lex::handle() {
    // 处理空格、换行符、tab符
    while (instring[p] == ' ' || instring[p] == '\n'||instring[p] == '\t'){
        // 换行符,行数需加1
        if (instring[p] == '\n'){
            row++;
            col = 1;
        }
        p++;
    }
    char ch = instring[p];
    // 判断是否是字母
    if (is_letter(ch)){
        handle_letter();
    }
        // 判断是否是数字
    else if (is_number(ch)){
        handle_number();
    }
        // 判断是否是行内注释
    else if(is_annotation()){
        handle_annotation();
    }
        // 判断是否是字符串
    else if(is_string(ch)){
        if(!handle_string()){
            handle_symbol();
        }
    }
        // 处理其他字符
    else{
        handle_symbol();
    }
}

// 将词法分析结果保存
void Lex::save_info() {
    WordInfo w = {
            outtoken,
            type,
            make_pair(row,col - 1)
    };

    words.push_back(w);
    // 清空缓存值
    outtoken = "";
    ci = "";
    id = "";
}

// 入口函数
string Lex::run(string words,string keywords,string delimiters,string method){
    init(keywords,delimiters);
    web_input(words);
    p = 0;
    int codelen = strlen(instring);
    while (p < codelen) {
        handle();
        save_info();
    }
    terminal_output();
    return web_output();
}

// 初始化，设置自定义的关键字种类
void Lex::init(string keywords,string delimiters) {
    // 以空格作为分隔符切割字符串得到关键字
    // 初始化关键字列表
    string one_keywords = "";
    for (int i = 0; i < keywords.length(); i++) {
        if (keywords[i] != ' ')
            one_keywords += keywords[i];
        else {
            k.push_back(one_keywords);
            one_keywords = "";
        }
    }
    k.push_back(one_keywords);
    // 初始化自定义分界符号列表
    string one_delimiter= "";
    for (int i = 0; i < delimiters.length(); i++) {
        if (delimiters[i] != ' ')
            one_delimiter += delimiters[i];
        else {
            s1.push_back(one_delimiter);
            one_delimiter = "";
        }
    }
    s1.push_back(one_delimiter);
}

// 终端控制台输入
void Lex::terminal_input() {
    // 以@符作为输入结束符号
    do {
        instring[p++] = getchar();
    } while (instring[p - 1] != '@');
    getchar();
    instring[p - 1] = '\0';
}

// web网页端输入
void Lex::web_input(string words) {
    // 将web端传过来的词法数据复制一份
    strcpy(instring, words.c_str());
}

// web网页端输出
string Lex::web_output() {
    // 拼接字符串,供web端输出
    string s = "";
    for (int i = 0; i < words.size(); i++) {
        WordInfo w = words[i];
        stringstream s1;
        stringstream s2;
        stringstream s3;
        s1 << w.category;
        s2 << w.position.first;
        s3 << w.position.second;
        s += w.word + " " + s1.str() + " " + s2.str() + " " + s3.str() + "\n";
    }
    return s;
}

// 终端控制台输出
void Lex::terminal_output() {
    cout << left;
    cout << setw(6) << "单词" << "        " << setw(6) << "二元序列" << "          " << setw(6) << "类型" << "        " << setw(6) << "位置(行，列)" << endl;
    for (int i = 0; i < words.size(); i++) {
        WordInfo w = words[i];
        if (w.category == 9) {
            cout << setw(6) << w.word << "        " << setw(6) << "Error" << setw(11) << " " << setw(10) << "Error";
        }
        else {
            cout << left;
            cout << setw(6) << w.word << "        " << "<" << w.category << "," << w.word;
            cout << setw(6 - w.word.length()) << ">" << "        ";
            switch (w.category) {
                case 1:cout << left << setw(10) << "关键字"; break;
                case 2:cout << left << setw(10) << "分界符"; break;
                case 3:cout << left << setw(10) << "算术运算符"; break;
                case 4:cout << left << setw(10) << "关系运算符"; break;
                case 5:cout << left << setw(10) << "无符号数"; break;
                case 6:cout << left << setw(10) << "标识符"; break;
            }
        }
        cout << left;
        cout << "        " << "(" << w.position.first << "," << w.position.second << ")" << endl;
    }
}