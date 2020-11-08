#include <iostream>
#include <cmath>
#include <locale.h>
#include "Pic.h"
#include "AI.h"

using namespace std;

int main(int argc, TCHAR* argv[])
{
	
	setlocale(0, "");
	Picture PicData;
	Brain Bra;
	float SpeedSetting = 0.05;
	float speedLearn = 0.05;

	while (true) {
		system("cls");
		int command = 0;
		std::cout << "НЕЙРОННАЯ СЕТЬ" << endl;
		std::cout << "1.Создать сеть				2.Запустить сеть" << endl;
		std::cout << "3.Обучить при помощи датасета		4.Вывести сеть" << endl;
		std::cout << "5.Очистить сеть				6.настройки нейросети" << endl;
		std::cout << "7.Выбрать изображение			8.вывести журнал в текстовом виде" << endl;
		std::cout << "9.Проверить точки			10.Загрузить сеть" << endl;
		std::cout << "11.Сохранить сеть			12.выход" << endl;
		std::cout << "13.текущий путь" << endl;
		cin >> command;
		if (command == 12) {
			Bra.Clear();
			break;
		}
		switch (command) {
		case 1: {
			if (Bra.layers.size() > 0) {
				std::cout << "нейроная сеть уже создана!" << endl;
				break;
			}
			system("cls");
			int klayer = 0;
			std::cout << "количество скрытых слоёв: "; cin >> klayer;
			for (int i = 0; i < klayer; i++) {
				layer buff;
				ActFun b;
				b.b = 0;
				int k = 0;
				std::cout << "количество нейронов в слое " << i + 1 << ": ";
				cin >> k;
				for (int j = 0; j < k; j++) buff.ActFuns.push_back(b);
				Bra.addL(buff);
			}
			bool finish = Bra.build();
			if (finish) std::cout << "нейрон собрался!" << endl;
			else std::cout << "нейрон не собрался!" << endl;
			break;
		}
		case 9: {
			PicData.Debug(4);
			break;
		}
		case 7: {
			char current_work_dir[FILENAME_MAX];
			_getcwd(current_work_dir, sizeof(current_work_dir));
			string dir = current_work_dir;
			dir = dir + "\\DataPhoto\\*.*";
			wstring DIRws;
			for (int i = 0; i < dir.length(); ++i) DIRws += wchar_t(dir[i]);
			const wchar_t* ResultDIR = DIRws.c_str();

			WIN32_FIND_DATA FindFileData;
			HANDLE hf;
			hf = FindFirstFile(ResultDIR, &FindFileData);

			if (hf != INVALID_HANDLE_VALUE) {
				vector<string> SaveName;
				cout << "0.Назад" << endl;
				int k = 1;
				do {
					wstring ws(FindFileData.cFileName);
					string str(ws.begin(), ws.end());
					if (str[0] == '.') continue;
					std::cout << k << '.' << str << "\n";
					SaveName.push_back(str);
					k++;
				} while (FindNextFile(hf, &FindFileData) != 0);
				FindClose(hf);
				while (true) {
					cout << "Выберите изображение" << endl;
					int  ChoiceSave = 0; cin >> ChoiceSave;
					if (ChoiceSave == 0) break;
					if (ChoiceSave > 0 && ChoiceSave <= SaveName.size()) {
						string Name = *(SaveName.begin() + ChoiceSave - 1);
						string path = "DataPhoto\\" + Name;
						PicData.Create(path);
						PicData.parsing();
						break;
					}
					cout << "Выбор выходит за пределы" << endl;
					system("pause");
					system("cls");
				}
			}
			break;
		}
		case 2: {
			if (!PicData.Check()) { std::cout << "ОШИБКА: отсутствует изображение" << endl; break; }
			std::cout << "введите номер строки: "; int col; cin >> col;
			std::cout << "введите номер столбца: "; int row; cin >> row;
			Mat rating = PicData.GetCut(row, col);
			vector<neuron> Input;
			PicVec(rating, Input);
			Bra.start(Input);
			Input.clear();
			layer& link = *(Bra.layers.end() - 1);
			int i = 0, iMax = 0;
			float Max = 0;
			for (vector<ActFun>::iterator it = link.ActFuns.begin(); it != link.ActFuns.end(); it++) {
				if (it->x > Max) {
					Max = it->x;
					iMax = i;
				}
				i++;
			}
			if (iMax > 1) std::cout << "это оценка - " << iMax << endl;
			else (iMax == 0) ? std::cout << "это пустое поле" << endl : std::cout << "это нб" << endl;
			namedWindow("original", WINDOW_NORMAL);
			imshow("original", rating);
			waitKey();
			break;
		}
		case 3: {
			if (!PicData.Check()) { std::cout << "ОШИБКА: отсутствует изображение" << endl; break; }

			char current_work_dir[FILENAME_MAX];
			_getcwd(current_work_dir, sizeof(current_work_dir));
			string dir = current_work_dir;
			dir = dir + "\\DataAnswer\\*.txt";
			wstring DIRws;
			for (int i = 0; i < dir.length(); ++i) DIRws += wchar_t(dir[i]);
			const wchar_t* ResultDIR = DIRws.c_str();

			WIN32_FIND_DATA FindFileData;
			HANDLE hf;
			hf = FindFirstFile(ResultDIR, &FindFileData);

			int ch[25][15]; bool flag = false;
			if (hf != INVALID_HANDLE_VALUE) {
				vector<string> SaveName;
				cout << "0.Назад" << endl;
				int k = 1;
				do {
					wstring ws(FindFileData.cFileName);
					string str(ws.begin(), ws.end());
					std::cout << k << '.' << str << "\n";
					SaveName.push_back(str);
					k++;
				} while (FindNextFile(hf, &FindFileData) != 0);
				FindClose(hf);
				while (true) {
					cout << "Выберите файл с ответами" << endl;
					int  ChoiceSave = 0; cin >> ChoiceSave;
					if (ChoiceSave == 0) break;
					if (ChoiceSave > 0 && ChoiceSave <= SaveName.size()) {
						string Name = *(SaveName.begin() + ChoiceSave - 1);
						string path = "DataAnswer\\" + Name;
						char answer[25][15];
						ifstream fin(path);
						for (int r = 0; r < 25; r++) fin.getline(answer[r], 16, '\n');
						fin.close();
						for (int i = 0; i < 25; i++) {
							for (int j = 0; j < 15; j++) { ch[i][j] = ToInt(answer[i][j]); }
						}
						flag = true;
						break;
					}
					cout << "Выбор выходит за пределы" << endl;
					system("pause");
					system("cls");
				}
			}
			if (!flag) break;
			std::cout << "сколько раз нейрон будет обучаться?" << endl;
			int learnC = 0; cin >> learnC;
			float CE = 0, MaxCE = 0, prevCE = 1;
			int a = 0, b = 0;

			for (int i = 0; i < learnC; i++) {
				if (b >= 15) { a++; b = 0; }
				if (a >= 25) a = 0;
				Mat ROI = PicData.GetCut(b, a);
				vector<neuron> Input;
				PicVec(ROI, Input);
				Bra.start(Input);

				layer& link = *(Bra.layers.end() - 1);
				int m = 0, mMax = 1;
				float Max = 0;
				for (vector<ActFun>::iterator it = link.ActFuns.begin(); it != link.ActFuns.end(); it++) {
					if (it->x > Max) {
						Max = it->x;
						mMax = m;
					}
					m++;
				}
				int TrueDigit = ch[a][b];
				if (mMax != TrueDigit) {
					CE = costError((Bra.layers.end() - 1)->ActFuns, TrueDigit, 1);
					Bra.learn(TrueDigit, speedLearn);
				}
				system("cls");
				std::cout << "Проход №" << i << endl;
				std::cout << CE << endl;
				std::cout << mMax << ';' << TrueDigit << '(' << b << ';' << a << ')' << endl;
				if (i == learnC - 1) {
					std::cout << "Продолжить обучение или закончить?" << endl;
					std::cout << "1.продолжить 2.закончить" << endl;
					int choice = 0; cin >> choice;
					if (choice == 1) {
						int Plus = 0;
						std::cout << "сколько раз сеть будет обучаться?" << endl;
						cin >> Plus;
						learnC += Plus;
					}

				}
				b++;
			}
			break;
		}
		case 8: {
			for (int i = 0; i < PicData.DataPoint.size() - 1; i++) {
				int SizeLine = (PicData.DataPoint.begin() + i)->size();
				for (int j = 0; j < SizeLine - 1; j++) {
					Mat rating = PicData.GetCut(j, i);
					vector<neuron> Input;
					PicVec(rating, Input);
					//for (vector<neuron>::iterator Inp = Input.begin(); Inp != Input.end(); Inp++) std::cout << Inp->x << endl;
					Bra.start(Input);
					Input.clear();
					layer& link = *(Bra.layers.end() - 1);
					int i = 0, iMax = 0;
					float Max = 0;
					for (vector<ActFun>::iterator it = link.ActFuns.begin(); it != link.ActFuns.end(); it++) {
						if (it->x > Max) {
							Max = it->x;
							iMax = i;
						}
						i++;
					}
					if (iMax > 1) std::cout << ' ' << iMax << " |";
					else (iMax == 0) ? std::cout << "   |" : std::cout << "н/б|";
				}
				cout << endl;
			}
			break;
		}
		case 4: {
			int counter = 1;
			for (vector<layer>::iterator Ls = Bra.layers.begin(); Ls != Bra.layers.end(); Ls++) {
				std::cout << "Слой №" << counter << endl;
				if (Ls == Bra.layers.begin()) {
					for (vector<neuron>::iterator n = Ls->neurons.begin(); n != Ls->neurons.end(); n++) {
						std::cout << n->x << '|';
						int d = distance(Ls->neurons.begin(), n);
						for (vector<ActFun>::iterator m = (Ls + 1)->ActFuns.begin(); m != (Ls + 1)->ActFuns.end(); m++) {
							std::cout << m->weight[d] << ',';
						}
						std::cout << endl;
					}
				}
				else {
					for (vector<ActFun>::iterator n = Ls->ActFuns.begin(); n != Ls->ActFuns.end(); n++) {
						std::cout << n->x << '|';
						if (Ls != Bra.layers.end() - 1) {
							int d = distance(Ls->ActFuns.begin(), n);
							for (vector<ActFun>::iterator m = (Ls + 1)->ActFuns.begin(); m != (Ls + 1)->ActFuns.end(); m++) {
								std::cout << m->weight[d] << ',';
							}
						}
						else std::cout << "нет весов";
						std::cout << endl;

					}
				}
				std::cout << endl << endl;
				counter++;
			}
			break;
		}
		case 5: {
			Bra.Clear();
			cout << "Успешно удалена сеть!" << endl;
			break;
		}
		case 6: {
			std::cout << "выберите настройку" << endl;
			std::cout << "1.изменить скорость обучения     2.изменить допустимую погрешность" << endl;
			std::cout << "3.вывести все настройки          4.выйти из настроек" << endl;
			int var; cin >> var;
			if (var == 4) break;
			switch (var) {
			case 1: {
				std::cout << "введите новую скорость обучения" << endl;
				float speedValue = 0;
				cin >> speedValue;
				if (speedValue > 0) {
					SpeedSetting = speedValue;
					std::cout << "новая скорость обучения - " << SpeedSetting << endl;
				}
				break;
			}
			case 3: {
				std::cout << "скорость обучения сети - " << SpeedSetting << endl;
				break;
			}
			default: std::cout << "неверно введен номер команды" << endl;
			}
			break;
		}
		case 10: {
			char current_work_dir[FILENAME_MAX];
			_getcwd(current_work_dir, sizeof(current_work_dir));
			string dir = current_work_dir;
			dir = dir + "\\DataAI\\*.AI";
			wstring DIRws;
			for (int i = 0; i < dir.length(); ++i) DIRws += wchar_t(dir[i]);
			const wchar_t* ResultDIR = DIRws.c_str();

			WIN32_FIND_DATA FindFileData;
			HANDLE hf;
			hf = FindFirstFile(ResultDIR, &FindFileData);

			if (hf != INVALID_HANDLE_VALUE) {
				vector<string> SaveName;
				cout << "0.Назад" << endl;
				int k = 1;
				do {
					wstring ws(FindFileData.cFileName);
					string str(ws.begin(), ws.end() - 3);
					std::cout << k << '.' << str << "\n";
					SaveName.push_back(str);
					k++;
				} while (FindNextFile(hf, &FindFileData) != 0);
				FindClose(hf);
				while (true) {
					cout << "Выберите нейросеть" << endl;
					int  ChoiceSave = 0; cin >> ChoiceSave;
					if (ChoiceSave == 0) break;
					if (ChoiceSave > 0 && ChoiceSave <= SaveName.size()) {
						string Name = *(SaveName.begin() + ChoiceSave - 1);
						string path = "DataAI\\" + Name + ".AI";
						Bra.Load(path);
						break;
					}
					cout << "Выбор выходит за пределы!" << endl;
					system("pause");
					system("cls");
				}
			}
			break;
		}
		case 11: {
			cout << "Введите название сохранения: ";
			string name; cin >> name;

			char current_work_dir[FILENAME_MAX];
			_getcwd(current_work_dir, sizeof(current_work_dir));
			string dir = current_work_dir;
			dir = dir + "\\DataAI\\*.AI";
			wstring DIRws;
			for (int i = 0; i < dir.length(); ++i) DIRws += wchar_t(dir[i]);
			const wchar_t* ResultDIR = DIRws.c_str();

			WIN32_FIND_DATA FindFileData;
			HANDLE hf;
			hf = FindFirstFile(ResultDIR, &FindFileData);

			if (hf != INVALID_HANDLE_VALUE) {
				vector<string> SaveName;
				do {
					wstring ws(FindFileData.cFileName);
					string str(ws.begin(), ws.end() - 3);
					SaveName.push_back(str);
				} while (FindNextFile(hf, &FindFileData) != 0);
				FindClose(hf);
				bool flag = false;
				for (vector<string>::iterator SN = SaveName.begin(); SN != SaveName.end(); SN++) {
					string buff = *SN;
					if (name == buff) {
						cout << "Такое сохранение уже существует! Заменить?" << endl;
						cout << "1.да		2.нет" << endl;
						int Choice = 1; cin >> Choice;
						if (Choice == 1) {
							string path = "DataAI\\" + name + ".AI";
							Bra.Load(path);
						}
						flag = true;
						break;
					}
				}
				if (flag)break;
				string path = "DataAI\\" + name + ".AI";
				Bra.Save(path);
			}
			break;
		}
		case 13: {
			char current_work_dir[FILENAME_MAX];
			_getcwd(current_work_dir, sizeof(current_work_dir));
			string dir = current_work_dir;
			cout << dir << endl;
			break;
		}
		}
		system("Pause");
	}
}