#include <bits/stdc++.h>
using namespace  std;

bool isNonTerminal(string s)
{
    return s.size() == 1 && isupper(s[0]);
}

vector<string> Concatenate_1(vector<string> s1, vector<string> s2)
{
    vector<string> res;
    set<string> temp;

    for (auto v1 : s1)
    {
        for (auto v2 : s2)
        {
            if (v1 == "eps" && v2 == "eps")
                temp.insert("eps");
            else if (v1 == "eps")
                temp.insert(v2);
            else
                temp.insert(v1);
        }
    }
    for (auto t : temp)  res.push_back(t);
    return res;
}

void ProcessGrammar(vector<vector<string>> &rules, map<string,int> &nonterms, map<string,int> &terms)
{
    int rules_count;
    cout << "Input amount of rules: ";
    cin >> rules_count;
    rules.resize(rules_count);

    cout << "Input rules:\n";
    for (int i=0; i<rules_count; ++i)
    {
        string rule;
        getline(cin, rule);
        if (rule.empty())
        {
            i--;
            continue;
        }

        stringstream ruleStream(rule);
        string r_NonTerm, arrow, lexeme;

        ruleStream >> r_NonTerm >> arrow;
        // 0s value is right nonterminal, 1+ are lexems on left side of the rule
        rules[i].push_back(r_NonTerm);
        nonterms.emplace(r_NonTerm, nonterms.size());

        while(ruleStream >> lexeme)
        {
            rules[i].push_back(lexeme);
            if (isNonTerminal(lexeme))
                nonterms.emplace(lexeme, nonterms.size());
            else
                terms.emplace(lexeme, terms.size());
        }
    }
}

vector<string> FindEpsNonTerminals(vector<vector<string>> rules)
{
    vector<string> eps_nonterms;
    for (int i=0; i<rules.size(); ++i)
    {
        if (rules[i][1] == "eps")
        {
            eps_nonterms.push_back(rules[i][0]);
        }
    }

    bool is_added = true;
    while (is_added)
    {
        is_added = false;
        for (int i=0; i<rules.size(); ++i)
        {
            bool non_eps = false;
            for (int j=1; j<rules[i].size(); ++j)
            {
                if (find(eps_nonterms.begin(), eps_nonterms.end(), rules[i][j]) == eps_nonterms.end())
                {
                    non_eps = true;
                    break;
                }
            }
            if (non_eps == false && find(eps_nonterms.begin(), eps_nonterms.end(), rules[i][0]) == eps_nonterms.end())
            {
                eps_nonterms.push_back(rules[i][0]);
                is_added = true;
            }
        }
    }

    return eps_nonterms;
}

vector<vector<string>> First_k(vector<vector<string>> rules, map<string,int> nonterms, vector<string> eps_nonterms)
{
    vector<vector<string>> first_table(nonterms.size());
    for (auto t : eps_nonterms)
    {
        first_table[nonterms[t]].push_back("eps");
    }

    for (int i=0; i<rules.size(); ++i)
    {
        int ind = nonterms[rules[i][0]];
        
        if (isNonTerminal(rules[i][1]) == false && find(first_table[ind].begin(), first_table[ind].end(), rules[i][1]) == first_table[ind].end())
        {
            first_table[ind].push_back(rules[i][1]);
        }
    }

    bool is_added = true;
    while (is_added)
    {
        is_added = false;
        for (int i=0; i<rules.size(); ++i)
        {
            int pos = 1;
            int ind = nonterms[rules[i][0]];
            while(pos < rules[i].size())
            {
                if (isNonTerminal(rules[i][pos]) == false && find(first_table[ind].begin(), first_table[ind].end(), rules[i][pos]) == first_table[ind].end())
                {
                    first_table[ind].push_back(rules[i][pos]);
                    is_added = true;
                    break;
                }
                else if (isNonTerminal(rules[i][pos]))
                {
                    int ind2 = nonterms[rules[i][pos]];
                    for (auto t : first_table[ind2])
                    {
                        if (find(first_table[ind].begin(), first_table[ind].end(), t) == first_table[ind].end() && t != "eps")
                        {
                            first_table[ind].push_back(t);
                            is_added = true;
                        }
                    }
                    if (find(eps_nonterms.begin(), eps_nonterms.end(), rules[i][pos]) != eps_nonterms.end())
                        pos++;
                    else
                        break;
                }
                else break;
            }
        }        
    }

    return first_table;
}

vector<vector<string>> Follow_k(vector<vector<string>> rules, map<string,int> nonterms, vector<vector<string>> first_k)
{
    vector<vector<string>> follow_table(nonterms.size());

    follow_table[nonterms[rules[0][0]]].push_back("eps");

    bool is_added = true;
    while (is_added)
    {
        is_added = false;
        for (auto nt : nonterms)
        {
            for (int i=0; i<rules.size(); ++i)
            {
                vector<string> res = {"eps"};
                int at = find(rules[i].begin()+1, rules[i].end(), nt.first) - rules[i].begin();
                if (at >= rules[i].size())
                    res.clear();
                else
                {
                    at++;
                    for (; at < rules[i].size(); at++)
                    {
                        if (isNonTerminal(rules[i][at]))
                            res = Concatenate_1(res, first_k[nonterms[rules[i][at]]]);
                        else
                        {
                            vector<string> term = {rules[i][at]};
                            res = Concatenate_1(res, term);
                        }
                    }
                }
                res = Concatenate_1(res, follow_table[nonterms[rules[i][0]]]);

                for (auto m : res)
                {
                    if (find(follow_table[nonterms[nt.first]].begin(), follow_table[nonterms[nt.first]].end(), m) == follow_table[nonterms[nt.first]].end())
                    {
                        follow_table[nonterms[nt.first]].push_back(m);
                        is_added = true;
                    }
                }
            }
        }
    }

    return follow_table;
}

vector<vector<string>> MakeContextTable(vector<vector<string>> rules, map<string,int> nonterms, vector<vector<string>> first_k, vector<vector<string>> follow_k)
{
    vector<vector<string>> context_table(rules.size());

    for (int i=0; i<rules.size(); ++i)
    {
        vector<string> res = {"eps"};
    
        for (int j=1; j<rules[i].size(); ++j)
        {
            vector<string> temp;
            if (isNonTerminal(rules[i][j]))
                temp = first_k[nonterms[rules[i][j]]];
            else
                temp = {rules[i][j]};
            res = Concatenate_1(res, temp);
        }
        res = Concatenate_1(res, follow_k[nonterms[rules[i][0]]]);
        context_table[i] = res;
    }

    return context_table;
}

vector<vector<int>> MakeControlTable(vector<vector<string>> rules, map<string,int> nonterms, map<string,int> terms, vector<vector<string>> context_table)
{
    vector<vector<int>> control_table(nonterms.size(), vector<int>(terms.size(), -1));

    for (int i=0; i<rules.size(); ++i)
    {
        for (auto lexeme : context_table[i])
        {
            int ind1 = nonterms[rules[i][0]];
            int ind2 = terms[lexeme];
            control_table[ind1][ind2] = i;
        }
    }

    return control_table;
}

bool Analyzer(vector<vector<string>> rules, map<string,int> nonterms, map<string,int> terms, vector<vector<int>> control_table, vector<string> input)
{
    stack<string> input_stack, lexeme_stack;

    input_stack.push("eps");
    lexeme_stack.push("eps");
    lexeme_stack.push(rules[0][0]);
    for (int i = input.size()-1; i>=0; i--)
        input_stack.push(input[i]);

    vector<int> lex_seq;
    
    while (true)
    {
        if (input_stack.top() == "eps" && lexeme_stack.top() == "eps")
        {
            cout << "\nInput is accepted\nSequence: ";
            for (auto s : lex_seq)
                cout << s << " ";
            return true;
        }
        else if (lexeme_stack.top() == "eps" && lexeme_stack.size() > 1)
        {
            lexeme_stack.pop();
        }
        else if (input_stack.top() == lexeme_stack.top())
        {
            input_stack.pop();
            lexeme_stack.pop();
        }
        else if (isNonTerminal(lexeme_stack.top()) == false)
        {
            cout << "\nERROR! In position " << input.size() - input_stack.size() + 1 <<  ". Expected value " << lexeme_stack.top();
            return false;
        }
        else
        {
            int ind1 = nonterms[lexeme_stack.top()];
            int ind2 = terms[input_stack.top()];
            if (control_table[ind1][ind2] == -1)
            {
                cout << "\nERROR! In position " << input.size() - input_stack.size() + 1 <<  ". Expected posible values are: ";
                for (auto v : terms)
                {
                    if (control_table[ind1][v.second] != -1)
                        cout << v.first << " ";
                }
                cout << "\n";
                return false;
            }
            else
            {
                int rule_ind = control_table[ind1][ind2];
                lex_seq.push_back(rule_ind + 1);
                lexeme_stack.pop();
                for (int i = rules[rule_ind].size()-1; i>0; i--)
                {
                    lexeme_stack.push(rules[rule_ind][i]);
                }
            }
        }
    }

    return 0;
}

int main()
{
    vector<vector<string>> rules;
    map<string,int> nonterms, terms;

    ProcessGrammar(rules, nonterms, terms);

    cout << "\n------------------------------------------------\n";
    cout << "NonTerms: ";
    for (auto a : nonterms)
    {
        cout << a.first << " ";
    }
    cout << "\n------------------------------------------------\n";
    cout << "Terms: ";
    for (auto a : terms)
    {
        cout << a.first << " ";
    }
    cout << "\n------------------------------------------------\n";

    cout << "Eps_NonTerms: ";
    vector<string> eps_nonterms = FindEpsNonTerminals(rules);
    for (string s : eps_nonterms)
    {
        cout << s << " "; 
    }
    cout << "\n------------------------------------------------\n";
        
    cout << "First: \n";
    vector<vector<string>> first_1 = First_k(rules, nonterms, eps_nonterms);
    for (auto m : nonterms)
    {
        cout << m.first << ": ";
        for (auto t : first_1[m.second])
        {
            cout << t << " ";
        }
        cout << '\n';
    }
    cout << "\n------------------------------------------------\n";
        
    cout << "Follow: \n";
    vector<vector<string>> follow_1 = Follow_k(rules, nonterms, first_1);
    for (auto m : nonterms)
    {
        cout << m.first << ": ";
        for (auto t : follow_1[m.second])
        {
            cout << t << " ";
        }
        cout << '\n';
    }
    cout << "\n------------------------------------------------\n";
        
    cout << "Context: \n";
    vector<vector<string>> context_table = MakeContextTable(rules, nonterms, first_1, follow_1);
    for (int i=0; i<rules.size(); ++i)
    {
        cout << "Rule #" << i+1 << ": ";
        for (auto q : context_table[i])
        {
            cout << q << " ";
        }
        cout << '\n';
    }
    cout << "\n------------------------------------------------\n";
        
    cout << "Control table: \n";
    vector<vector<int>> control_table = MakeControlTable(rules, nonterms, terms, context_table);
    for (auto a : nonterms)
    {
        for (auto b : terms)
        {
            if (control_table[a.second][b.second] != -1)
            {
                cout << a.first << " - " << b.first << " - " << control_table[a.second][b.second] + 1 << '\n';
            }
        }
    }
    cout << "\n------------------------------------------------\n";

    cout << "Input lexeme to analyze: ";
    vector<string> input;
    string input_str, lex;
    getline(cin, input_str);
    stringstream input_stream(input_str);

    while(input_stream >> lex)
        input.push_back(lex);

    Analyzer(rules, nonterms, terms, control_table, input);

    return 0;
}