#include "api.hpp"
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <stack>
#include <map>
#include <queue>
using namespace std;

// Построение ДКА по регулярному выражению

/*
Функции, применяемые к вершинам синтаксического дерева 
nullable(n) - true если соответствующее поддерево может породить пустую строку
firstpos(n) - множество позиций, которые соответствуют первым символам в подцепочках, генерируемых подвыражением с вершиной в n
lastpos(n) - множество позиций, которые соответствуют последним символам в подцепочках, генерируемых подвыражением с вершиной в n
*/

//followpos(i) - множество позиций, которые могут встретиться в каком-либо слове за i (i - позиция)

class RvNode // Реализация синтаксического дерева РВ
{
public:
	bool nullable;
	set<int> firstpos;
	set<int> lastpos;
	RvNode* left;
	RvNode* right;
	char c;
	int pos;

	RvNode(char in, int p)
	{
		left = nullptr;
		right = nullptr;
		c = in;
		pos = p;
		nullable = false;
	}

	bool setNullable()
	{
		bool l;
		bool r;
		switch(c)
		{
			case '_': 
				nullable = true; break;
			case '*':
				right->setNullable();
				nullable = true; break;
			case '|':
				l = left->setNullable();
				r = right->setNullable();
				nullable = l || r; break;
			case '.':
				l = left->setNullable();
				r = right->setNullable();
				nullable = l && r; break;
			default:
				nullable = false; break;
		}
		return nullable;
	}

	set<int> setFirst()
	{
		set<int> ans;
		set<int> right_first;
		set<int> left_first;
		if(right)
		{
			right_first = right->setFirst();
		}
		if(left)
		{
			left_first = left->setFirst();
		}
		switch(c)
		{
		case '_':
			break;
		case '*':
			if(right)
				ans = right_first;
			else
				ans = left_first;
			break;
		case '|':
			ans = left_first;
			ans.insert(right_first.begin(), right_first.end());
			break;
		case '.':
			ans = left_first;
			if(left->nullable)
				ans.insert(right_first.begin(), right_first.end());
			break;
		default:
			ans.insert(pos); break;
		}
		firstpos = ans;
		return ans;
	}

	set<int> setLast()
	{
		set<int> ans;
		set<int> right_last;
		set<int> left_last;
		if(right)
		{
			right_last = right->setLast();
		}
		if(left)
		{
			left_last = left->setLast();
		}
		switch (c)
		{
		case '_':
			break;
		case '*':
			if(right)
				ans = right_last;
			else
				ans = left_last;
			break;
		case '|':
			ans = left_last;
			ans.insert(right_last.begin(), right_last.end());
			break;
		case '.':
			ans = right_last;
			if(right->nullable)
				ans.insert(left_last.begin(), left_last.end());
			break;
		default:
			ans.insert(pos); break;
		}
		lastpos = ans;
		return ans;
	}
};


string add_(string s) // Добавление символа пустого слова в РВ
{
	if(s.length() == 0)
		return "_";
	string res = "";
	for(int i = 0; i < s.length(); i++)
	{
		if(s[i] == '|')
		{
			if(i == 0)
				res += "_";
			else if(s[i-1] == '(')
				res += "_";
			else if(s[i-1] == '|')
				res += "_";
		}
		if(s[i] == ')')
		{
			if(i > 0 && s[i-1] == '|')
				res += "_";
		}
		res += s[i];
	}
	if(s[s.length()-1] == '|')
		res += "_";
	return res;
}

string add_concat(string s) // Добавление символа конкатенации в РВ
{
	set<char> op({'|', '*', ')'});
	if(s.length() == 0)
		return s;
	string res = "";
	for(int i = 0; i < s.length(); i++)
	{
		res += s[i];
		if(i < s.length() - 1 && ((op.find(s[i]) == op.end() && s[i] != '(') || s[i] == ')' || s[i] == '*'))
		{
			if(op.find(s[i+1]) == op.end())
				res += ".";
		}
	}
	return res;
}


int priority(char s) // Приоритет операций в РВ
{
	if(s == '|')
		return 1;
	if(s == '.')
		return 2;
	if(s == '*')
		return 3;
	return 0;
}

string postfix_notation(string s) // Привевдение РВ в постфиксный вид с помощью метода вагонов
{
	string res = "";
	stack<char> st;
	for(int i = 0; i < s.length(); i++)
	{
		if(s[i] == '*' || s[i] == '|' || s[i] == '.')
		{
			while(!st.empty() && priority(st.top()) >= priority(s[i]))
			{
				char op = st.top();
				st.pop();
				res += op;
			}
			st.push(s[i]);
		}
		else if(s[i] == '(')
		{
			st.push(s[i]);
		}
		else if(s[i] == ')')
		{
			while(!st.empty() && st.top() != '(')
			{
				char op = st.top();
				st.pop();
				res += op;
			}
			if(!st.empty())
			{
				st.pop();
			}
		}
		else
		{
			res += s[i];
		}
	}
	while(!st.empty())
	{
		char op = st.top();
		st.pop();
		res += op;
	}
	return res;
}


int poscount(string rv) // Получение позиции последнего символа в РВ
{
	int n = rv.length();
	int pos = 0;
	set<char> op({ '|', '*', '.', '(', ')', '_'});
	for(int i = 0; i < n; i++)
	{
		if (op.find(rv[i]) == op.end())
			pos++;
	}
	return pos;
}

RvNode* buildTree(string s) // Построение синтаксического дерева РВ
{
	std::stack<RvNode*> st;
	int p = poscount(s);
	int n = s.length();
	RvNode* root = new RvNode(s[n-1], p);
	RvNode* tmp = root;
	st.push(root);
	for(int i = n - 2; i >= 0; i--) {
		if(s[i] == '|' || s[i] == '.')
		{
			if(!tmp->right)
			{
				tmp->right = new RvNode(s[i], p);
				tmp = tmp->right;
			}
			else
			{
				tmp->left = new RvNode(s[i], p);
				tmp = tmp->left;
			}
			st.push(tmp);
		}
		else if(s[i] == '*')
		{
			if(!tmp->right)
			{
				tmp->right = new RvNode(s[i], p);
				tmp = tmp->right;
			}
			else
			{
				tmp->left = new RvNode(s[i], p);
				tmp = tmp->left;
			}
		}
		else
		{
			if(!tmp->right)
			{
				tmp->right = new RvNode(s[i], p);
				if(s[i] != '_')
					p--;
			}
			else
			{
				tmp->left = new RvNode(s[i], p);
				if(s[i] != '_')
					p--;
			}
			if(st.top() != root)
			{
				tmp = st.top();
				st.pop();
			}
			else
				tmp = st.top();
		}
	}
	st.pop();
	return root;
}

void findFollow(RvNode* root, map<int, set<int> >& m) // Вычисление значений функции followpos и добавление их в словарь m
{
	if(!root)
		return;
	findFollow(root->left, m);
	findFollow(root->right, m);
	if(root->c == '.')
	{
		for(auto i = m.begin(); i != m.end(); i++)
		{
			int pos = i->first;
			if(root->left->lastpos.find(pos) != root->left->lastpos.end())
			{
				auto lst = root->right->firstpos;
				for(auto j = lst.begin(); j != lst.end(); j++)
				{
					m[pos].insert(*j);
				}
			}
		}
	}
	if(root->c == '*')
	{
		for(auto i = m.begin(); i != m.end(); i++)
		{
			int pos = i->first;
			if(root->right)
			{
				if (root->right->lastpos.find(pos) != root->right->lastpos.end())
				{
					auto lst = root->right->firstpos;
					for (auto j = lst.begin(); j != lst.end(); j++)
					{
						m[pos].insert(*j);
					}
				}
			}
			else
			{
				if(root->left->lastpos.find(pos) != root->left->lastpos.end())
				{
					auto lst = root->left->firstpos;
					for (auto j = lst.begin(); j != lst.end(); j++)
					{
						m[pos].insert(*j);
					}
				}
			}
		}
	}
}

char find_pos(int p, string rv) // Получение символа РВ в позиции p
{
	int n = rv.length();
	int pos = p;
	set<char> op({ '|', '*', '.', '(', ')', '_'});
	for(int i = 0; i < n; i++)
	{
		if(op.find(rv[i]) == op.end())
			pos--;
		if(pos == 0)
			return rv[i];
	}
	return '?';
}

DFA re2dfa(const std::string &s) //Конвертация РВ в ДКА
{
	DFA res = DFA(Alphabet(s));
	auto alph = Alphabet(s);
	if(s.length() == 0)
		return res;
	auto rv = add_("(" + s + ")#");
	auto post = postfix_notation(add_concat(rv));
	auto root = buildTree(post);
	root->setNullable();
	root->setFirst();
	root->setLast();
	int pos = poscount(post);
	map<int, set<int> > followpos;
	for(int i = 1; i <= pos; i++)
	{
		set<int> tmp;
		followpos[i] = tmp;
	}
	findFollow(root, followpos);
	int counter = 1;
	map<set<int>, int> sttoi;
	sttoi[root->firstpos] = counter;
	res.create_state(to_string(counter));
	res.set_initial(to_string(counter));
	if(root->firstpos.find(pos) != root->firstpos.end())
		res.make_final(to_string(counter));
	auto states = res.get_states();
	queue<set<int> > unmarked;
	unmarked.push(root->firstpos);
	counter++;
	map<int, char> char_pos;
	for(int i = 1; i <= pos; i++)
	{
		char_pos[i] = find_pos(i, rv);
	}
	while(!unmarked.empty())
	{
		set<int> R = unmarked.front();
		unmarked.pop();
		for(auto ch : alph)
		{
			set<int> S;
			for(int i = 1; i <= pos; i++)
			{
				if(char_pos[i] == ch && R.find(i) != R.end())
				{
					if(followpos[i].size() != 0)
						S.insert(followpos[i].begin(), followpos[i].end());
				}
			}
			if(S.size() != 0)
			{
				if(sttoi.find(S) == sttoi.end())
				{
					sttoi[S] = counter;
					counter++;
					res.create_state(to_string(sttoi[S]));
					unmarked.push(S);
					if(S.find(pos) != S.end())
						res.make_final(to_string(sttoi[S]));
				}
				res.set_trans(to_string(sttoi[R]), ch, to_string(sttoi[S]));
			}
		}
	}
	return res;
}
