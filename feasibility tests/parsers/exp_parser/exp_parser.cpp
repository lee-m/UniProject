// exp_parser.cpp : Defines the entry point for the console application.
//

#include <stack>
#include <iostream>
#include <string>
using namespace std;

//operator precedence
#define ADD 0
#define SUBTRACT 0
#define MULTIPLY 1
#define DIVIDE 1
#define PAREN -1

int ConvertOp(string Op)
{
	if(Op == "+")
		return ADD;

	if(Op == "-")
		return SUBTRACT;

	if(Op == "*")
		return MULTIPLY;

	if(Op == "/")
		return DIVIDE;

	if(Op == "(")
		return PAREN;
}

bool IsDigit(string Val)
{
	return Val == "0" || Val == "1" || Val == "2"
				 || Val == "3" || Val == "4" || Val == "5"
				 || Val == "6" || Val == "7" || Val == "8"
				 || Val == "7" || Val == "8" || Val == "9";
}

//reduce the expression on top of the stack
void Reduce(stack<float> &Vals, stack< pair<int, string> > &Ops)
{
	float rhs = Vals.top();
	Vals.pop();

	float lhs = Vals.top();
	Vals.pop();

	pair<int, string> op = Ops.top();
	Ops.pop();

	if(op.second == "+")
		Vals.push(lhs + rhs);

	if(op.second == "*")
		Vals.push(lhs * rhs);

	if(op.second == "-")
		Vals.push(lhs - rhs);

	if(op.second == "/")
		Vals.push(lhs / rhs);
}

float EvaluateExpression(const string &Exp, stack<float> &Vals, 
												 stack< pair<int, string> > &Ops);

int main(int argc, char* argv[])
{
	string InputExp; //= "5+(4*(4+1))";

	cout << "enter an expression (single digits only): ";
  cin >> InputExp;

	stack<float> ValStack;
	stack< pair<int, string> > OpStack;

	cout << "value of the expression is " 
			 << EvaluateExpression(InputExp, ValStack, OpStack) << endl;

	system("pause");

	return 0;
}

float EvaluateExpression(const string &Exp, stack<float> &Vals, 
												 stack< pair<int, string> > &Ops)
{
	int CurrExpPos = 0;
	string Temp;

	while(CurrExpPos != Exp.length())
	{
		Temp = Exp.substr(CurrExpPos++, 1);

		//if it's a digit, shift
		if(IsDigit(Temp))
		{
			Vals.push((float)atof(Temp.c_str()));
			continue;
		}

		//handle the negation operator specially
		if(Temp == "-" && IsDigit(Exp.substr(CurrExpPos, 1)))
		{
			Vals.push((float)atof(Exp.substr(CurrExpPos - 1, 2).c_str()));

			//incremement the position so we skip over the digit
			++CurrExpPos;
			continue;
		}

		if(Temp == ")")
		{
			while(Ops.top().second != "(")
				Reduce(Vals, Ops);
			Ops.pop();
			continue;
		}

		//we must have an operator, if the parsed operator's precedence is greater
		//than the current operator on top of the stack, reduce, otherwise shift
		if(Ops.empty()
			 || Ops.top().first < ConvertOp(Temp)
			 || Temp == "(")
		{
			Ops.push(make_pair(ConvertOp(Temp), Temp));
			continue;
		}
		else
		{
			//keep reducing the stack until we reach an operator of lesser precedence
			//or we've exhausted the stack
			while(Ops.top().first >= ConvertOp(Temp))
			{
				Reduce(Vals, Ops);

				if(Ops.empty())
					break;
			}

			Ops.push(make_pair(ConvertOp(Temp), Temp));
		}
	}

	//evaluate the expression
	while(Vals.c.size() != 1)
		Reduce(Vals, Ops);

	return Vals.top();
}