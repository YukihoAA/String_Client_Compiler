#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>

using namespace std;

/*
	string_client_c.dat에 대해한 파싱 규칙

	ID문자열은 0a xx 0a xx 다음에 시작하지만 가끔 0a xx yy 0x xx (yy는 80 이하의 숫자)로 시작하는 경우가 있다.

	yy는 d로 저장된다.

	xx는 0a가 와서는 안된다. 0a0a는 str의 내용이기 때문이다.

	두번째 xx (b)는 ID 문자열의 길이이다.

	str문자열은 12 xx 으로 시작하지만 가끔 12 xx zz (zz는 20이하의 숫자)로 시작하는 경우가 있다.

	str문자열에는 개행이 있을 수 있기 때문에 반드시 개행이 두번 되었는지 체크하고 다음항목으로 진행해야한다.

	12다음의 xx (c)는 str 문자열의 길이(멀티바이트기준)로 생각된다.
*/

int main(int argc, char *argv[])
{
	FILE *fp=NULL, *txt = NULL;
	int a, b, c, i, d, e, tmp;
	char id[200], str[2000], sel;
	if (argc == 1 || !strcmp(argv[1], "help"))
	{
		cout << "\tUsage: scc [mode] \"string_client_c.dat Path\" \".txt Path\"" << endl << endl;
		cout << "\tmode:\tcompile \t- .txt to dat" << endl;
		cout << "\t\tdecompile \t- .dat to .txt" << endl;
	}
	else if (argc == 4 && !strcmp(argv[1], "decompile"))
	{
		cout << endl << ">> WARNING: this is DEcompile mode. this will destroy your work data if .txt file Exists.<<" << endl << endl << "Do you want to continue? (y/n) :";

		cin >> sel;

		if (sel != 'y')
		{
			cout << "exits..." << endl;
			return 0;
		}
		try {
			fp = fopen(argv[2], "rb");
			if (!fp)		//파일이 없다면
			{
				throw "ERR: No .dat File";
			}
			txt = fopen(argv[3], "wb+");
			if (!txt)		//파일생성이 불가능하다면
			{
				throw "ERR: Cannot create new .txt file";
			}

			if (fgetc(fp) != 0x0a)		//시작은 무조건 0a
			{
				throw "ERR: invaild string_client_c.dat file";
			}

			for (; !feof(fp);)
			{
				a = 0, b = 0, c = 0, d=0, e=0, i=0;
				a=fgetc(fp);	//0a 다음의 숫자는 a로 받는다
				if (a == 0x0a)
					throw "ERR: invaild string_client_c.dat file";
				d = fgetc(fp);	//가끔 0a가 바로 오지 않고 80이하의 숫자가 나오는 경우가 있다. 이 경우는 d로 저장
				if (d != 0x0a)
				{
					if (d >= 0x80 || fgetc(fp) != 0x0a)
					{
						throw "ERR: invaild string_client_c.dat file";
					}
				}
				b = fgetc(fp);	//0a 다음 숫자를 받는다
				for (i=0; !feof(fp);i++)
				{
					if (i >= 200)
						throw "ERR: Need more ID string buffer in source code";
					id[i] = fgetc(fp);
					if (id[i] == 0x12)	//12가 나오면 str문자열로
					{
						id[i] = 0;
						c = fgetc(fp);	//12 다음숫자는 c로
						e = fgetc(fp);	//가끔 20이하의 숫자가 추가로 작성된다.
						if (e >= 0x20)
						{
							fseek(fp, -1, SEEK_CUR);
							e = 0;
						}
						break;
					}
					else if (id[i] == 0x0a)	//ID에는 개행이 없다. 즉, str이 공란인경우.
					{
						id[i] = 0;
						fseek(fp, -1, SEEK_CUR);
						break;
					}
				}
					for (i = 0; !feof(fp); i++)
					{
						if (i >= 2000)
							throw "ERR: Need more str string buffer in source code";
						str[i] = fgetc(fp);
						if (str[i]==EOF)
						{
							str[i] = 0;
							break;
						}
						else if (str[i] == 0x0a)	//개행인지 판단
						{
							tmp = 0;
							if (fgetc(fp) == 0x0a)	//str의 내용일경우
							{
								fseek(fp, -1, SEEK_CUR);
								continue;
							}
							tmp = fgetc(fp);
							if (tmp == 0x0a)	//다음 항목인가
							{
								fgetc(fp);
								if (fgetc(fp) == 0x0a)
								{
									fseek(fp, -4, SEEK_CUR);
									continue;
								}
								str[i] = 0;
								fseek(fp, -4, SEEK_CUR);
								break;
							}
							else if (tmp < 0x80)	//다음 항목인가 판단
							{
								if (fgetc(fp) == 0x0a)
								{
									str[i] = 0;
									fseek(fp, -3, SEEK_CUR);
									break;
								}
								fseek(fp, -1, SEEK_CUR);
							}
							fseek(fp, -2, SEEK_CUR);	//다음줄 아니였어 그러면 다시 돌려놓고 계속 입력받기
						}
					}
					fprintf(txt, "%d %d %d %d %d %s", a, b, c, d, e, id);
					fputc(0x0a, txt);
					fprintf(txt, "%s", str);
					fputc(0x0a, txt);
					fputc(0x0a, txt);
			}
			cout << endl << "decompile finished." << endl;
			cout << endl << "<!--Important--!>" << endl;
			cout << "You should edit string in same byte (this is multibyte char data)" << endl;
			cout << "You shouldn't edit binary number data in file (it will corrupt data.jmp)" << endl;
		}
		catch (char *e) {
			cout << e << endl;
		}
		catch (...) {
		}
		if(fp)
			fclose(fp);
		if(txt)
			fclose(txt);
	}
	else if (argc == 4 && !strcmp(argv[1], "compile"))
	{
		cout << endl << "info: this is compile mode. this will overwite if .dat file Exists." << endl << endl << "Do you want to continue? (y/n) :";

		cin >> sel;

		if (sel != 'y')
		{
			cout << "exits..." << endl;
			return 0;
		}
		try {
			txt = fopen(argv[3], "rb");
			if (!txt)		//파일이 없다면
			{
				throw "ERR: No .txt File";
			}
			fp = fopen(argv[2], "wb+");
			if (!fp)		//파일생성이 불가능하다면
			{
				throw "ERR: Cannot create new .dat file";
			}

			for (; !feof(txt);)
			{
				a = 0, b = 0, c = 0, d = 0, e = 0, i = 0;
				fscanf(txt, "%d %d %d %d %d %[^\n]", &a, &b, &c, &d, &e, id);
				fgetc(txt);	// 0a제거
				cout << a << b << c << d << e << id << endl;
				
				for (int i=0; !feof(txt); i++)
				{
					if (i >= 2000)
						throw "ERR: Need more str string buffer in source code";
					str[i] = fgetc(txt);
					if (str[i] == 0x0a)
					{
						if (fgetc(txt) == 0x0a)
						{
							tmp = fgetc(txt);
							if (feof(txt))
							{
								str[i] = 0;
								break;
							}
							if (tmp >= '0' && tmp <= '9')
							{
								fseek(txt, -1, SEEK_CUR);
								str[i] = 0;
								break;
							}
							fseek(txt, -1, SEEK_CUR);
						}
						fseek(txt, -1, SEEK_CUR);
					}
				}
				fputc(0x0a, fp);
				fputc(a, fp);
				if (d != 0x0a)
					fputc(d, fp);
				fputc(0x0a, fp);
				fputc(b, fp);
				
				for (int i=0;;i++)
				{
					if (id[i] == 0)
						break;
					fputc(id[i], fp);
				}
				if (str[0] != 0)
				{
					fputc(0x12, fp);
					fputc(c, fp);
					if (e != 0)
						fputc(e, fp);
					for (int i = 0;; i++)
					{
						if (str[i] == 0)
							break;
						fputc(str[i], fp);
					}
				}
			}

		}
		catch (char *e) {
			cout << e << endl;
		}
		catch (...) {
		}
		if (fp)
			fclose(fp);
		if (txt)
			fclose(txt);
	}
	else
	{
		cout << "Check Usage: scc -help" << endl;
	}
	return 0;
}