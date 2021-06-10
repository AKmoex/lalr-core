#include"OPA.h"



json js;
char table[20][20];                    //算符优先关系表
char s[100];
char str_in[20][10];
char T_label[20];
char input_s[100];
int FVT[10]={0};
int LVT[10]={0};
int k;
int r;                                   //文法规则个数
int r1;
char grammar[10][30];                    //用来存储文法产生式
char FIRSTVT[10][10];                   //FIRSTVT集
char LASTVT[10][10];                    //LASTVT集

bool build_table(){
    T_label[k]='#';
    T_label[k+1]='\0';
    char text[20][10];
    int i,j,k,t,l,x=0,y=0;
    int m,n;
    x=0;
    for(i=0;i<r;i++){
        firstvt(grammar[i][0]);
        lastvt(grammar[i][0]);
    }
    for(i=0;i<r;i++){
        text[x][y]=grammar[i][0];
        y++;
        for(j=1;grammar[i][j]!='\0';j++){
            if(grammar[i][j]=='|'){
                text[x][y]='\0';
                x++;
                y=0;
                text[x][y]=grammar[i][0];
                y++;
                text[x][y++]='-';
                text[x][y++]='>';
            }
            else{
                text[x][y]=grammar[i][j];
                y++;
            }
        }
        text[x][y]='\0';
        x++;
        y=0;
    }
    r1=x;

    for(i=0;i<x;i++){
        str_in[i][0]=text[i][0];

        for(j=3,l=1;text[i][j]!='\0';j++,l++)
            str_in[i][l]=text[i][j];

        str_in[i][l]='\0';
    }



    // 是否是算符优先文法
    for(i=0;i<x;i++){
        for(j=1;text[i][j+1]!='\0';j++){
            if(is_T(text[i][j])&&is_T(text[i][j+1])){
                m=index(text[i][j]);
                n=index(text[i][j+1]);
                if(table[m][n]=='='|table[m][n]=='<'|table[m][n]=='>'){
                    js["operatprPrecedence"]=false;
                    cout<<"该文法不是算符优先文法"<<endl;
                    return false;
                }
                table[m][n]='=';
            }
            if(text[i][j+2]!='\0'&&is_T(text[i][j])&&is_T(text[i][j+2])&&!is_T(text[i][j+1])){
                m=index(text[i][j]);
                n=index(text[i][j+2]);
                if(table[m][n]=='='|table[m][n]=='<'|table[m][n]=='>'){
                    js["operatprPrecedence"]=false;
                    cout<<"该文法不是算符优先文法"<<endl;
                    return false;
                }
                table[m][n]='=';
            }
            if(is_T(text[i][j])&&!is_T(text[i][j+1]))
            {
                for(k=0;k<r;k++)
                {
                    if(grammar[k][0]==text[i][j+1])
                        break;
                }
                m=index(text[i][j]);
                for(t=0;t<FIRSTVT[k][0];t++)
                {
                    n=index(FIRSTVT[k][t+1]);
                    if(table[m][n]=='='|table[m][n]=='<'|table[m][n]=='>')
                    {
                        js["operatprPrecedence"]=false;
                        cout<<"该文法不是算符优先文法"<<endl;
                        return false;
                    }
                    table[m][n]='<';
                }
            }
            if(!is_T(text[i][j])&&is_T(text[i][j+1]))
            {
                for(k=0;k<r;k++)
                {
                    if(grammar[k][0]==text[i][j])
                        break;
                }
                n=index(text[i][j+1]);
                for(t=0;t<LASTVT[k][0];t++)
                {
                    m=index(LASTVT[k][t+1]);
                    if(table[m][n]=='='|table[m][n]=='<'|table[m][n]=='>')
                    {
                        js["operatprPrecedence"]=false;
                        cout<<"该文法不是算符优先文法"<<endl;
                        return false;
                    }
                    table[m][n]='>';
                }
            }
        }
    }
    m=index('#');
    for(t=0;t<FIRSTVT[0][0];t++)
    {
        n=index(FIRSTVT[0][t+1]);
        table[m][n]='<';
    }
    n=index('#');
    for(t=0;t<LASTVT[0][0];t++)
    {
        m=index(LASTVT[0][t+1]);
        table[m][n]='>';
    }
    table[n][n]='=';
    js["operatprPrecedence"]=true;
    return true;
}

void firstvt(char c){
    int i,j,k,m,n;
    for(i=0;i<r;i++)
    {
        if(grammar[i][0]==c)
            break;
    }
    if(FVT[i]==0)
    {
        n=FIRSTVT[i][0]+1;
        m=0;
        do
        {
            if(m==2||grammar[i][m]=='|')
            {
                if(is_T(grammar[i][m+1]))
                {
                    FIRSTVT[i][n]=grammar[i][m+1];
                    n++;
                }
                else
                {
                    if(is_T(grammar[i][m+2]))
                    {
                        FIRSTVT[i][n]=grammar[i][m+2];
                        n++;
                    }
                    if(grammar[i][m+1]!=c)
                    {
                        firstvt(grammar[i][m+1]);
                        for(j=0;j<r;j++)
                        {
                            if(grammar[j][0]==grammar[i][m+1])
                                break;
                        }
                        for(k=0;k<FIRSTVT[j][0];k++)
                        {
                            int t;
                            for(t=0;t<n;t++)
                            {
                                if(FIRSTVT[i][t]==FIRSTVT[j][k+1])
                                    break;
                            }
                            if(t==n)
                            {
                                FIRSTVT[i][n]=FIRSTVT[j][k+1];
                                n++;
                            }
                        }
                    }
                }
            }
            m++;
        }
        while(grammar[i][m]!='\0');
        FIRSTVT[i][n]='\0';
        FIRSTVT[i][0]=--n;
        FVT[i]=1;
    }
}

void lastvt(char c){
    int i,j,k,m,n;
    for(i=0;i<r;i++)
    {
        if(grammar[i][0]==c)
            break;
    }

    if(LVT[i]==0)
    {
        n=LASTVT[i][0]+1;
        m=0;

        do
        {
            if(grammar[i][m+1]=='\0'||grammar[i][m+1]=='|')
            {
                if(is_T(grammar[i][m]))
                {
                    LASTVT[i][n]=grammar[i][m];
                    n++;
                }
                else
                {
                    if(is_T(grammar[i][m-1]))
                    {
                        LASTVT[i][n]=grammar[i][m-1];
                        n++;
                    }

                    if(grammar[i][m]!=c)
                    {
                        lastvt(grammar[i][m]);
                        for(j=0;j<r;j++)
                            if(grammar[j][0]==grammar[i][m])
                                break;

                        for(k=0;k<LASTVT[j][0];k++)
                        {
                            int t;
                            for(t=0;t<n;t++)
                                if(LASTVT[i][t]==LASTVT[j][k+1])
                                    break;

                            if(t==n)
                            {
                                LASTVT[i][n]=LASTVT[j][k+1];
                                n++;
                            }
                        }
                    }
                }
            }
            m++;
        }while(grammar[i][m]!='\0');

        LASTVT[i][n]='\0';
        LASTVT[i][0]=--n;
        LVT[i]=1;
    }
}

int test_s(){
    int i,j,x,y,z;
    k=1;
    s[k]='#';
    printf("栈             输入串         动作\n");
    for(i=0;input_s[i]!='\0';i++);
    z=i--;
    i=0;
    char a,q;
    int sn_c=0;
    js["Process"];
    vector<string>op;
    while((a=input_s[i])!='\0'){
        if(is_T(s[k]))
            j=k;
        else
            j=k-1;
        x=index(s[j]);
        y=index(a);
        if(table[x][y]=='>'){
            string ss1=stack_p(1,k,s);
            cout<<ss1<<"\t\t";
            op.push_back(ss1);
            string ss2=string(1,a)+stack_p(i+1,z,input_s);
            cout<<ss2<<"\t\t";
            op.push_back(ss2);
            cout<<"归约"<<"\t";
            op.push_back("Reduce");
            do{
                q=s[j];
                if(is_T(s[j-1])){
                    j=j-1;
                    if(j<=0){
                        cout<<"输入串错误"<<endl;
                        exit(-1);
                    }
                }
                else{
                    j=j-2;
                    if(j<=0){
                        printf("输入串错误\n");
                        exit(-1);
                    }
                }
                x=index(s[j]);
                y=index(q);
            }while(table[x][y]!='<');

            int n,N;

            int N1;//存放规约符号标号
            char tep[100];//存放最左素短语
            for(int x=j+1;x<=k;x++){
                tep[x]=s[x];
            }

            for(int m=j+1;m<=k;m++){
                for(N=0;N<r1;N++)
                    for(n=1;str_in[N][n]!='\0';n++){
                        if(!is_T(s[m])&&!is_T(str_in[N][n])){
                            if(is_T(s[m+1])&&is_T(str_in[N][n+1])&&s[m+1]==str_in[N][n+1]){
                                s[j+1]=str_in[N][0];
                                N1=N;
                                break;
                            }
                        }
                        else
                        if(is_T(s[m]))
                            if(s[m]==str_in[N][n]){
                                s[j+1]=str_in[N][0];
                                N1=N;
                                break;
                            }
                    }
            }


            string left=string(1,str_in[N1][0]);
            op.push_back(left);

            string right="";
            for(int x=j+1;x<=k;x++){
                right+=string(1,tep[x]);
            }
            op.push_back(right);
            cout<<left<<"->"<<right<<endl;
            js["Process"].push_back(op);
            op.clear();



            k=j+1;

            if(k==2&&a=='#'){

                string st1=stack_p(1,k,s);
                cout<<st1<<"\t\t";
                string st2=string(1,a)+stack_p(i+1,z,input_s);
                cout<<st2<<"\t\t";
                cout<<"ACCEPT"<<endl;
                js["isSuccess"]=true;
                vector<string>op_={st1,st2,"ACCEPT"};
                js["Process"].push_back(op_);
                return 1;                               //规约成功
            }
        }
            //移进操作
        else if(table[x][y]=='<'||table[x][y]=='='){
            string sss1=stack_p(1,k,s);
            cout<<sss1<<"\t\t";
            op.push_back(sss1);
            string sss2=string(1,a)+stack_p(i+1,z,input_s);
            op.push_back(sss2);
            cout<<sss2<<"\t\t";
            cout<<"移进"<<endl;
            op.push_back("Move");
            js["Process"].push_back(op);
            op.clear();
            k++;
            s[k]=a;
            i++;
        }
        else{
            op.push_back("ERROR");
            op.push_back("ERROR");
            op.push_back("ERROR");
            js["isSuccess"]=false;
            js["Process"].push_back(op);
            op.clear();
            cout<<endl<<"该单词串不是该文法的句子"<<endl;
            return 0;
        }
    }

    cout<<endl<<"该单词串不是该文法的句子"<<endl;
    return 0;
}

string stack_p(int j,int k,char *s){
    string ss="";
    for(int i=j;i<=k;i++){
        if(s[i]!=' '){
            ss+=s[i];
        }
    }
    return ss;
}

//计算下标
int index(char c){
    for(int i=0;T_label[i]!='\0';i++){
        if(c==T_label[i]){
            return i;
        }
    }
    return -1;
}

//判断非终结符
int is_T(char c){
    for(int i=0;T_label[i]!='\0';i++){
        if(c==T_label[i])
            return 1;
    }
    return 0;
}

void web_input(string gg,string expression){
    bool flag=0;
    //string gg="S->a|^|(T)\nT->T,S|S\n#\n";
    //string gg="E->E+T|T\nT->T*F|F\nF->(E)|i\n#\n";
    vector<string>gs;
    string ogs="";
    for(int i=0;i<gg.length();i++){
        if(gg[i]!='\n'){
            ogs+=gg[i];
        }
        else{
            bool flag=false;
            string sss;
            for(int j=0;j<gs.size();j++){
                if(gs[j][0]==ogs[0]){
                    sss=gs[j];
                    gs.erase(gs.begin()+j);
                    flag=true;
                    break;
                }
            }
            if(flag){
                sss=sss+"|"+ogs.substr(3,ogs.length()-3);
                gs.push_back(sss);
            }
            else{
                gs.push_back(ogs);
            }
            ogs="";
        }
    }

    for(int i=0;i<gs.size();i++){
        strcpy(grammar[i], gs[i].c_str());
        cout<<grammar[i]<<endl;
        FIRSTVT[i][0]=0;
        LASTVT[i][0]=0;
        for (int j = 0;grammar[i][j]; j++)
            if (grammar[i][j] == '#'){
                if (j)
                    r = i+1;
                else
                    r = i;
                flag = 1;
                grammar[i][j] = '\0';
                break;
            }
        if (flag){
            break;
        }
    }

    strcpy(input_s, expression.c_str());
}

// 判断是否是算符文法
bool judge_ag(){
    for(int i=0;i<r;i++){
        for(int j=0;grammar[i][j]!='\0';j++){
            if(grammar[i][0]<'A'||grammar[i][0]>'Z'){
                printf("该文法不是算符文法\n!");
                return false;
            }
            if(grammar[i][j]>='A'&&grammar[i][j]<='Z'){
                if(grammar[i][j+1]>='A'&&grammar[i][j+1]<='Z'){
                    printf("该文法不是算符文法\n!");
                    return false;
                }
            }
        }
    }
    return true;
}

int handle_grammars(){
    cout<<r<<endl;
    for(int i=0;i<r;i++)
    {
        for(int j=0;grammar[i][j]!='\0';j++)
        {
            if((grammar[i][j]<'A'||grammar[i][j]>'Z')&&grammar[i][j]!='-'&&grammar[i][j]!='>'&&grammar[i][j]!='|')
                T_label[k++]=grammar[i][j];
        }
    }
    return k;
}

void web_output(){

    // FIRSVT集
    js["FIRSTVT"];
    cout<<endl<<"FIRSTVT集"<<endl;
    for(int i=0;i<r;i++){
        vector<string>of;
        of.push_back(string(1,grammar[i][0]));
        cout<<string(1,grammar[i][0])<<" : ";
        for(int j=0;j<FIRSTVT[i][0];j++){
            of.push_back(string(1,FIRSTVT[i][j+1]));
            cout<<string(1,FIRSTVT[i][j+1])<<" ";
        }
        js["FIRSTVT"].push_back(of);
        cout<<endl;
    }

    // LASTVT集
    js["LASTVT"];
    cout<<endl<<"LASTVT集"<<endl;
    for(int i=0;i<r;i++){
        vector<string>ol;
        ol.push_back(string(1,grammar[i][0]));
        cout<<string(1,grammar[i][0])<<" : ";
        for(int j=0;j<LASTVT[i][0];j++){
            ol.push_back(string(1,LASTVT[i][j+1]));
            cout<<string(1,LASTVT[i][j+1])<<" ";
        }
        js["LASTVT"].push_back(ol);
        cout<<endl;
    }

    // 算符优先分析表
    cout<<endl<<"算符优先分析表:"<<endl;
    // 表头
    js["Table_header"].push_back("$");
    for(int i=0;T_label[i]!='\0';i++){
        cout<<"\t"<<T_label[i];
        js["Table_header"].push_back(string(1,T_label[i]));
    }
    cout<<endl;
    // 表body
    js["Table_body"];
    for(int i=0;i<k+1;i++){
        cout<<T_label[i]<<"\t";
        vector<string>ob;
        ob.push_back(string(1,T_label[i]));
        for(int j=0;j<k+1;j++){
            cout<<table[i][j]<<"\t";
            ob.push_back(string(1,table[i][j]));
        }
        js["Table_body"].push_back(ob);
        cout<<endl;
    }

}

string run(string grammars,string expression){
    web_input("E->E+T|T\nT->T*F|F\nF->(E)|i\n#\n","i+i#");
    if(judge_ag()){
        js["isOperatorGrammar"]=true;
    }else{
        js["isOperatorGrammar"]=false;
        js["isSuccess"]=false;
        return js.dump();
    }
    k=handle_grammars();

    if(!build_table()){
        return js.dump();
    }
    web_output();

    test_s();

    return js.dump();
}
