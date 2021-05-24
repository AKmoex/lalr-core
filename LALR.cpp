//
// Created by AKmoex on 2021/5/23.
//

#include "LALR.h"
set<char>Item::Vn;
set<char>Item::Vt;
set<char>Item::Symbol;
const char* LR::actionStatStr[] = {
        "acc",
        "s",
        "r"
};

Prod::Prod(const string& in) {
    left_VN = in[0];
    right = cut(in, 3, in.length()); // A->X1X2X3X4
}

void Item::analyse_grammar_line(const string& prod) {

    // �Ƚ�����ߵķ��ս������
    Vn.insert(prod[0]);
    Symbol.insert(prod[0]);

    // ��ȡ�ս�������ս��
    for (int i = 3; i < prod.length(); i++) {
        char ch = prod[i];
        if (isupper(ch)) {
            Vn.insert(ch);
            Symbol.insert(ch);
        }
        else if (ch != '|') {
            Vt.insert(ch);
            Symbol.insert(ch);
        }
    }

    // ��ȡ��ѡʽ
    for (int i = 3; i < prod.length(); i++) {
        int j;
        for (j = i + 1; j < prod.length() && prod[j] != '|'; j++);
        Prod p = Prod(string(1, prod[0]) + "->" + Prod::cut(prod, i, j));
        if (find(prods.begin(), prods.end(), p) == prods.end()) // ȥ��
            prods.push_back(p);
        i = j;
    }
}

void LR::web_input(string grammars, string expression) {
    G.Vt.clear();
    G.Vn.clear();
    G.Symbol.clear();
    cout<<"�������ս��"<<endl;
    for (auto vn : G.Vn) {
       cout<<vn<<endl;
    }
    cout<<"�������ս��"<<G.prods.size()<<endl;
    // �ķ���
    string grammar_line = "";
    for (int i = 0; i < grammars.length(); i++) {
        if (grammars[i] != '\n') {
            grammar_line += grammars[i];
        }
        else {
            cout << grammar_line << endl;
            G.analyse_grammar_line(grammar_line);
            grammar_line = "";
        }
    }
    // ���⴦��һ�����һ���ķ��ַ���
    G.analyse_grammar_line(grammar_line);
    // ���ʽ
    inStr.push_back('#');
    status.push_back(0);
    for (int i = expression.length() - 1; i >= 0; --i) {
        inStr.push_back(expression[i]);
    }
    cout<<"�������ս��"<<endl;
    for (auto vn : G.Vn) {
        cout<<vn<<endl;
    }
}

Item LR::closure(Item I) {
    if (I.prods.size() == 0) return I;

    // ö��I�Ĳ���ʽ
    for (int i = 0; i < I.prods.size(); ++i) {

        Prod prod = I.prods[i];
        unsigned long pointLoc = 0;
        if ((pointLoc = prod.right.find('.')) != string::npos && pointLoc != prod.right.length() - 1) { // �ҵ�.��A->a.Bc,d
            char X = prod.right[pointLoc + 1];
            // �ս��
            if (G.Vt.find(X) != G.Vt.end()) {
                if (X == '@') // @���⴦��
                    swap(I.prods[i].right[pointLoc], I.prods[i].right[pointLoc + 1]);
                continue;
            }

            string f = Prod::cut(prod.right, pointLoc + 2, prod.right.length());

            set<char> ff = {};
            for (const auto& c : prod.prospect) {
                set<char> fs = first(f + c);
                ff.insert(fs.begin(), fs.end());
            }

            for (vector<Prod>::iterator it = G.prods.begin(); it != G.prods.end(); ++it) {
                if (*it == X) { // �ҵ�����ʽ
                    Prod p = *it;
                    if (p.right[0] == '@') { // ���⴦��.@ => @.
                        p.right = '.' + p.right;
                        swap(p.right[0], p.right[1]);
                    }
                    else
                        p.right = '.' + p.right;

                    vector<Prod>::iterator Iit = find(I.prods.begin(), I.prods.end(), p); // ��I���Ƿ���ڲ���ʽ
                    if (Iit != I.prods.end())  // �ҵ�
                        Iit->prospect.insert(ff.begin(), ff.end());
                    else {
                        p.prospect.insert(ff.begin(), ff.end());
                        I.prods.push_back(p);
                    }
                }
            }
        }
    }
    // }
    return I;
}

Item LR::Goto(const Item& I, char X) {
    Item J;
    // ��Ŀ��Ϊ�ջ���X==@����
    if (I.prods.size() == 0 || X == '@') return J;

    // ����I�е�ÿ����Ŀ
    for (const auto& p : I.prods) {
        string right = p.right;
        unsigned long pointLoc = right.find('.');
        if (right[pointLoc + 1] == X) {
            swap(right[pointLoc], right[pointLoc + 1]);
            J.prods.push_back(Prod(p.left_VN, right, p.prospect));
        }
    }
    return closure(J);
}

// ����Ŀ��״̬��DFA
void LR::items() {
    Item initial;
    // ��չ�ķ�
    initial.prods.push_back(Prod('^', '.' + string(1, G.prods[0].left_VN), { '#' }));
    // ���״̬��Ӧ��Ŀ
    C.push_back(closure(initial));

    int size = 0;
    // ����״̬����������

    while (size != C.size()) {
        size = C.size();
        // ������ÿ������Ŀ��������һ������ָ���״̬(��Ŀ��)
        for (int i = 0; i < C.size(); i++) {
            Item I = C[i];
            // ÿ���ķ�����X�ҳ�����Ļ�������Ŀ
            for (const auto& X : G.Symbol) {
                Item next = Goto(I, X);
                // ��Ϊ��ʱ�ſ������ӵ�����Ŀ����
                if (next.prods.size() != 0) {
                    auto it = find(C.begin(), C.end(), next);
                    // ��һ״̬�Ѿ�����
                    if (it != C.end()) {
                        // ����һ��������
                        GOTO[make_pair(i, X)] = it - C.begin();
                    }
                    else {
                        // ����һ��״̬�������ϻ���
                        C.push_back(next);
                        GOTO[make_pair(i, X)] = C.size() - 1;
                    }
                }
            }
        }
    }
}

// LALR�ϲ���Ŀ����
void LR::merge(){

    // ������Ŀ����
//    cout<<"--------------------------------------------------"<<endl<<endl;
//    for (const auto& I : C) {
//        vector<string>s;
//        int i = &I - &C[0];
//        cout<<endl<<"��Ŀ"<<i<<endl;
//        for (const auto& p : I.prods) { // �г���Ŀ
//            vector<string>v=p.displayStr2();
//            cout<<v[0]<<"   "<<v[1]<<"  ";
//            for(int m=2;m<v.size();m++){
//                cout<<v[m]<<",";
//            }
//            cout<<endl;
//        }
//        cout<<endl<<endl;
//    }

    cout<<"--------------------------------------------------"<<endl<<endl;
    // ����GOTO
    cout<<"����GOTO��"<<endl;
    for(map<pair<int,char>,int>::iterator it = GOTO.begin(); it != GOTO.end(); it++){
        cout<<it->first.first<<" + "<<it->first.second<<" -> "<<it->second<<endl;
    }
    cout<<"--------------------------------------------------"<<endl<<endl;


    map<int,vector<int>>same;
    set<int>skip_item;
    for(int i=1;i<C.size();i++){
        if(skip_item.count(i)==0){
            cout<<endl<<"�����Ŀ"<<i<<endl;
            set<string>items_i;
            for (const auto& p_i : C[i].prods) {
                vector<string>v_i=p_i.displayStr2();
                string s_i=v_i[0]+"->"+v_i[1];
                items_i.insert(s_i);
            }

            //
            for(int j=i+1;j<C.size();j++){
                set<string>items_j;
                for (const auto& p_j : C[j].prods) {
                    vector<string>v_j=p_j.displayStr2();
                    string s_j=v_j[0]+"->"+v_j[1];
                    items_j.insert(s_j);
                }
                bool isEqual=items_i.size()==items_j.size()&&equal(items_i.begin(), items_i.end(), items_j.begin(), items_j.end());
                if(isEqual){
                    cout<<j<<"  ";
                    for(auto x:items_i){
                        cout<<x<<endl;
                    }
                    same[i].push_back(j);
                    skip_item.insert(j);
                }
            }
        }

    }

    cout<<"���������-----------------------------------------"<<endl<<endl;
    for(auto x:skip_item){
        cout<<x<<endl;
    }


    map<int,int>ys;
    int dd=0;
    for(int d=0;d<C.size();d++){
        if(skip_item.count(d)==1){
            continue;
        }
        else{
            ys[d]=dd;
            dd++;
        }
    }


    for(map<int,vector<int>>::iterator it = same.begin(); it != same.end(); it++){
        cout<<it->first<<":";
        for(int i=0;i<it->second.size();i++){
            int same_item=it->second[i];

            // Ѱ��ԭGOTO������same_item����Ļ�
            for(map<pair<int,char>,int>::iterator it2 = GOTO.begin(); it2 != GOTO.end(); it2++){
                // �ҵ������
                if(it2->first.first==same_item){
                    // �ж���������յ��Ƿ��Ǻϲ����������Ŀ��
                    // false����������
                    bool zhong=false;
                    for(int zz=0;zz<it->second.size();zz++){
                        if(it2->second==it->second[zz]){
                            zhong=true;
                        }
                    }
                    // �յ�������
                    if(zhong){
                        GOTO[make_pair(it->first,it2->first.second)]=it->first;
                    }
                    else{
                        GOTO[make_pair(it->first,it2->first.second)]=it2->second;
                    }
                }
            }

            // Ѱ��ԭGOTO��������same_item�Ļ�
            for(map<pair<int,char>,int>::iterator it2 = GOTO.begin(); it2 != GOTO.end(); it2++){
                // �ҵ�����
                if(it2->second==same_item){
                    GOTO[make_pair(it2->first.first,it2->first.second)]=it->first;
                }
            }

            // ɾ��������same_item�йصĻ�
            // �ȸ���һ��
            map<pair<int,char>,int>g=GOTO;
            for(map<pair<int,char>,int>::iterator it2 = g.begin(); it2 != g.end(); it2++){
                // �ҵ������
                if(it2->first.first==same_item){
                    // ɾ����
                    GOTO.erase(make_pair(it2->first.first,it2->first.second));
                }
                // �ҵ�����
                if(it2->second==same_item){
                    GOTO.erase(make_pair(it2->first.first,it2->first.second));
                }
            }

            // �ϲ�չ����

            // ɾ����Ŀ����
            // ��ȡ������������
            //swap(*(begin(C)+same_item),*(end(C)-1));
            //C.pop_back();
        }

    }


    // ɾ���������Ŀ����
    int delete_item_nums=0;
    for(auto x:skip_item){
        C.erase(C.begin()+x-delete_item_nums);
        delete_item_nums++;
    }
    // ����GOTO���ӳ���ϵ
    map<pair<int,char>,int>gg=GOTO;
    for(map<pair<int,char>,int>::iterator it = gg.begin(); it != gg.end(); it++){
        int x=it->first.first;
        char ch=it->first.second;
        int y=it->second;

        GOTO.erase(make_pair(it->first.first,it->first.second));

        GOTO[make_pair(ys[x],ch)]=ys[y];
    }


    cout<<"--------------------------------------------------"<<endl<<endl;
    // ����GOTO
    cout<<"����GOTO��"<<endl;
    for(map<pair<int,char>,int>::iterator it = GOTO.begin(); it != GOTO.end(); it++){
        cout<<it->first.first<<" + "<<it->first.second<<" -> "<<it->second<<endl;
    }
    cout<<"--------------------------------------------------"<<endl<<endl;
    cout<<C.size();

//    for (const auto& I : C) {
//        vector<string>s;
//        int i = &I - &C[0];
//        if(i==0){
//            continue;
//        }
//        else{
//            cout<<endl<<"��Ŀ"<<i<<endl;
//            for (const auto& p : I.prods) { // �г���Ŀ
//                vector<string>v=p.displayStr2();
//                cout<<v[0]<<"   "<<v[1]<<"  ";
//                for(int m=2;m<v.size();m++){
//                    cout<<v[m]<<",";
//                }
//                cout<<endl;
//            }
//
//            cout<<endl<<endl;
//        }
//    }



}

void LR::run() {
    build_table();

    draw_graph();

    web_output_table();
    web_output_steps();
}

// ����Action��GOTO��
void LR::build_table() {
    // ����DFA״̬��
    items();

    merge();

    // ����ÿ��״̬(��Ŀ��)
    for (int i = 0; i < C.size(); i++) {
        const Item& item = C[i];
        // ����ÿ������ʽ
        for (const auto& prod : item.prods) {
            unsigned long pointLoc = prod.right.find('.');
            // �ж�.��λ��
            // .�������
            if (pointLoc < prod.right.length() - 1) {
                char X = prod.right[pointLoc + 1];
                if (G.Vt.find(X) != G.Vt.end() && GOTO.find(make_pair(i, X)) != GOTO.end()) { // �ս��
                    int j = GOTO[make_pair(i, X)];
                    ACTION[make_pair(i, X)] = make_pair(SHIFT, j);
                }
            }
                // .�����
            else {
                if (prod == Prod('^', string(1, G.prods[0].left_VN) + '.', {}) && prod.prospect == set<char>({ '#' }))  // S'->S.,# acction[i, #] = acc
                    ACTION[make_pair(i, '#')] = make_pair(ACCEPT, 0);
                else if (prod.left_VN != '^') {
                    string right = prod.right;
                    right.erase(pointLoc, 1); // ɾ��.
                    for (const auto& X : prod.prospect) { // A->a.,b��ö��b
                        vector<Prod>::iterator it = find(G.prods.begin(), G.prods.end(), Prod(prod.left_VN, right, set<char>{}));
                        if (it != G.prods.end())  // �ҵ���
                            ACTION[make_pair(i, X)] = make_pair(REDUCE, it - G.prods.begin());
                    }
                }
            }
        }
    }
    if (G.Vt.find('@') != G.Vt.end()) { // ɾ��@���ƽ�#
        G.Vt.erase(G.Vt.find('@'));
        G.Symbol.erase(G.Symbol.find('@'));
    }
    G.Vt.insert('#');
    G.Symbol.insert('#');
}

set<char> LR::first(const string& s) { // s��Ϊ����ʽ��
    if (s.length() == 0)
        return set<char>({ '@' });
    else if (s.length() == 1) {
        if (G.Vt.find(s[0]) != G.Vt.end() || s[0] == '#') // �ս��
            return set<char>({ s[0] });
        else
        if (FIRST[s[0]].size() != 0) return FIRST[s[0]];
        else {
            for (vector<Prod>::iterator it = G.prods.begin(); it != G.prods.end(); ++it)
                if (*it == s[0]) {
                    // ��ֱֹ����ݹ�
                    int xPos = it->right.find(it->left_VN);
                    if (xPos != string::npos) { // �ҵ�X->aXb
                        if (xPos == 0) continue; // X->Xb
                        else { // X->aXb
                            string a = Prod::cut(it->right, 0, xPos);
                            if (first(a) == set<char>{'@'}) continue;
                        }
                    }
                    set<char> f = first(it->right);
                    FIRST[s[0]].insert(f.begin(), f.end());
                }


            return FIRST[s[0]];
        }
    }
    else { // first(X1X2X3X4)...
        set<char> ret;
        for (unsigned int i = 0; i < s.length(); ++i) {
            set<char> f = first(string(1, s[i])); // ���������first(Xi)��
            if (f.find('@') != f.end() && s.length() - 1 != i) { // ����@
                f.erase(f.find('@')); // ��ȥ@
                ret.insert(f.begin(), f.end()); // ����first����
            }
            else { // ��@�������һ��Xi������Ҫ����ȥ��
                ret.insert(f.begin(), f.end());
                break;
            }
        }
        return ret;
    }
}

void LR::follow() {
    FOLLOW[G.prods[0].left_VN].insert('#'); // ��ʼ���ŷ�'#'
    for (auto pp : G.prods) { // ֱ��follow(X)��������
        unsigned int size = 0;
        while (size != FOLLOW[pp.left_VN].size()) {
            size = FOLLOW[pp.left_VN].size();
            for (auto prod : G.prods) { // ������з��ս����follow����
                char X = prod.left_VN;
                for (auto p : G.prods) {// ���X��follow����
                    string s = p.right;
                    unsigned long loc = 0;
                    if ((loc = s.find(X)) != string::npos) { // �ҵ����ս��X
                        set<char> f = first(string(s.begin() + loc + 1, s.end())); // ��first(b)
                        FOLLOW[X].insert(f.begin(), f.end()); // ���뵽follow(X)��
                        if (f.find('@') != f.end()) {// �ҵ�@
                            FOLLOW[X].erase(FOLLOW[X].find('@')); // ɾ��@
                            set<char> fw = FOLLOW[p.left_VN]; // ��follow(A)����follow(X)
                            FOLLOW[X].insert(fw.begin(), fw.end());
                        }
                    }
                }
            }
        }
    }
}

string Prod::replaceAll(const string& in, const string from, const string to) {
    size_t replacePos = in.find(from);
    string res = in;
    if (replacePos != string::npos)
        res.replace(replacePos, from.length(), to);
    return res;
}

void LR::draw_graph() {

    // ���ڵ�
    // ������Ŀ��
    j["n"];
    for (const auto& I : C) {
        vector<string>s;
        int i = &I - &C[0];
        s.push_back(to_string(i));
        for (const auto& p : I.prods) { // �г���Ŀ
            string res = p.displayStr();
            if (res.find('^') != string::npos)
                res = Prod::replaceAll(res, "^", string(1, G.prods[0].left_VN) + "'");
            s.push_back(res.c_str());

        }
        j["n"].push_back(s);
    }


    // ״̬�ڵ�
    for (const auto& I : C) {
        int i = &I - &C[0];
        j["nodes"].push_back(i);
    }

    // ״̬��
    for (const auto& link : GOTO) {
        int i = link.first.first;
        int jj = link.second;
        vector<int>edge;
        edge.push_back(i);
        edge.push_back(jj);
        j["edges"].push_back(edge);
    }
}

void LR::web_output_table() {
    // �ս��
    for (auto vt : G.Vt) {
        if (vt != '#') {
            j["VT"].push_back(string(1, vt));
        }
    }
    j["VT"].push_back(string(1, '#'));

    // ���ս��
    for (auto vn : G.Vn) {
        j["VN"].push_back(string(1, vn));
    }

    // ��
    for (int i = 0; i < C.size(); i++) {

        vector<string>one_tb;

        // ������ͨ�ս��
        for (const auto& X : G.Vt) {
            if (X != '#') {
                pair<int, char> p = make_pair(i, X);
                // ������һ��
                if (ACTION.find(p) != ACTION.end()) {
                    pair<actionStat, int> res = ACTION[p];
                    string second=to_string(res.second);
                    string str = actionStatStr[res.first] + second;
                    one_tb.push_back(str);
                }
                else {
                    one_tb.push_back("");
                }
            }
        }
        // #ͳһ�������
        pair<int, char> p = make_pair(i, '#');
        // �ҵ���
        if (ACTION.find(p) != ACTION.end()) {
            pair<actionStat, int> res = ACTION[p];
            string str=actionStatStr[res.first];
            if (res.first != ACCEPT) {
                str += to_string(res.second);
            }
            one_tb.push_back(str);
        }
        else {
            one_tb.push_back("");
        }


        // ���ڷ��ս��
        for (const auto& X : G.Vn) {
            pair<int, char> p = make_pair(i, X);
            string str = "";
            if (GOTO.find(p) != GOTO.end()) {
                str += to_string(GOTO[make_pair(i, X)]);
            }
            one_tb.push_back(str);
        }

        j["Body"].push_back(one_tb);
    }

}

void LR::web_output_steps() {
    bool success = false;
    // �Ƚ�#�����ջ
    // ������ʵҲû��ϵ
    parse.push_back('#');
    while (!success) {
        vector<string>one_step;
        // ״̬ջ
        string status_string = "";
        for (int i = 0; i < status.size();i++) {
            status_string += to_string(status[i]);
        }
        one_step.push_back(status_string);

        // ����ջ
        string analyse_string = "";
        for (int i = 0; i < parse.size(); i++) {
            analyse_string += string(1, parse[i]);
        }
        one_step.push_back(analyse_string);

        // ���봮
        string expression_string = "";
        for (int i = inStr.size() - 1; i >= 0; i--) {
            expression_string+= string(1, inStr[i]);
        }
        one_step.push_back(expression_string);


        // �ֱ�ȡ����ջ�����봮ջ��
        int sTop = status.size() - 1;
        int iTop = inStr.size() - 1;
        pair<int, char> p = make_pair(status[sTop], inStr[iTop]);

        // ����,����
        if (ACTION.find(p) == ACTION.end()) {
            one_step.push_back("ERROR");
            j["Process"].push_back(one_step);
            break;
        }

        pair<actionStat, int> act = ACTION[p];

        // �ƽ�
        if (act.first == SHIFT) {

            string s = "ACTION[";
            s += to_string(p.first) + ",";
            s += string(1, p.second) + "]=S";
            s += to_string(act.second) + ",";
            s += " Status" + to_string(act.second) + "into Stack";

            status.push_back(act.second);
            parse.push_back(inStr[iTop]);
            inStr.pop_back();
            one_step.push_back(s);
        }
            // ��Լ
        else if (act.first == REDUCE) {
            string s = "r" + to_string(act.second) + ":";
            Prod p = G.prods[act.second];
            s += string(1, p.left_VN) + "->" + p.right + " Reduce, GOTO(" + to_string(status[status.size() - 1]) + "," + string(1, p.left_VN) + ")=";
            // �մ��������ջ��ֱ�ӹ�Լ
            if (p.right != "@") {
                for (unsigned i = 0; i < p.right.size(); ++i) {
                    status.pop_back();
                    parse.pop_back();
                }
            }
            parse.push_back(p.left_VN);
            status.push_back(GOTO[make_pair(status[status.size() - 1], p.left_VN)]);
            s += to_string(status[status.size() - 1]) + "into Stack";
            one_step.push_back(s);

        }
            // ����
        else if (act.first == ACCEPT) {
            success = true;
            string s = "acc,Success";
            one_step.push_back(s);
        }
        j["Process"].push_back(one_step);
    }
}

string LR::get_data() {
    string data = j.dump();
    return data;
}