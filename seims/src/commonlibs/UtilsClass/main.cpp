#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
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
    cout << "Number of processors: " << GetAvailableThreadNum() << endl;
    vector<float> values;
    vector<int> positionRows;
    vector<int> positionCols;
    int r[10] = {0, 1, 1, 2, 2, 3, 3, 4, 4, 5};
    int c[10] = {0, 2, 5, 3, 4, 0, 1, 2, 3, 5};
    float v[10] = {3.1f, 4.2f, 1.5f, 0.6f, 31.2f, 2.4f, 8.4f, 9.1f, 80.f, 108.f};
    for (int i = 0; i < 10; i++) {
        positionRows.push_back(r[i]);
        positionCols.push_back(c[i]);
        values.push_back(v[i]);
    }

    vector<int>::iterator rit = positionRows.begin();
    vector<int>::iterator cit = positionCols.begin();
    vector<float>::iterator vit = values.begin();
    for (vector<float>::iterator it = values.begin(); it != values.end();) {
        int idx = distance(vit, it);
        int tmpr = positionRows.at(idx);
        int tmpc = positionCols.at(idx);
        if (tmpr > 4 || tmpr < 1 || tmpc > 4 || tmpc < 1) {
            it = values.erase(it);
            positionCols.erase(cit + idx);
            positionRows.erase(rit + idx);
            /// reset the iterators
            rit = positionRows.begin();
            cit = positionCols.begin();
            vit = values.begin();
        } else {
            ++it;
        }
    }
    string a[] = {"aasdf", "bn"};
    cout << a[0] << "," << a[1] << endl;
    for (string::iterator it = a->begin(); it != a->end(); it++) {
        cout << *it << endl;
    }
    string oldpath = "C:/test/t.asc";
    cout << ReplaceSuffix(oldpath, "tif") << endl;
    cout << ReplaceSuffix(oldpath, "txt") << endl;

    string pathtest = "C://Windows";
    if (PathExists(pathtest)) {
        cout << pathtest << " exists!" << endl;
    }

    int *array1 = new int[10];
    for (int i = 0; i < 10; i++) {
        array1[i] = i + 1;
    }
    int *&array2 = array1;
    int *array3(NULL);
    int *array4(NULL);
    Initialize1DArray(5, array3, 10);
    Initialize1DArray(6, array4, 20);

    //// Batch release
    //BatchRelease1DArray(array1, array3, array4, NULL);
    Release1DArray(array2);
    if (array2 == NULL) cout << "array2 released!" << endl;
    cout << "array1 addr: " << array1 << endl;
    if (array1 == NULL) cout << "array1 released!" << endl;
    Release1DArray(array3);
    Release1DArray(array4);

    int **array2d = NULL;
    array2d = new int *[2];
    for (int i = 0; i < 2; i++) {
        array2d[i] = new int[3];
        for (int j = 0; j < 3; j++) {
            array2d[i][j] = i * 3 + j;
        }
    }
    int **array2dCopy = NULL;
    try {
        if (Initialize2DArray(2, 3, array2dCopy, array2d)) {
            // do something
        }
        else {
            throw exception();
        }
    }
    catch (exception& e) {
    	cout << e.what() << endl;
    }
    catch (...) {
        cout << "Unknown exception occurred!" << endl;
    }
    //BatchRelease2DArray(2, array2d, array2dCopy, NULL);
    Release2DArray(2, array2d);
    Release2DArray(2, array2dCopy);


    cout << "*** String Utils Demo ***" << endl;
    // string match
    const char *str1 = "elevation";
    const char *str2 = "T_MEAN";
    const char *str3 = "t_mean";
    if (StringMatch(str3, str2)) {
        cout << "str1 is equal to str2" << endl;
    }
    string itemstr = "NO,NAME,NUMBER";
    vector<string> items = SplitString(itemstr, ',');
    return 0;
}