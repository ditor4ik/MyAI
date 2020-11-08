#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>
#include <windows.h>
#include <locale.h>
#include <stdio.h> 
#include <direct.h>

using namespace cv;
using namespace std;

class pointer {
public:
	int x = 0;
	int y = 0;
	Mat* rgb;
	Mat* hsv;

	pointer(int X, int Y, Mat& RGB, Mat& HSV) { x = X; y = Y; rgb = &RGB; hsv = &HSV; }
	~pointer() {}

	Vec3b& GetRGB() { return rgb->at<Vec3b>(this->y, this->x); }
	Vec3b& GetRGB(int X, int Y) { return rgb->at<Vec3b>(this->y + Y, this->x + X); }
	Vec3b& GetHSV() { return hsv->at<Vec3b>(this->y, this->x); }
	Vec3b& GetHSV(int X, int Y) { return hsv->at<Vec3b>(this->y + Y, this->x + X); }

	void paintPoint(Mat& source, int r) { rectangle(source, Rect(this->x - r, this->y - r, r * 2, r * 2), Scalar(255, 0, 0)); }

	bool checkH(int r) {
		bool buff = false;
		for (int i = 0; i <= (r * 2); i++) {
			int CoorX = (-r + i);
			if (this->x + CoorX < 1 || this->x + CoorX > rgb->cols || this->y > rgb->rows) continue;
			if (abs(this->x - (CoorX + this->x)) > 1) {
				int PointV1 = this->GetHSV(CoorX, 0)[2];
				int PointV2 = this->GetHSV()[2];
				if (abs(PointV1 - PointV2) > 90) {
					buff = true;
					break;
				}
			}
		}
		return buff;
	}
	bool checkV(int r) {
		bool buff = false;
		int d = r * 2;
		for (int i = 0; i <= d; i++) {
			int CoorY = (i - r);
			if ((CoorY + this->y) >= 0 && (CoorY + this->y) <= rgb->rows) {
				int PointV1 = this->GetHSV(0, CoorY)[2];
				int PointV2 = this->GetHSV()[2];
				if (abs(PointV1 - PointV2) > 85) {
					buff = true;
					break;
				}
			}
		}
		return buff;
	}
};
class breakpoint {
public:
	int x = 0;
	int y = 0;

	breakpoint(int X, int Y) { x = X; y = Y; }
	breakpoint(breakpoint* CLONE) { x = CLONE->x; y = CLONE->y; }
};
class equation {
public:
	int A = 0;
	int B = 0;
	int C = 0;

	equation(breakpoint& p1, breakpoint& p2) {
		A = p1.y - p2.y;
		B = p2.x - p1.x;
		C = (p1.x * p2.y) - (p2.x * p1.y);
	}
	int ResultY(int X) {
		float buff = float(this->A * X + this->C) / -B;
		int buff2 = int(buff);
		int result = (buff - float(buff2) >= 0.5) ? buff2 + 1 : buff2;
		return result;
	}
	int ResultX(int Y) {
		float buff = float(this->B * Y + this->C) / -A;
		int buff2 = int(buff);
		int result = (buff - float(buff2) >= 0.5) ? buff2 + 1 : buff2;
		return result;
	}
};

class Picture {
private:
	int width = 0;
	int height = 0;
	Mat picture;
public:
	vector<vector<breakpoint> > DataPoint;
	Picture(string p) {
		this->picture = imread(p);
		if (!this->picture.data) std::cout << "ОШИБКА: неверно указан путь или такого файла не существует!" << endl;
		else {
			this->width = this->picture.cols;
			this->height = this->picture.rows;
		}
	};
	Picture() {};

	void Create(string path) {
		if (this->picture.data)this->Clear();
		this->picture = imread(path);
		if (!this->picture.data) {
			std::cout << "ОШИБКА: неверно указан путь или такого файла не существует!" << endl;
		}
		else {
			this->width = this->picture.cols;
			this->height = this->picture.rows;
		}
	}

	void parsing() {
		if (!this->picture.data) std::cout << "ОШИБКА: изображение - пустое!" << endl;
		else {
			Mat src1, src2;
			src1 = this->picture.clone();
			cvtColor(src1, src2, COLOR_BGR2HSV);

			pointer per(0, 0, src1, src2);
			int buff = per.GetHSV()[2];
			for (int i = 0; i < src1.rows; i++) {
				if (abs(buff - int(per.GetHSV()[2])) > 40) break;
				buff = int(per.GetHSV()[2]);
				per.x++;
			}
			per.y += 3;


			int skip = 0;
			//строим столбец
			for (int i = 0; i < src1.rows - 5; i++) {
				if (!per.checkH(3) && skip < 0) {
					skip = 3;
					vector<breakpoint> buffVec;
					buffVec.push_back(new breakpoint(per.x, per.y));
					DataPoint.push_back(buffVec);
				}

				int Hmin = 1000, Px = 0;
				for (int j = -1; j < 2; j++) {
					if (per.x + j < 0 || per.y + 1 > src1.rows) continue;
					int Buff = per.GetHSV(j, 1)[2];
					if (Buff < Hmin) Hmin = Buff, Px = j;
				}
				per.x += Px, per.y++;
				skip--;
			}
			//строим верхнюю строку
			vector<vector<breakpoint> >::iterator bp = DataPoint.begin();
			per.x = bp->begin()->x + 1, per.y = bp->begin()->y;
			skip = 3;
			while (per.GetHSV()[2] > per.GetHSV(0, 1)[2]) per.y++;
			int diff = 15, dis = 10000;
			for (int i = per.x; i < src1.cols; i++) {	//до конца изображения
				if (per.y < 0) per.y = 0;	//если изображение выскакивает из изображения
				if ((!per.checkV(6) && skip < 0)) {	//проверяем соседние пиксели на линии(то бишь на персечение двух линий) и проверка пропуска
					skip = 3;	//обновляем пропуск
					bp->push_back(new breakpoint(per.x, per.y));	//закидываем контр. точку в вектор
					dis = (dis == 10000) ? abs(per.x - bp->begin()->x) + diff : abs(per.x - (bp->end() - 2)->x) + diff;
				}
				if (dis <= 0) {
					int midY = (bp->end() - 1)->y - per.y;
					breakpoint pointerPoint(per.x, per.y);
					equation equabuff(*(bp->end() - 1), pointerPoint);
					per.x -= diff;
					int findY = equabuff.ResultY(per.x);
					int buffY = findY - per.y;
					per.y += buffY;
					skip = 3;	//обновляем пропуск
					bp->push_back(new breakpoint(per.x, findY));	//закидываем его в вектор
					per.x += diff, per.y -= buffY;
					dis = abs((bp->end() - 1)->x - (bp->end() - 2)->x) + diff - abs(per.x - (bp->end() - 1)->x);
				}

				int Hmin = 1000; int Py = 0;//минимальная яркость и сдвиг по y
				for (int j = -1; j <= 1; j++) { //проверяем 3 соседних пикселя
					if (j + per.y < 0 || j + per.y >= per.rgb->rows) continue;//если выходит за границы - пропускаем
					int Buff = per.GetHSV(1, j)[2];//сохраняем яркость в буффер
					if (Buff < Hmin) Hmin = Buff, Py = j;	//если яркость в буффере меньше чем найденная минимальная, то сохраняем сдвиг и мин. яркость = яркость из буффера
				}

				if (abs(per.GetHSV()[2] - Hmin) > 60 && per.x > (src1.cols / 2)) {//проверка на конец таблицы во 2ой половине изображения
					if (skip <= 0) bp->push_back(new breakpoint(per.x, per.y));
					break;
				}
				per.y += Py, per.x++;
				skip--, dis--;
			}

			//строим нижнюю строку
			bp = (DataPoint.end() - 1);
			per.x = bp->begin()->x + 1, per.y = bp->begin()->y;
			skip = 3;
			while (per.GetHSV()[2] > per.GetHSV(0, 1)[2]) per.y++;
			diff = 15;
			dis = 10000;
			for (int i = per.x; i < src1.cols; i++) {	//до конца изображения
				if (per.y < 0) per.y = 0;	//если изображение выскакивает из изображения

				if ((!per.checkV(3) && skip < 0)) {	//проверяем соседние пиксели на линии(то бишь на персечение двух линий) и проверка пропуска
					skip = 3;	//обновляем пропуск
					bp->push_back(new breakpoint(per.x, per.y));	//закидываем его в вектор
					dis = (dis == 10000) ? abs(per.x - bp->begin()->x) + diff : abs(per.x - (bp->end() - 2)->x) + diff;
				}

				if (dis <= 0) {
					int midY = (bp->end() - 1)->y - per.y;
					breakpoint pointerPoint(per.x, per.y);
					equation equabuff(*(bp->end() - 1), pointerPoint);
					per.x -= diff;
					int findY = equabuff.ResultY(per.x), buffY = findY - per.y;
					per.y += buffY;
					skip = 3;	//обновляем пропуск
					bp->push_back(new breakpoint(per.x, findY));	//закидываем его в вектор
					per.x += diff, per.y -= buffY;
					dis = abs((bp->end() - 1)->x - (bp->end() - 2)->x) + diff - abs(per.x - (bp->end() - 1)->x);
				}

				int Hmin = 1000; int Py = 0;//минимальная яркость и сдвиг по y
				for (int j = -1; j <= 1; j++) { //проверяем 3 соседних пикселя
					if (j + per.y < 0 || j + per.y >= per.rgb->rows) continue;//если выходит за границы - пропускаем
					int Buff = per.GetHSV(1, j)[2];//сохраняем яркость в буффер
					if (Buff < Hmin) Hmin = Buff, Py = j;//если яркость в буффере меньше чем найденная минимальная, то сохраняем сдвиг и мин. яркость = яркость из буффера
				}

				if (abs(per.GetHSV()[2] - Hmin) > 60 && per.x > (src1.cols / 2) && skip <= 0) {//проверка на конец таблицы во 2ой половине изображения
					bp->push_back(new breakpoint(per.x, per.y));		//если вдруг не нашел точку в конце изображения, то ставим всё равно
					break;
				}
				per.y += Py;//меняем ось У у указателя
				per.x++;//меняем ось Х у указателя
				skip--;//отнимаем пропуск
				dis--;
			}
			bp = DataPoint.begin();

			//оставляем следы по вертикали
			for (vector<breakpoint>::iterator row = bp->begin() + 1; row != bp->end(); row++) {
				per.x = row->x, per.y = row->y + 2;
				skip = 3;
				int Hmin = 1000; int Px = 0;
				for (int j = -4; j <= 4; j++) {
					int Buff = per.GetHSV(j, 1)[2];
					if (Buff < Hmin) Hmin = Buff, Px = j;
				}
				row->x += Px;
				int disITER = distance(bp->begin(), row);
				equation equbuff(*row, *(((DataPoint.end() - 1)->begin()) + disITER));
				per.x += Px;
				for (int i = per.y; i < src1.rows - 5; i++) {
					if (per.x >= src1.cols - 1 || per.x <= 0) break;
					per.GetRGB() = Vec3b(0, 255, 0);
					if (abs(per.x - equbuff.ResultX(per.y)) > 3)  per.x = equbuff.ResultX(per.y);
					int Hmin = 1000, Px = 0;
					for (int j = -1; j <= 1; j++) {
						int Buff = per.GetHSV(j, 1)[2];
						if (Buff < Hmin) Hmin = Buff, Px = j;
					}
					per.x += Px, per.y++;
					skip--;
				}
			}
			skip = 3;
			//ходим по горизонтали

			for (vector<vector<breakpoint> >::iterator bp = (DataPoint.begin() + 1); bp != (DataPoint.end() - 1); bp++) {
				per.x = bp->begin()->x + 3, per.y = bp->begin()->y;
				while (per.GetHSV()[2] > per.GetHSV(0, 1)[2]) { per.y++; }//чтобы указатель не гулял по белой зоне
				for (int i = per.x; i < src1.cols; i++) {
					if (per.GetRGB()[1] == 255 && skip < 0) {
						bp->push_back(new breakpoint(per.x, per.y));
						skip = 3;
					}
					int Hmin = 1000, Py = 0;//минимальная яркость и сдвиг по y
					for (int j = -1; j <= 1; j++) { //проверяем 3 соседних пикселя
						if (j + per.y < 0 || j + per.y >= per.rgb->rows) continue;//если выходит за границы - пропускаем
						int Buff = per.GetHSV(1, j)[2];//сохраняем яркость в буффер
						if (Buff < Hmin) Hmin = Buff, Py = j; //если яркость в буффере меньше чем найденная минимальная, то сохраняем сдвиг и мин. яркость = яркость из буффера
					}
					per.y += Py, per.x++;//меняем ось Х у указателя
					skip--;//отнимаем пропуск
				}
			}
		}
	}
	void Show_RectImg(int col, int row) {
		if (this->DataPoint.size() == 0) std::cout << "ОШИБКА: Отсутствуют данные об точках на изображениях" << endl;
		else {
			vector<vector<breakpoint> >::iterator RectRow = (this->DataPoint.begin() + row);
			vector<vector<breakpoint> >::iterator RectRowButton = (RectRow + 1);
			vector<breakpoint>::iterator RectCol = (RectRow->begin() + col);
			vector<breakpoint>::iterator RectColButton = (RectRowButton->begin() + col);
			int WMin = ((RectCol + 1)->x - RectCol->x > (RectColButton + 1)->x - RectColButton->x) ? (RectColButton + 1)->x - RectColButton->x : (RectCol + 1)->x - RectCol->x;
			int HMin = (RectColButton->y - RectCol->y > (RectColButton + 1)->y - (RectCol + 1)->y) ? (RectColButton + 1)->y - (RectCol + 1)->y : RectColButton->y - RectCol->y;
			Mat buff(picture, Rect(RectCol->x + 1, RectCol->y + 1, WMin - 1, HMin - 1));
			resize(buff, buff, cv::Size(31, 31));
			cvtColor(buff, buff, COLOR_RGB2GRAY);
			cv::namedWindow("rect", WINDOW_NORMAL);
			cv::imshow("rect", buff);
			cv::waitKey();
		}
	}

	vector<vector<int> > MatToVec(int col, int row) {
		vector<vector<int> > buffVec;
		if (this->DataPoint.size() == 0) std::cout << "ОШИБКА: Отсутствуют данные об точках на изображениях" << endl;
		else {
			vector<vector<breakpoint> >::iterator RectRow = (this->DataPoint.begin() + row);
			vector<vector<breakpoint> >::iterator RectRowButton = (RectRow + 1);
			vector<breakpoint>::iterator RectCol = (RectRow->begin() + col);
			vector<breakpoint>::iterator RectColButton = (RectRowButton->begin() + col);
			int WMin = ((RectCol + 1)->x - RectCol->x > (RectColButton + 1)->x - RectColButton->x) ? (RectColButton + 1)->x - RectColButton->x : (RectCol + 1)->x - RectCol->x;
			int HMin = (RectColButton->y - RectCol->y > (RectColButton + 1)->y - (RectCol + 1)->y) ? (RectColButton + 1)->y - (RectCol + 1)->y : RectColButton->y - RectCol->y;
			Mat buff(picture, Rect(RectCol->x + 1, RectCol->y + 1, WMin - 1, HMin - 1));
			resize(buff, buff, cv::Size(31, 31));
			cvtColor(buff, buff, COLOR_RGB2GRAY);
			for (int i = 0; i < 31; i++) {
				vector<int> temp;
				buffVec.push_back(temp);
				for (int j = 0; j < 31; j++) (buffVec.end() - 1)->push_back(buff.at<uchar>(i, j));
			}
		}
		return buffVec;
	}

	void Clear() {
		this->height = 0, this->width = 0;
		for (vector<vector<breakpoint> >::iterator iter = this->DataPoint.begin(); iter != this->DataPoint.end(); iter++) iter->clear();
		this->DataPoint.clear();
		this->picture.release();
	}

	Mat GetCut(int col, int row) {
		vector<vector<breakpoint> >::iterator RectRow = (this->DataPoint.begin() + row);
		vector<vector<breakpoint> >::iterator RectRowButton = (RectRow + 1);
		vector<breakpoint>::iterator RectCol = (RectRow->begin() + col);
		vector<breakpoint>::iterator RectColButton = (RectRowButton->begin() + col);
		int WMin = ((RectCol + 1)->x - RectCol->x > (RectColButton + 1)->x - RectColButton->x) ? (RectColButton + 1)->x - RectColButton->x : (RectCol + 1)->x - RectCol->x;
		int HMin = (RectColButton->y - RectCol->y > (RectColButton + 1)->y - (RectCol + 1)->y) ? (RectColButton + 1)->y - (RectCol + 1)->y : RectColButton->y - RectCol->y;
		Mat buff(picture, Rect(RectCol->x + 1, RectCol->y + 1, WMin - 2, HMin - 2));
		resize(buff, buff, cv::Size(30, 30));

		float kernel[9];
		kernel[0] = -0.1;
		kernel[1] = -0.1;
		kernel[2] = -0.1;

		kernel[3] = -0.1;
		kernel[4] = 2;
		kernel[5] = -0.1;

		kernel[6] = -0.1;
		kernel[7] = -0.1;
		kernel[8] = -0.1;

		Mat kernel_matrix = Mat(3, 3, CV_32FC1, kernel);

		// накладываем фильтр
		filter2D(buff, buff, -1, kernel_matrix, Point(-1, -1));
		filter2D(buff, buff, -1, kernel_matrix, Point(-1, -1));
		cvtColor(buff, buff, COLOR_RGB2GRAY);
		bitwise_not(buff, buff);
		int GrayMin = 1000; int GRAY = 0;
		for (int i = 0; i < buff.rows; i++) {
			if (i == 0 || i == buff.cols - 1) {
				for (int j = 0; j < buff.cols; j++) {
					GRAY = buff.at<uchar>(i, j);
					if (GRAY < GrayMin) GrayMin = GRAY;
				}
			}
			else {
				GRAY = (buff.at<uchar>(i, 0) < buff.at<uchar>(i, buff.cols - 1)) ? buff.at<uchar>(i, 0) : buff.at<uchar>(i, buff.cols - 1);
				if (GRAY < GrayMin) GrayMin = GRAY;
			}
		}
		for (int i = 0; i < buff.rows; i++) {
			if (i == 0 || i == buff.cols - 1) {
				for (int j = 0; j < buff.cols; j++) buff.at<uchar>(i, j) = GrayMin;
			}
			else {
				buff.at<uchar>(i, 0) = GrayMin;
				buff.at<uchar>(i, buff.cols - 1) = GrayMin;
			}
		}
		return buff;
	}

	void Debug(int Size) {
		Mat buffMat = this->picture.clone();
		for (vector<vector<breakpoint> >::iterator VecBP = this->DataPoint.begin(); VecBP != this->DataPoint.end(); VecBP++) {
			std::cout << "точек в строке " << distance(this->DataPoint.begin(), VecBP) << ": " << VecBP->size() << endl;
			for (vector<breakpoint>::iterator BP = VecBP->begin(); BP != VecBP->end(); BP++)
				rectangle(buffMat, Rect(BP->x - Size, BP->y - Size, Size * 2, Size * 2), Scalar(255, 0, 0), 2);
		}
		std::cout << "всего строк - " << this->DataPoint.size() << endl;
		namedWindow("Debug", WINDOW_NORMAL);
		imshow("Debug", buffMat);
		waitKey();
	}

	bool Check() { return picture.data; }

};