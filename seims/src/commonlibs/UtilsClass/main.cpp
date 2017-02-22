#include "utilities.h"
using namespace std;

int main() {
	double t1 = TimeCounting();
	// or create an utilsTime instance first
	utilsTime ut;
	t1 = ut.TimeCounting();

    int threadnum = 1;
    SetDefaultOpenMPThread();

    cout << "*** Utils Demo ***" << endl;
	cout << "Number of processors: " << GetAvailableThreadNum()<< endl;
	vector<float> values;
	vector<int> positionRows;
	vector<int> positionCols;
	int r[10] = { 0,1,1,2,2,3,3,4,4,5 };
	int c[10] = { 0,2,5,3,4,0,1,2,3,5 };
	float v[10] = { 3.1,4.2,1.5,0.6,31.2,2.4,8.4,9.1,80,108 };
	for (int i = 0; i < 10; i++){
		positionRows.push_back(r[i]);
		positionCols.push_back(c[i]);
		values.push_back(v[i]);
	}

	vector<int>::iterator rit = positionRows.begin();
	vector<int>::iterator cit = positionCols.begin();
	vector<float>::iterator vit = values.begin();
	for (vector<float>::iterator it = values.begin(); it != values.end();)
	{
		int idx = distance(vit, it);
		int tmpr = positionRows.at(idx);
		int tmpc = positionCols.at(idx);
		if (tmpr > 4 || tmpr < 1 || tmpc > 4 || tmpc < 1)
		{
			it = values.erase(it);
			positionCols.erase(cit + idx);
			positionRows.erase(rit + idx);
			/// reset the iterators
			rit = positionRows.begin();
			cit = positionCols.begin();
			vit = values.begin();
		}
		else
			++it;
	}
	string a[] = {"aasdf", "bn"};
	cout<<a[0]<<","<<a[1]<<endl;
	for (string::iterator it = a->begin(); it != a->end(); it++)
	{
		cout<<*it<<endl;
	}
	string oldpath = "C:/test/t.asc";
	cout<<ReplaceSuffix(oldpath, "tif")<<endl;
    cout<<ReplaceSuffix(oldpath, "txt")<<endl;

	string pathtest = "C://Windows";
	if (PathExists(pathtest))
		cout<< pathtest << " exists!" <<endl;

	int *array1 = new int[10];
	for (int i = 0; i < 10; i++)
		array1[i] = i + 1;
	int *&array2 = array1;
	Release1DArray(array2);
	if (array2 == NULL) cout<<"array2 released!"<<endl;
	cout<<"array1 addr: "<<array1<<endl;
	if (array1 == NULL) cout<<"array1 released!"<<endl;

	// string match
	const char* str1 = "elevation";
	const char* str2 = "T_MEAN";
	const char* str3 = "t_mean";
	if (StringMatch(str3, str2))
		cout << "str1 is equal to str2" << endl;
    return 0;
}