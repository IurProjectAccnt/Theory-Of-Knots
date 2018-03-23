#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <algorithm>
#include <vector>
#include <ctime>
#include <string>
#include <cmath>
#include <map>
#include <time.h>
using namespace std;


struct line
{
	double a, b, c;
};
struct point
{
	double x, y;
};
struct point_z
{
	double x, y, z;
};
struct knot_line///точки узла
{
	point_z start, end;
};
struct cross
{
	point place;
	knot_line  lower, upper;
};
const double EPS = 1e-8;
const double INF = 1111234;
int knot_size = 0;
vector<point> crosses;///массив, отвечающий за перекрестки
vector<vector<knot_line>> arcs;
map <knot_line, bool> lower_lines;///словарь для определения, принадлежит ли перекресток данному отрезку
vector<cross> crosses_extended;///вспомогательный массив для поиска дуг
vector<point_z> knot_by_points;///задание узла в виде последовательности точек
vector<knot_line> knot_by_lines;///задание узла в виде последовательности отрезков
int drawings_amount = 0;
inline line getline(point p, point q)///функция, возвращающая уравнение прямой по отрезку
{
	double a = 1;
	if (q.y == p.y)
	{
		a = 0;
		double b = 1;
		double c = -p.y;
		double z = sqrt(a*a + b*b);
		a /= z;
		b /= z;
		c /= z;///нормируем коэффициенты
		return{ a,b,c };
	}
	double b = a*(p.x - q.x) / (q.y - p.y);
	double c = -a*p.x - b*p.y;
	double z = sqrt(a*a + b*b + c*c);
	a /= z;
	b /= z;
	c /= z;///нормируем коэффициенты
	return{ a,b,c };
}///РАБОТАЕТ
inline double hypote(point a, point b)
{
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));///Функция, возвращающая расстояние между 2 отрезками
}///РАБОТАЕТ
inline bool checkers(point p, point q, point res)///Функция, проверяющая, лежит ли точка пересечения на отрезках
{
	if (abs(hypote(p, res) + hypote(q, res) - hypote(p, q)) <= EPS)
	{
		return true;
	}
	return false;
}///РАБОТАЕТ
double det(double a, double b, double c, double d)
{
	return a*d - b*c;///определитель
}///работает
bool operator==(vector<int> a, vector<int> b)
{
	for (int i = 0; i < a.size(); ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}///РАБОТАЕТ

bool operator==(point_z a, point_z b)
{
	return (a.x == b.x) && (a.y == b.y) && (a.z == b.z);
}///РАБОТАЕT
bool operator ==(knot_line a, knot_line b)
{
	return (a.start == b.start) && (a.end == b.end);
}
bool operator<(point_z a, point_z b)
{
	if (a.x == b.x)
	{
		if (a.y == b.y)
		{
			return a.z < b.z;
		}
		return a.y < b.y;
	}
	return a.x < b.x;
}
bool operator< (knot_line a, knot_line b)
{
	if (a.start == b.start)
	{
		return a.end < b.end;
	}
	return a.start < b.start;

}
bool operator==(point a, point b)
{
	return (a.x == b.x) && (a.y == b.y);
}///РАБОТАЕТ
bool operator==(line a, line b)
{
	return (a.a == b.a) && (a.b == b.b) && (a.c == b.c);
}///РАБОТАЕТ
inline bool intersect(point a, point b, point c, point d, point &res)
{
	line n = getline(a, b);
	line m = getline(c, d);
	res.x = det(-n.c, -m.c, n.b, m.b);///по формуле считаем точку пересения прямых
	if (abs(det(n.a, m.a, n.b, m.b)) < EPS)///если отрезки на плоскости параллельны или совпадают
	{
		res = { INF,INF };
		return false;
	}
	res.x /= det(n.a, m.a, n.b, m.b);
	res.y = det(n.a, m.a, -n.c, -m.c);
	res.y /= det(n.a, m.a, n.b, m.b);
	if (!(checkers(a, b, res) && checkers(c, d, res)))///если точка пересечения прямых не лежит на отрезках, возвращаем бесконечность
	{
		res = { INF,INF };
		return false;
	}
	return true;
}
inline point intersect_by_z(knot_line a, knot_line b)///функция, которая пересекает проекции 2 отрезков
{
	point res = { INF,INF };
	pair<point, point> line1;
	pair<point, point> line2;
	line1 = { { a.start.x,a.start.y },{ a.end.x,a.end.y } };
	line2 = { { b.start.x,b.start.y },{ b.end.x,b.end.y } };///преобразуем отрезки в пространстве в отрезки на плоскости
	if (a.start == b.start || a.end == b.start || a.start == b.end || a.end == b.end)///если 2 отрезка совпадают по вершине, считаем, что перекрестка нет
	{
		return res;
	}
	if (line1.first == line1.second || line2.first == line2.second)///если отрезок в проекции - точка
	{
		return res;
	}
	intersect(line1.first, line1.second, line2.first, line2.second, res);///ищем точку пересечения проекций
	if ((abs(res.x - line1.first.x)<EPS&&abs(res.y - line1.first.y)<EPS) || (abs(res.x - line1.second.x)<EPS&&abs(res.y - line1.second.y)<EPS) || (abs(res.x - line2.first.x)<EPS&&abs(res.y - line2.first.y)<EPS) || (abs(res.x - line2.second.x)<EPS&&abs(res.y - line2.second.y)<EPS))
	{
		res = { INF,INF };
		return res;
	}
	if (res.x<1000)
	{
		if (b.start.z > a.start.z)
		{
			knot_line c = a;
			a = b;
			b = c;
		}
		crosses_extended.push_back({ { 0,0 },{ { 0,0,0 },{ 0,0,0 } },{ { 0,0,0 },{ 0,0,0 } } });
		crosses_extended.back().place = res;
		crosses_extended.back().lower = b;
		lower_lines[b] = true;
		crosses_extended.back().upper = a;///заполняем отрезками массив с перекрестками
	}
	return res;
}

void rebuild()///функция для изменения конфигурации отрезков - превращения последовательных отрезков в одну прямую - сделано
{
	vector<knot_line> knot_by_lines_rebuilded;
	int indexrel = 1;

	for (int i = 1; i < knot_by_points.size(); ++i)
	{
		indexrel = i;
		for (int j = i + 1; j < knot_by_points.size(); ++j)
		{
			vector<int> delta_last(3);
			delta_last[0] = knot_by_points[j - 1].x - knot_by_points[j - 2].x;
			delta_last[1] = knot_by_points[j - 1].y - knot_by_points[j - 2].y;
			delta_last[2] = knot_by_points[j - 1].z - knot_by_points[j - 2].z;
			vector<int> delta_recent(3);	
			delta_recent[0] = knot_by_points[j].x - knot_by_points[j - 1].x;
			delta_recent[1] = knot_by_points[j].y - knot_by_points[j - 1].y;
			delta_recent[2] = knot_by_points[j].z - knot_by_points[j - 1].z;///считаем вектора соседних отрезков
			if (delta_recent != delta_last)///если вектора различаются
			{
				break;
			}
			indexrel = j;
		}
		knot_by_lines_rebuilded.push_back({ knot_by_points[i - 1],knot_by_points[indexrel] });///сливаем отрезки в одну прямую
		i = indexrel;
	}
	knot_by_lines.clear();
	for (int i = 0; i < knot_by_lines_rebuilded.size(); ++i)
	{
		knot_by_lines.push_back(knot_by_lines_rebuilded[i]);///заменяем изначальное представление узла
	}
}///РАБОТАЕТ 
void arcs_searching()///функция для поиска дуг
{
	knot_line start = crosses_extended[0].lower;
	vector < knot_line> knot_by_lines_ring(2 * knot_size);///массив с закольцованным узлом

	for (int i = 0; i < 2 * knot_size; ++i)
	{
		knot_by_lines_ring[i] = knot_by_lines[i % knot_size];///закольцовываем узел для более легкой реализации поиска дуг
	}
	int beginning = find(knot_by_lines_ring.begin(), knot_by_lines_ring.end(), start) - knot_by_lines_ring.begin();///ищем точку старта поиска
	vector<knot_line> current_arc;
	for (int i = beginning + 1; i < beginning + 1 + knot_size; ++i)
	{
		if (lower_lines[knot_by_lines_ring[i]])///если дошли до перекрестка, под которым мы проходим, начинаем строить новую дугу
		{
			arcs.push_back(current_arc);
			current_arc.clear();
		}

		current_arc.push_back(knot_by_lines_ring[i]);///подключаем к создаваемой дуге новый отрезок
	}
}
void cross_belonging(point a, vector<knot_line> arc)
{

}
void drawing_the_arcs()///функция для покраски дуг узла
{

}
void information()///вывод инофрмации о данном узле на экран
{
	cout << "Количество перекрестков: " << crosses.size() << endl;
	for (int i = 0; i < crosses.size(); ++i)
	{
		cout << crosses[i].x << ';' << crosses[i].y << endl;
	}
	cout << "Количество дуг: " << arcs.size() << endl;
	for (int i = 0; i < arcs.size(); ++i)
	{
		cout << i + 1 << endl;
		cout << arcs[i].size() << endl;
		for (int j = 0; j < arcs[i].size(); ++j)
		{
			cout << arcs[i][j].start.x << ";" << arcs[i][j].start.y << ";" << arcs[i][j].start.z << endl;
		}

	}
	cout << "Размер узла: " << knot_by_points.size() << endl;
	for (int i = 0; i < knot_by_points.size(); ++i)
	{
		cout << knot_by_points[i].x << ";" << knot_by_points[i].y << ";" << knot_by_points[i].z << endl;
	}
	cout << "-----------------------------------------" << endl;
}
int main()
{
	setlocale(LC_ALL, "Russian");
	///TODO - придумать способ задания узлов в системе - ГОТОВО	
	freopen("Перекрестки.txt", "w", stdout);
	freopen("out.txt", "r", stdin);
	for (int k = 0; k < 1000; ++k)
	{
		int n;
		cin >> n;
		for (int i = 0; i < n; ++i)
		{
			double x, y, z;
			char c;
			cin >> x >> c >> y >> c >> z;
			knot_by_points.push_back({ x,y,z });
		}
		string s;
		cin >> s;
		for (int i = 1; i < n; ++i)
		{
			knot_by_lines.push_back({ knot_by_points[i - 1], knot_by_points[i] });///заполнение массива с отрезками
		}
		rebuild();
		knot_size = knot_by_lines.size();
		for (int i = 0; i < knot_size; ++i)
		{
			for (int j = i + 1; j < knot_size; ++j)
			{

				point result = intersect_by_z(knot_by_lines[i], knot_by_lines[j]);

				if (result.x > 1000)///если с пересечением отрезков какие-то проблемы
				{
					continue;
				}
				if (abs(result.x) < EPS)
				{
					result.x = 0;
				}
				if (abs(result.y) < EPS)///из-за проблем с точностью очень маленькие координаты (меньше 10^-9) делаем нулем
				{
					result.y = 0;
				}
				crosses.push_back(result);
			}
		}
		cout << "";
		if (crosses.size())///если нашлись перекрестки - запускаем процедуру нахождения дуг
		{
			arcs_searching();
		}
		information();
		crosses_extended.clear();
		lower_lines.clear();
		knot_by_lines.clear();
		knot_by_points.clear();
		crosses.clear();
		arcs.clear();///Очистка массивов для обработки последующих узлов
	}
}///РАБОТАЕТ

