#pragma once
#include <mysql/mysql.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

#define SQL_HOST "localhost"
#define SQL_USER "ik"
#define SQL_PASSWORD "123456"
#define SQL_DATEBASE "bigdata"

class DateBase
{
protected:
	DateBase()
	{
		connect_ = mysql_init(nullptr);
		if (mysql_real_connect(connect_, SQL_HOST, SQL_USER, SQL_PASSWORD, SQL_DATEBASE, 0, nullptr, 0) == nullptr)
		{
			exit(0);
		}
	}

	~DateBase()
	{
		mysql_close(connect_);
	}

	//向table表中增加value_vec的信息
	void sql_add(string table,vector<string>& value_vec)
	{
		string add_com = "INSERT INTO ";
		add_com += table;
		add_com += " VALUES (";

		for (int i=0;i<value_vec.size();i++)
		{
			add_com = add_com + value_vec[i].c_str();
			if(i!=value_vec.size()-1)
			{
				add_com += ",";
			}
		}
		add_com = add_com + ");";


		if (general_operation(add_com.c_str()) == 0)
		{
			cout << "sql add error!" << endl;
		}
	}

	//在table表中查询满足cond条件的信息
	MYSQL_RES* sql_select(string table, string cond = "")
	{
		string sel_com = "SELECT * FROM ";
		sel_com += table;
		if (cond.empty())
		{
			sel_com += " ;";
		}
		else
		{
			sel_com = sel_com + " WHERE " + cond + " ;";
		}
		//cout << sel_com << endl;
		MYSQL_RES* res = select_operation(sel_com.c_str());
		if (res == nullptr)
		{
			cout << "sql select error!" << endl;
		}

		return res;
	}

	//得到table表中的行数
	int sql_get_rownumber(string table)
	{
		string sql = "SELECT COUNT(*) FROM ";
		sql += table;
		sql += ";";
		MYSQL_RES* res = select_operation(sql.c_str());
		if (res == nullptr)
		{
			cout << "get row number error!" << endl;
		}
		
		MYSQL_ROW row = mysql_fetch_row(res);
		string ret = row[0];

		return atoi(ret.c_str());
	}

	//得到table表中满足conditions条件的行数
	int sql_get_rownumber(string table,string conditions)
	{
		string sql = "SELECT COUNT(*) FROM ";
		sql += table;
		if (!conditions.empty())
		{
			sql = sql + " WHERE " + conditions + ";";
		}
		//cout << sql << endl;
		MYSQL_RES* res = select_operation(sql.c_str());
		if (res == nullptr)
		{
			cout << "get row number error!" << endl;
		}
		
		MYSQL_ROW row = mysql_fetch_row(res);
		string ret = row[0];

		return atoi(ret.c_str());
	}

	//得到table表中先筛选出满足conditions条件的信息，再根据group来继续计数统计
	vector<pair<string,int>> sql_get_rownumber_bygroup(string table,string group, string conditions = "")
	{
		string sql = "SELECT ";
		sql += group;
		sql+=" ,COUNT(*) FROM ";
		sql += table;
		if (!conditions.empty())
		{
			sql = sql + " WHERE " + conditions;
		}
		
		sql += " GROUP BY ";
		sql = sql + group + ";";
		
		MYSQL_RES* res = select_operation(sql.c_str());
		if (res == nullptr)
		{
			cout << "get row number by group error!" << endl;
		}
		
		vector<pair<string,int>> vec;
		while (MYSQL_ROW row = mysql_fetch_row(res))
		{
			string first = row[0];
			string second = row[1];
			vec.push_back(pair<string, int>(first, atoi(second.c_str())));
		}

		return vec;
	}

	//删除table表中满足cond条件的信息
	void sql_delete(string table, string cond = "")
	{
		string del_com = "DELETE FROM ";
		del_com = del_com + table;
		if (cond.empty())
		{
			del_com = del_com + " WHERE " + cond + ";";
		}
		else
		{
			return;
		}

		if (general_operation(del_com.c_str()) == 0)
		{
			cout << "sql delete error!" << endl;
		}
	}

	//在满足cond条件下向table表中更新str代表的信息
	void sql_update(string table, string str, string cond = "")
	{
		string update_com = "UPDATE ";
		update_com = update_com + table + " SET " + str;
		if (cond.empty())
		{
			update_com += " ;";
		}
		else
		{
			update_com = update_com + " WHERE " + cond + ";";
		}
		
		if (general_operation(update_com.c_str()) == 0)
		{
			cout << "sql update error!" << endl;
		}
	}

public:
	//打印查询到的输出结果
	void print_res(MYSQL_RES* res)
	{
		while (MYSQL_ROW row = mysql_fetch_row(res))
		{
			for (int i = 0; i < mysql_num_fields(res); i++) {
				cout << row[i] << "  ";
			}
			cout << endl;
		}
	}

	//释放查询的数据结构
	void free_res(MYSQL_RES* res)
	{
		mysql_free_result(res);
	}
private:
	//select 选择操作
	MYSQL_RES* select_operation(const char* str)
	{
		if (mysql_real_query(connect_, str, strlen(str)) != 0)
		{
			cout << "select operation error!" << endl;
			return nullptr;
		}
		return mysql_store_result(connect_);
	}

	//通用操作
	bool general_operation(const char* str)
	{
		bool flag = true;
		if (mysql_real_query(connect_, str, strlen(str)) != 0)
		{
			flag = false;
		}
		return flag;
	}

private:
	MYSQL* connect_;
};