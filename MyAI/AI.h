#pragma once
#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>
#include <windows.h>
#include <fstream>

using namespace std;

float randomF() { srand(time(0)); return ((float)rand()) / RAND_MAX; }


class neuron {
public:
	neuron() {};
	neuron(float inp) { x = inp; };
	~neuron() {};

	float x = 0.0f;
};

float methodAF(float num, int NumMethod) {
	switch (NumMethod) {
	case 1: return num; //ТОЖДЕСТВО
	case 2: return(((float)1.0) / (1 + exp(-num)));//СИГМОИД
	case 3: return tanh(num);//ГИПЕРБОЛОИДНЫЙ ТАНГЕНС
	case 4: {//ReLU
		if (num >= 0) return num;
		else return 0;
	}
	default: return 0;
	}
}
float DSigmoid(float num) { return (num * (1 - num)); }

class ActFun : public neuron {
public:
	ActFun() :neuron(0) {};
	~ActFun() {};
	ActFun(vector <neuron*> ArrN) :neuron(0) {
		nodeN.swap(ArrN);
		sum();
	};

	vector <neuron*> nodeN;
	vector <ActFun*> nodeAF;
	vector <float> weight;
	float d = 0.0f;
	float b = 0;
	float bw = randomF();

	void sum() {
		float s = 0.0f;
		if (nodeN.size() != 0) {
			for (int i = 0; i < nodeN.size(); i++) {
				float ws = *(this->weight.begin() + i);
				float xs = (*(this->nodeN.begin() + i))->x;
				s += (ws * xs);
			}
		}
		else {
			for (int i = 0; i < nodeAF.size(); i++) {
				float ws = *(this->weight.begin() + i);
				float xs = (*(this->nodeAF.begin() + i))->x;
				s += (ws * xs);
			}
		}
		this->x = (float)methodAF(s, 2);
	}
	void addN(neuron* n) { nodeN.push_back(n); weight.push_back(randomF() - 0.5); }
	void addN(ActFun* AF) { nodeAF.push_back(AF); weight.push_back(randomF() - 0.5); }
};

float costError(vector <ActFun>& Result, int right, int method) {
	float a = 0;
	switch (method) {
	case 1: {
		for (int i = 0; i < Result.size(); i++) {
			float buff = (i == right) ? 1 : 0;
			a += ((Result[i]).x - buff) * ((Result[i]).x - buff);
		}
		return (a / Result.size());
		break;
	}
	case 2: {
		for (int i = 0; i < Result.size(); i++) {
			float buff = (i == right) ? 1 : 0;
			a += ((Result[i]).x - buff) * ((Result[i]).x - buff);
		}
		return sqrt(a / Result.size());
		break;
	}
	case 3: {
		for (int i = 0; i < Result.size(); i++) {
			float buff = (i == right) ? 1 : 0;
			a += atan((Result[i]).x - buff) * atan((Result[i]).x - buff);
		}
		return a / Result.size();
		break;
	}
	}
}

class layer {
public:
	layer() {};
	layer(vector <neuron> n) { neurons.swap(n); };
	layer(vector <ActFun> AF) { ActFuns.swap(AF); };
	~layer() {};

	vector <neuron> neurons;
	vector <ActFun> ActFuns;
};

class Brain {
public:
	Brain() {};
	~Brain() {};

	vector<layer> layers;

	void addL(layer l) { layers.push_back(l); }

	bool build() {
		bool status = false;
		if (layers.size() < 2) std::cout << "Ошибка: для работы сети необходимо хотя бы два скрытых слоя!" << endl;
		else {
			bool flag1 = false, flag2 = false;
			vector<layer>::iterator it;
			int i = 0;
			for (it = layers.begin(); it != layers.end(); it++) {
				if (it->ActFuns.size() + it->neurons.size() == 0) { flag1 = true; break; }
				if (it->neurons.size() > 0) { if (i != 0) { flag2 = true; break; } }
				i++;
			}
			if (flag1) std::cout << "Ошибка: слой №" << i << " - пустой!" << endl;
			else if (flag2) std::cout << "Ошибка: в слое №" << i << " есть входной нейрон, уберите все входные нейроны в скрытых слоях!" << endl;
			else status = true;
		}
		return status;
	}
	void start(vector<neuron> data) {//запуск нейронной сети
		if (data.size() != 0) {//если входные данные пустые, то просто запустить сеть ничего не меняя, иначе код ниже
			if (layers.begin()->neurons.size() == 0) {//если первый слой - не входной, то вставляем входные данные и подключаем его к первому скрытому словю
				layers.insert(layers.begin(), data);

				for (vector<ActFun>::iterator AFs = ((layers.begin() + 1)->ActFuns).begin(); AFs != ((layers.begin() + 1)->ActFuns).end(); AFs++) {
					for (int iter = 0; iter < layers.begin()->neurons.size(); iter++) {
						neuron* buff = &(layers.begin()->neurons[iter]);
						AFs->addN(buff);
					}
				}
				vector<layer>::iterator it1, it2;
				for (it1 = (layers.begin() + 2); it1 != layers.end(); it1++) {
					it2 = (it1 - 1);
					for (vector<ActFun>::iterator itC = it1->ActFuns.begin(); itC != it1->ActFuns.end(); itC++) {
						for (int iter = 0; iter < it2->ActFuns.size(); iter++) {
							ActFun* buff = &(*(it2->ActFuns.begin() + iter));
							itC->addN(buff);
						}
					}
				}
			}
			else {//иначе первый слой - входной, то заменяем там входные значения
				for (vector<neuron>::iterator n = layers.begin()->neurons.begin(); n != layers.begin()->neurons.end(); n++) {//цикл
					int d = distance(layers.begin()->neurons.begin(), n);//расстояние между началом и текущим итератором, чтобы какой номер сейчас у итератора
					n->x = data[d].x;
				}
			}
		}
		for (vector<layer>::iterator L = (layers.begin() + 1); L != layers.end(); L++) {
			for (vector<ActFun>::iterator AFs = (L->ActFuns).begin(); AFs != (L->ActFuns).end(); AFs++) AFs->sum();
		}
	}
	void learn(int value, float sp) {
		float speed = sp;
		for (int j = 0; j < (this->layers.end() - 1)->ActFuns.size(); j++) {//изменение весов нейросети для каждого результирующего нейрона
			ActFun& Result = *(((layers.end() - 1)->ActFuns.begin()) + j);//ссылка на итоговый слой
			float True = 0.0;//истинное значение
			if (j == value) True = 1.0;//если текущий нейрон равен значению правильному значению, то истинное значение равно для него - 1
			Result.d = True - Result.x;//разница между предроложением и истины
			for (vector<layer>::iterator L = (layers.end() - 2); L != layers.begin(); L--) {//цикл для расчета разниц всех нейронов
				for (vector<ActFun>::iterator AF = L->ActFuns.begin(); AF != L->ActFuns.end(); AF++) {
					int d = distance(L->ActFuns.begin(), AF);
					if (L == (layers.end() - 2)) { AF->d = Result.d * Result.weight[d]; }
					else {
						float Dsum = 0;
						for (vector<ActFun>::iterator AF2 = (L + 1)->ActFuns.begin(); AF2 != (L + 1)->ActFuns.end(); AF2++) { Dsum += (AF2->d) * (AF2->weight[d]); }
						AF->d = Dsum;
					}
				}
			}
			for (vector<layer>::iterator L = (layers.begin() + 1); L != layers.end(); L++) {
				if (L == (layers.end() - 1)) {
					for (vector<float>::iterator w = Result.weight.begin(); w != Result.weight.end(); w++) {
						int d = distance(Result.weight.begin(), w);
						*w = (*w) + speed * (Result.d) * DSigmoid(Result.x) * ((L - 1)->ActFuns.begin() + d)->x;
					}
				}
				else {
					for (vector<ActFun>::iterator AF = L->ActFuns.begin(); AF != L->ActFuns.end(); AF++) {
						for (vector<float>::iterator w = AF->weight.begin(); w != AF->weight.end(); w++) {
							int d = distance(AF->weight.begin(), w);
							if (L == (layers.begin() + 1)) *w = (*w) + speed * (AF->d) * DSigmoid(AF->x) * ((L - 1)->neurons.begin() + d)->x;
							else *w = (*w) + speed * (AF->d) * DSigmoid(AF->x) * ((L - 1)->ActFuns.begin() + d)->x;
						}
						AF->sum();
					}
				}
			}
		}
	}
	void Save(string dir) {
		if (this->layers.size() == 0) cout << "ОШИБКА: нельзя сохранять пустую нейросеть!" << endl;
		else if (this->layers.begin()->neurons.size() == 0) cout << "ОШИБКА: нейросеть должна иметь в начале входной слой!" << endl;
		else {
			ofstream fout(dir, ios_base::binary);
			fout << this->layers.begin()->neurons.size() << '\n';
			fout << this->layers.size() - 1 << '\n';
			for (vector<layer>::iterator Ls = (this->layers.begin() + 1); Ls != this->layers.end(); Ls++) {
				fout << Ls->ActFuns.size() << '\n';
				for (vector<ActFun>::iterator AFs = Ls->ActFuns.begin(); AFs != Ls->ActFuns.end(); AFs++) {
					for (vector <float>::iterator W = AFs->weight.begin(); W != AFs->weight.end(); W++) fout << *W << '\n';
				}
			}
			fout << "end";
			fout.close();
		}
	}
	void Load(string dir) {
		ifstream fin(dir, ios_base::binary);
		if (!fin.is_open())cout << "Такого файла/директории не существует!";
		else {
			if (this->layers.size() > 0) this->Clear();
			setlocale(LC_NUMERIC, "C");
			char input[10];
			fin.getline(input, 13, '\n');
			int layerInp = stoi(input);
			vector<neuron> nBuff;
			for (int i = 0; i < layerInp; i++) nBuff.push_back(*(new neuron(0)));
			layer lBuff(nBuff);
			this->layers.push_back(lBuff);

			char kl[10];
			fin.getline(kl, 13, '\n');
			int klayer = stoi(kl);
			for (int i = 0; i < klayer; i++) {
				int layerSize = (i == 0) ? this->layers.begin()->neurons.size() : (this->layers.end() - 1)->ActFuns.size();
				int kActFun = 0;
				fin >> kActFun;
				vector<ActFun> AFs;
				for (int j = 0; j < kActFun; j++) {
					ActFun AFBuff;
					float buffF = 0;
					for (int i = 0; i < layerSize; i++) {
						fin >> buffF;
						AFBuff.weight.push_back(buffF);
						buffF = 0;
					}
					AFs.push_back(AFBuff);
				}
				layer l(AFs);
				this->layers.push_back(l);
			}

			for (vector<ActFun>::iterator AFs = (this->layers.begin() + 1)->ActFuns.begin(); AFs != (this->layers.begin() + 1)->ActFuns.end(); AFs++) {
				for (vector<neuron>::iterator Ns = this->layers.begin()->neurons.begin(); Ns != this->layers.begin()->neurons.end(); Ns++) AFs->nodeN.push_back(&(*Ns));
			}
			for (vector<layer>::iterator Ls = (this->layers.begin() + 2); Ls != this->layers.end(); Ls++) {
				for (vector<ActFun>::iterator AFs = Ls->ActFuns.begin(); AFs != Ls->ActFuns.end(); AFs++) {
					for (vector<ActFun>::iterator Lsp = (Ls - 1)->ActFuns.begin(); Lsp != (Ls - 1)->ActFuns.end(); Lsp++) AFs->addN(&(*Lsp));
				}
			}

		}
		fin.close();
	}
	void Clear() {
		if (this->layers.size() == 0) cout << "Нейронная сеть пустая! Что удалять-то?" << endl;
		else {
			for (vector<layer>::iterator L = (this->layers.begin() + 1); L != this->layers.end(); L++) {
				for (vector<ActFun>::iterator AF = L->ActFuns.begin(); AF != L->ActFuns.end(); AF++) {
					AF->weight.clear();
					AF->nodeN.clear();
					AF->nodeAF.clear();
				}
			}
			this->layers.clear();
		}
	}

};

void PicVec(Mat& source, vector<neuron>& data) {
	int w = source.rows;
	int h = source.cols;
	for (int a = 0; a < h; a++) {
		for (int b = 0; b < w; b++) {
			int pixel = source.at<uchar>(a, b);
			//cout << pixel << ';';
			float pixelf = (pixel / 1000.0);
			//cout << pixelf << endl;
			neuron buff(pixelf);
			data.push_back(buff);
		}
	}
}
int ToInt(char ch) {
	switch (ch)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	default: return 0;
	}
}
void trans(int digit[15][35]) {
	char buff[15][35];

	string pathStr = "test.txt";
	ifstream fin(pathStr.c_str());
	for (int r = 0; r < 15; r++) fin.getline(buff[r], 37, '\n');
	fin.close();
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 35; j++) digit[i][j] = ToInt(buff[i][j]);
	}
}