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
#include <filesystem> // 需要包含这个头文件来使用std::filesystem

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
	list<fileNode> children;
	string name;
	string src;
	int grade;
	int filetype; // 文件类型，0为文件夹，1为文件
	int fileSize()
	{
		struct stat statbuf;
		stat(name.data(), &statbuf);
		int size = statbuf.st_size;

		return size;
	};
	
	// 保存文件系统状态，用于持续化存储
	void saveToFile(ofstream &outfile) {
		outfile << src << " " << filetype << " " << endl;
		for (auto &child : children) {
			child.saveToFile(outfile);
		}
	}

	// 加载文件系统状态
	void loadFromFile(ifstream &infile) {
		string line;
		while (getline(infile, line)) {
			stringstream ss(line);
			string src;
			int filetype;
			ss >> src >> filetype;
			fileNode node;
			node.src = src;
			node.name = src.substr(src.find_last_of("/") + 1);
			node.filetype = filetype;
			node.father = this;
			children.push_back(node);
			if (filetype == 0) { // 文件夹
				children.back().loadFromFile(infile);
			}
		}
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
	cout << "10. copy + source_file_name + target_file_name\n";
	cout << "11. mov + source_file_name + target_file_name\n";
    cout << "12. exit 退出系统\n";
    cout << "------------------------------------------------------------------------\n";
    cout << endl;
}



/* 使用 access 函数来判断文件是否存在 */
bool existRecentFolder(string filename)
{
	return access(filename.data(), F_OK) == 0;
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
				cout << "访问成功!\n";
			}
			else if ((*i).name == namein && (*i).filetype == 0)
			{
				cout << "不支持对文件夹操作\n";
				return;
			}
	}
	else
	{
		cout << "未找到文件\n" ;
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
					cout << "文件打开失败\n"; exit(1);
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
				cout << "不支持对文件夹操作\n" ;
				return;
			}
		}
	}
	else
	{
		cout << "未找到文件\n";
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
					cout << "写入成功！\n";
				}
			}
			else if ((*i).name == namein && (*i).filetype == 0)
			{
				cout << "不支持对文件夹操作\n";
				return;
			}
		}
	}
	else

	{
		cout << "未找到文件\n" ;
	}

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
		recent_ptr->children.push_back(IN);
		ofstream filestream(name.data());
		filestream.close();
		cout << "创建成功！\n";
	}
	else
	{
		cout << "命名冲突\n" ;
	}
}

void moveFile(string name_and_path)
{
	string source_file, target_file;
	for (auto i = 0; i < name_and_path.size(); i++)
	{
		if (name_and_path[i] == ' ')
		{
			source_file = name_and_path.substr(0, i);
			target_file = name_and_path.substr(i + 1);
			break;
		}
	}
	if (!existRecentFolder(source_file))
	{
		cout << "未找到文件" ;
		return;
	}
	if (existRecentFolder(target_file))
	{
		string I;
		cout << ">已存在同名文件, 是否覆盖<y, n>:";
		cin >> I;
		if (I != "y")
		{
			return;
		}
	}
	// 使用system 函数移动文件
	if (system(("mv " + source_file + " " + target_file).data()) == 0)
	{
		cout << "移动成功";
	}
	else
	{
		cout << "移动失败";
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
		recent_ptr->children.push_back(IN);
		if (mkdir(name.data()) == 0)
		{
			cout << "创建成功！\n";
		}
		else
		{
			cout << "创建失败\n";
		}
	}
	else
	{
		cout << "命名冲突\n" ;
		return;
	}
}
// 初始化
void init() {
    // 创建根目录
    {
        root.filetype = 0;
        root.src = "./root";
        root.grade = 0;
        root.name = "root";
        if (mkdir("root") != 0) {
            cout << "根目录已存在，尝试加载文件系统状态\n" << endl;
            chdir("root");
            ifstream infile("file-system.txt");
            if (infile.is_open()) {
                root.loadFromFile(infile);
                infile.close();
            } else {
                cout << "无法打开文件系统状态文件，或者file-system.txt文件不存在" << endl;
            }
        } else {
            chdir("root");
        }
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
			cout << "不支持对文件操作\n";
			return;
		}
		else if (!file_exist)
		{
			cout << "未找到文件夹\n" ;
			return;
		}
	}
	cout << "切换成功!\n";
}

//ls
void dir()
{
	if (recent_ptr->children.size() != 0)
	{
		cout << left << setw(18) << "文件名" << left << setw(24) << "文件地址" << left << setw(19) << "文件大小" << endl;
		for (auto i = recent_ptr->children.begin(); i != recent_ptr->children.end(); i++)
			cout << left << setw(15) << (*i).name << left << setw(20) << (*i).src << left << setw(15) << (*i).fileSize() << endl;
	}
	else
	{
		cout << "空目录\n";
	}
}

/* :"源文件路径 目的路径" */
void copyFile(string filename_and_path)
{
	string source_file, target_file;
	for (auto i = 0; i < filename_and_path.size(); i++)
	{
		if (filename_and_path[i] == ' ')
		{
			source_file = filename_and_path.substr(0, i);
			target_file = filename_and_path.substr(i + 1);
			break;
		}
	}
	if (!existRecentFolder(source_file))
	{
		cout << "源文件不存在\n" ;
		return;
	}
	if (!existRecentFolder(target_file))
	{
		string I;
		cout << ">目标文件已存在," ;
		cout << "是否覆盖<y, n>:" ;
		cin >> I;
		if (I != "y")
		{
			return;
		}
	}
	string command = "copy " + filename_and_path;
	system(command.data());
	cout << "复制成功!\n";
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
				cout << "删除成功!\n";
				return;
			}
		}
	}
	else
	{
		cout << "权限检查失败\n" ;
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
                    string command = "rename " + oldName + " " + newName;
                    system(command.data());
                    cout << "重命名成功!\n" << endl;
                    return;
                }
            }
        }
        else
        {
            cout << "文件不存在\n" << endl;
        }
    }
    else
    {
        cout << "重命名命令格式错误, 应为: rename old_name new_name\n" << endl;
    }
}

//退出
void exit() {
	// 调用文件系统存储，保存文件系统状态
    ofstream outfile("file-system.txt");
    if (outfile.is_open()) {
        root.saveToFile(outfile);
        outfile.close();
    } else {
        cout << "无法打开文件来保存文件系统状态 或未找到file-system.txt\n" << endl;
    }
    
    for (int i = recent_grade + 1; i != 0; i--) {
        chdir("..");
    }

    // system("rm -rf root");
    cout << "目录删除成功!即将退出系统.\n" << endl;
    exit(0);
}

//命令解释
void shell() {
    while (1) {
        string line;
        cout << recent_ptr->src << " > "; // 显示当前位置路径
        getline(cin, line);

        size_t len = line.size();
        if (len == 0) continue;

        size_t first_space = line.find(' ');
        string command = line.substr(0, first_space);
        string arg = (first_space == string::npos) ? "" : line.substr(first_space + 1);

        switch (hash_(command.c_str())) {
        case "help"_hash:
            help();
            break;
        case "touch"_hash:
            creatFile(arg);
            break;
        case "mkdir"_hash:
            creatFolder(arg);
            break;
        case "open"_hash:
            open(arg);
            break;
        case "read"_hash:
            read(arg);
            break;
        case "write"_hash:
            write(arg);
            break;
        case "delete"_hash:
            deleteFile(arg);
            break;
        case "cd"_hash:
            cd(arg);
            break;
        case "dir"_hash:
            dir();
            break;
        case "rename"_hash:
            renameFile(arg);
            break;
        case "copy"_hash:
            copyFile(arg);
            break;
        case "exit"_hash:
            return;
        default:
            cout << "未知命令, 输入help查看命令列表\n";
        }
    }
}


int main()
{
	init();
	cout << "输入help查看命令\n" << endl;
	while (true)
	{
		shell();
	}
	return 0;
}
