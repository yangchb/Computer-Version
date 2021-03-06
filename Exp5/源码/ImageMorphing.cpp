#include "pch.h"
#include "ImageMorphing.h"
#include <string>
#include <fstream>
#include <sstream>

bool IsTriangle(point p, triangle t);

ImageMorphing::ImageMorphing() {
	source.load_bmp("picture/1.bmp");
	target.load_bmp("picture/2.bmp");
	/*if (source.size() > target.size())
		source.resize(target.width(), target.height());
	else
		target.resize(source.width(), source.height());*/
	//读入源图中面部特征点
	ifstream filePoint;
	filePoint.open("source.txt");
	string s;
	int x = 0, y = 0;
	while (getline(filePoint, s)) {
		stringstream ss(s);
		ss >> x >> y;
		point p(x, y);
		source_point.push_back(p);
	}
	filePoint.close();
	//读入目标图中面部特征点
	filePoint.open("target.txt");
	while (getline(filePoint, s)) {
		stringstream ss(s);
		ss >> x >> y;
		point p(x, y);
		target_point.push_back(p);
	}
	filePoint.close();
	//读入需要建立的仿射变换网格点
	filePoint.open("grid.txt");
	int p1 = 0, p2 = 0, p3 = 0;
	while (getline(filePoint, s)) {
		vector<int> p;
		stringstream ss(s);
		ss >> p1 >> p2 >> p3;
		p.push_back(p1);
		p.push_back(p2);
		p.push_back(p3);
		triangle_grid.push_back(p);
	}
	filePoint.close();
	int i = 0;
	//建立源图和目标图中的三角形网格
	for (i = 0; i < triangle_grid.size(); ++i) {
		triangle t1(source_point[triangle_grid[i][0]], source_point[triangle_grid[i][1]], source_point[triangle_grid[i][2]]);
		triangle t2(target_point[triangle_grid[i][0]], target_point[triangle_grid[i][1]], target_point[triangle_grid[i][2]]);
		source_triangle.push_back(t1);
		target_triangle.push_back(t2);
	}
	//获取中间每一帧图片的三角形网格
	get_middleGrid();
	//获取每一帧三角形网格从源图到目标图变换需要的特征矩阵
	AffineTransform();
}

//获取中间每一帧图片的三角形网格
void ImageMorphing::get_middleGrid() {
	//存储每一帧中间图的点
	vector <vector<point>> mid_point;
	int i = 0, j = 0;
	for (i = 0; i < frame; ++i) {
		vector<point> temp;
		for (j = 0; j < source_point.size(); ++j) {
			int x = float(source_point[j].x) + float(float(i + 1) / (frame + 1)) * float(target_point[j].x - source_point[j].x);
			int y = float(source_point[j].y) + float(float(i + 1) / (frame + 1)) * float(target_point[j].y - source_point[j].y);
			point p(x, y);
			temp.push_back(p);
		}
		mid_point.push_back(temp);
	}
	//获取中间每一帧的三角形网格
	for (i = 0; i < frame; ++i) {
		vector<triangle> temp;
		for (j = 0; j < triangle_grid.size(); ++j) {
			triangle t(mid_point[i][triangle_grid[j][0]], mid_point[i][triangle_grid[j][1]], mid_point[i][triangle_grid[j][2]]);
			temp.push_back(t);
		}
		mid_triangle.push_back(temp);
	}
}

void ImageMorphing::AffineTransform() {
	//获取每一帧图与源图网格的变换矩阵
	int i = 0, j = 0, k = 0;
	for (i = 0; i < frame; ++i) {
		vector<CImg<float>> temp;
		for (j = 0; j < triangle_grid.size(); ++j) {
			triangle src = mid_triangle[i][j];
			triangle dst = source_triangle[j];
			CImg<float> A(3, 3, 1, 1, 1);
			CImg<float> y1(1, 3, 1, 1, 0), y2(1, 3, 1, 1, 0);
			CImg<float> c1(1, 3, 1, 1, 0), c2(1, 3, 1, 1, 0);
			A(0, 0) = src.a.x; A(1, 0) = src.a.y;
			A(0, 1) = src.b.x; A(1, 1) = src.b.y;
			A(0, 2) = src.c.x; A(1, 2) = src.c.y;
			y1(0, 0) = dst.a.x; y2(0, 0) = dst.a.y;
			y1(0, 1) = dst.b.x; y2(0, 1) = dst.b.y;
			y1(0, 2) = dst.c.x; y2(0, 2) = dst.c.y;
			c1 = y1.solve(A);
			c2 = y2.solve(A);
			CImg<float> transform(3, 3, 1, 1, 0);
			for (k = 0; k < 3; ++k) {
				transform(k, 0) = c1(0, k);
				transform(k, 1) = c2(0, k);
			}
			transform(2, 2) = 1;
			temp.push_back(transform);
		}
		source_matrix.push_back(temp);
		//temp.clear();
	}
	//获取每一帧图与目标图网格的变换矩阵
	for (i = 0; i < frame; ++i) {
		vector<CImg<float>> temp;
		for (j = 0; j < triangle_grid.size(); ++j) {
			triangle src = mid_triangle[i][j];
			triangle dst = target_triangle[j];
			CImg<float> A(3, 3, 1, 1, 1);
			CImg<float> y1(1, 3, 1, 1, 0), y2(1, 3, 1, 1, 0);
			CImg<float> c1(1, 3, 1, 1, 0), c2(1, 3, 1, 1, 0);
			A(0, 0) = src.a.x; A(1, 0) = src.a.y;
			A(0, 1) = src.b.x; A(1, 1) = src.b.y;
			A(0, 2) = src.c.x; A(1, 2) = src.c.y;
			y1(0, 0) = dst.a.x; y2(0, 0) = dst.a.y;
			y1(0, 1) = dst.b.x; y2(0, 1) = dst.b.y;
			y1(0, 2) = dst.c.x; y2(0, 2) = dst.c.y;
			c1 = y1.solve(A);
			c2 = y2.solve(A);
			CImg<float> transform(3, 3, 1, 1, 0);
			for (k = 0; k < 3; ++k) {
				transform(k, 0) = c1(0, k);
				transform(k, 1) = c2(0, k);
			}
			transform(2, 2) = 1;
			temp.push_back(transform);
			
		}
		target_matrix.push_back(temp);
		//temp.clear();
	}
}

//对图片进行变换处理
void ImageMorphing::Morphing_process() {
	int i = 0, j = 0;
	result.push_back(source);
	for (i = 0; i < frame; ++i) {
		float k = float(i + 1) / (frame + 1);
		CImg<float> middle(target.width(), target.height(), 1,3,0);
		cimg_forXY(middle, x, y) {
			for (j = 0; j < mid_triangle[0].size(); ++j) {
				point p(x, y);
				if (IsTriangle(p, mid_triangle[i][j])) {
					CImg<float> x0(1, 3, 1, 1, 1);
					CImg<int> b1(1, 3, 1, 1, 1), b2(1, 3, 1, 1, 1);
					x0(0, 0) = x;
					x0(0, 1) = y;
					CImg<float> A1 = source_matrix[i][j];
					CImg<float> A2 = target_matrix[i][j];
					b1 = A1 * x0;
					b2 = A2 * x0;
					middle(x, y, 0) = (1 - k) * source(b1(0, 0), b1(0, 1), 0) + k * target(b2(0, 0), b2(0, 1), 0);
					middle(x, y, 1) = (1 - k) * source(b1(0, 0), b1(0, 1), 1) + k * target(b2(0, 0), b2(0, 1), 1);
					middle(x, y, 2) = (1 - k) * source(b1(0, 0), b1(0, 1), 2) + k * target(b2(0, 0), b2(0, 1), 2);
					break;
				}
			}
		}
		result.push_back(middle);
	}
	result.push_back(target);
	//保存结果的视频
	result.save_video("result.mp4");
	//保存结果的gif动图
	result.save_gif_external("a.gif");
	//存储每一帧图片
	for (i = 0; i < result.size(); i++) {
		string s = to_string(i + 1);
		s += ".bmp";
		result[i].save_bmp(s.c_str());
	}
}


//判断点是否在该三角形网格内
//若点p在ABC组成的三角形内，则p = A +  u * (C – A) + v * (B - A)
//u >= 0  v >= 0 u + v <= 1
bool IsTriangle(point p, triangle t) {
	float x0 = t.c.x - t.a.x, y0 = t.c.y - t.a.y;
	float x1 = t.b.x - t.a.x, y1 = t.b.y - t.a.y;
	float x2 = p.x - t.a.x, y2 = p.y - t.a.y;
	float temp = x0 * x0 + y0 * y0, temp1 = x0 * x1 + y0 * y1, temp2 = x0 * x2 + y0 * y2, temp3 = x1 * x1 + y1 * y1, temp4 = x1 * x2 + y1 * y2;
	float u = float(temp3 * temp2 - temp1 * temp4) / (float)(temp * temp3 - temp1 * temp1);
	float v = float(temp * temp4 - temp1 * temp2) / (float)(temp * temp3 - temp1 * temp1);
	if (u >= 0 && v >= 0 && u + v <= 1)
		return true;
	return false;
}