#include <iostream>
#include <string>
#include <list>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <direct.h>
using namespace std;

typedef uint64_t hash_t;

constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

class fileNode;   //文件节点 用来存储文件的信息 并以树的形式构成文件系统

int recent_grade;	//recent_ptr所指向的文件相对于/root的层级（root为0）

class fileNode
{
public:
	fileNode * father;
	list<fileNode> children; string owner;
	string name;
	bool type[3] = { true,true,true };  //rwx权限，默认为true,true,true
	string src;
	int grade;
	int filetype;

	bool changeType()
	{

			string a;
			cout << "输入文件读写状态码，默认状态为-r-w-x(000)" << endl;
			cin >> a;
			if (a[0] = 0)
				type[0] = false;
			else
				type[0] = true;
			if (a[1] = 0)
				type[1] = false;
			else
				type[1] = true;
			if (a[2] = 0)
				type[2] = false;
			else
				type[2] = true;
			return true;
	};
	int fileSize()
	{
		struct stat statbuf;
		stat(name.data(), &statbuf);
		int size = statbuf.st_size;

		return size;
	};
	string print_type()
	{
		string a;
		if (type[0])
			a += "r";
		else
			a += "-r";
		if (type[1])
			a += "w";
		else
			a += "-w";
		if (type[0])
			a += "x";
		else
			a += "-x";
		return a;
}
};

fileNode *recent_ptr;  //目前所在目录
fileNode root;		//文件系统根节点/root

/* hash函数, 为了去掉脱裤子放屁的部分代码 */
hash_t hash_(char const* str)
{
    hash_t ret{basis};
    while(*str)
	{
        ret ^= *str;
        ret *= prime;
        str++;
    }

    return ret;
}

/* 利用hash函数返回自定义文字常量 */
constexpr hash_t hash_compile_time(char const* str, hash_t last_value = basis)
{
    return *str ? hash_compile_time(str+1, (*str ^ last_value) * prime) : last_value;
}

/* 为了美观, 重载operator */
constexpr unsigned long long operator "" _hash(char const* p, size_t)
{
    return hash_compile_time(p);
}

void help()
{
    cout << "------------------------------------------------------------------------\n";
    cout << "函数表:\n";
    cout << "1. touch + file_name 新建文件\n";
    cout << "2. mkdir + folder_name 新建文件夹\n";
    cout << "3. cd + 相对地址 （目前只支持一层）\n";
    cout << "4. dir 等效 ls -l\n";
    cout << "5. open + file_name 打开文件\n";
    cout << "6. write + file_name 编辑文件\n";
    cout << "7. read + file_name 读取文件\n";
    cout << "8. delete + file_name 删除文件/文件夹\n";
    cout << "9. rename + old_name + new_name 重命名文件/文件夹\n";
    cout << "10. exit 退出系统\n";
    cout << "------------------------------------------------------------------------\n";
    cout << endl;
}


//扫描输入的文件是否在当前目录中
bool existRecentFolder(string filename)
{
	for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
		if ((*i).name == filename)
			return true;
	return false;
}

//打开文件
void open(string namein)
{
	if (existRecentFolder(namein))
	{
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
			if ((*i).name == namein && (*i).filetype == 1) //类型判断
			{
				ofstream filestream(namein.data());
				cout << "访问成功!";
			}
			else if ((*i).name == namein && (*i).filetype == 0)
			{
				cout << "不支持对文件夹操作";
				return;
			}
	}
	else
	{
		cout << "未找到文件" ;
	}
}

//读文件
void read(string namein)
{
	if (existRecentFolder(namein))
	{
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
		{
			if ((*i).name == namein && (*i).filetype == 1)
			{
				ifstream filestream(namein.data());
				if (!filestream.is_open())
				{
					cout << "文件打开失败"; exit(1);
				}
				while (!filestream.eof())
				{
					cout << "读取到的内容是:";
					string content;
					getline(filestream, content);
					cout << content;
				}
			}
			else if ((*i).name == namein && (*i).filetype == 0)
			{
				cout << "不支持对文件夹操作" ;
				return;
			}
		}
	}
	else
	{
		cout << "未找到文件";
	}
}

//写文件
void write(string namein)
{
	if (existRecentFolder(namein))
	{
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
		{
			if ((*i).name == namein && (*i).filetype == 1)
			{
				ofstream filestream(namein.data());
				if (filestream.is_open())
				{
					string content;
					cout << "输入要写入的内容:"<<endl ;
					cin >> content;
					filestream << content ;
					filestream.close();
					getchar();
					cout << "写入成功！";
				}
			}
			else if ((*i).name == namein && (*i).filetype == 0)
			{
				cout << "不支持对文件夹操作";
				return;
			}
		}
	}
	else
		cout << "未找到文件" ;
}

//新建文件
void creatFile(string name)
{
	fileNode IN;
	if (!existRecentFolder(name))
	{
		IN.father = recent_ptr;
		IN.name = name;
		IN.src = recent_ptr->src + "/" + name;
		IN.grade = recent_grade + 1;
		IN.filetype = 1;
		IN.type[0] = true;
		IN.type[1] = true;
		IN.type[2] = true;
		recent_ptr->children.push_back(IN);
		ofstream filestream(name.data());
		filestream.close();
		cout << "创建成功！";
	}
	else
	{
		cout << "命名冲突" ;
	}
}

//新建文件夹
void creatFolder(string name)
{

	fileNode IN;

	if (!existRecentFolder(name))
	{
		IN.father = recent_ptr;
		IN.name = name;
		IN.src = recent_ptr->src + "/" + name;
		IN.grade = recent_grade + 1;
		IN.filetype = 0;
		IN.type[0] = true;
		IN.type[1] = true;
		IN.type[2] = false;
		recent_ptr->children.push_back(IN);
		if (mkdir(name.data()) == 0)
		{
			cout << "创建成功！";
		}
		else
		{
			cout << "创建失败";
		}
	}
	else
	{
		cout << "命名冲突" ;
		return;
	}
}

//初始化
void init()
{
	//创建根目录
	{
		root.owner = "all";
		root.filetype = 0;
		root.src = "./root";
		root.grade = 0;
		root.name = "root";
		system("mkdir root");
		chdir("root");
		recent_grade = root.grade;
		recent_ptr = &root;
	}
}


//cd命令（只实现了一层目录）
void cd(string address)
{
	bool file_exist = false, is_folder = false;
	list<fileNode>::iterator a;

	if (address == "..")
	{
		recent_grade--;
		recent_ptr = recent_ptr->father;
		chdir("..");
	}
	else if (address == ".");
	else
	{
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
		{
			if ((*i).name == address)
			{
				file_exist = true;
				if ((*i).filetype == 0)
				{
					is_folder = true;
					a = i;
					break;
				}
				else
				{
					is_folder = false;
				}
			}
		}
		if (file_exist && is_folder)
		{
			recent_grade++;
			chdir(address.data());
			recent_ptr = &(*a);
		}
		else if (file_exist && !is_folder)
		{
			cout << "不支持对文件操作";
			return;
		}
		else if (!file_exist)
		{
			cout << "未找到文件夹" ;
			return;
		}
	}
	cout << "切换成功!";
}

//ls
void dir()
{
	if (recent_ptr->children.size() != 0)
	{
		cout << left << setw(18) << "文件名" << left << setw(25) << "文件所有者" << left << setw(36) << "文件读写类型" << left << setw(24) << "文件地址" << left << setw(19) << "文件大小" << endl;
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
			cout << left << setw(15) << (*i).name << left << setw(20) << (*i).owner << left << setw(30) << (*i).print_type() << left << setw(20) << (*i).src << left << setw(15) << (*i).fileSize() << endl;
	}
	else
	{
		cout << "空目录";
	}
}

void copyFile(string filename_and_path)
{
	
}

//删除文件
void deleteFile(string namein)
{
	if (existRecentFolder(namein))
	{
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
		{
			if ((*i).name == namein)
			{
				recent_ptr->children.erase(i);
				string command = "rm -rf " + namein;
				system(command.data());
				cout << "删除成功!";
				return;
			}
		}
	}
	else
	{
		cout << "权限检查失败" ;
		return;
	}
}

// 重命名文件
void renameFile(string option)
{
    size_t spacePos = option.find(' ');
    if (spacePos != string::npos)
    {
        string oldName = option.substr(0, spacePos);
        string newName = option.substr(spacePos + 1);

        if (existRecentFolder(oldName))
        {
            for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
            {
                if ((*i).name == oldName)
                {
                    (*i).name = newName;
                    (*i).src = recent_ptr->src + "/" + newName;
                    string command = "mv " + oldName + " " + newName;
                    system(command.data());
                    cout << "重命名成功!" << endl;
                    return;
                }
            }
        }
        else
        {
            cout << "文件不存在" << endl;
        }
    }
    else
    {
        cout << "重命名命令格式错误, 应为: rename old_name new_name" << endl;
    }
}

//退出
void exit()
{
	for (int i= recent_grade+1; i != 0; i--)
	{
		chdir("..");
	}

	// system("rm -rf root");
	cout << "目录删除成功!即将退出系统."<<endl;
	exit(0);
}
//命令解释
void shell()
{
	int orders;
	string order, option, command;
	cout << ">";
	getline(cin, order);
	for (auto i = 0; i != order.size(); i++)
	{
		if (order[i] == ' '&&i != order.size() - 1)
		{
			command = order.substr(0, i);
			option = order.substr(i + 1, order.size());
			break;
		}
		else if (i == order.size() - 1)
		{
			command = order;
			break;
		}
	}

	/* cpp不让string直接用switch-case */
	switch (hash_(command.c_str()))
	{
		case "help"_hash:
			help();
			break;
		case "touch"_hash:
			creatFile(option);
			break;
		case "mkdir"_hash:
			creatFolder(option);
			break;
		case "cd"_hash:
			cd(option);
			break;
		case "dir"_hash:
			dir();
			break;
		case "open"_hash:
			open(option);
			break;
		case "write"_hash:
			write(option);
			break;
		case "read"_hash:
			read(option);
			break;
		case "delete"_hash:
			deleteFile(option);
			break;
		case "rename"_hash:
			renameFile(option);
			break;
		case "exit"_hash:
			exit();
			break;
		case "copy"_hash:
			copyFile(option);
			break;
		default:
			cout << "无效命令" ;
			break;
	}
	cout << endl;
}

int main()
{
	init();
	cout << "输入help查看命令" << endl;
	while (true)
	{
		shell();
	}
	return 0;
}
