#include <algorithm>
#include <chrono>
#include <ctime>
#include <iostream>
#include <memory.h>
#include <random>

using namespace std;

void quickSort(int[], int, int);

int main(int argc, char *argv[]) {
    int times = 20;
    int mes[times];
    for (int t = 0; t < times; t++) {
        int size = 2000000;
        int *arr = new int[size];
        srand(time(0));
        for (int i = 0; i < size; i++) {
            arr[i] = rand() % 1000000;
        }
        chrono::steady_clock::time_point start = chrono::steady_clock::now();
        quickSort(arr, 0, size);
        chrono::steady_clock::time_point end = chrono::steady_clock::now();
        int k = arr[0];
        bool isOk = true;
        for (int i = 0; i < size; i++) {
            // cout << arr[i] << ' ';
            if (k > arr[i])
                isOk = false;
            k = arr[i];

        }
        cout << t << ") " << (isOk ? "OK" : "NOT OK") << endl;
        mes[t] = chrono::duration_cast<chrono::microseconds>(end - start).count();
    }
    int s = 0;
    for(int i = 0; i < times; i++) {
        s += mes[i];
    }
    cout << (double)s / (double)times << " [μs]\n";
}

void quickSort(int arr[], int s, int e) {
    if (s >= e)
        return;
    int p = e - 1;
    int w = s;
    for (int i = s; i < e - 1; i++) {
        if (arr[i] < arr[p]) {
            swap(arr[i], arr[w]);
            w++;
        }
    }
    swap(arr[w], arr[p]);
    quickSort(arr, s, w);
    quickSort(arr, w + 1, e);
}