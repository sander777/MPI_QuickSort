#include <chrono>
#include <iostream>
#include <mpi.h>

using namespace std;

int ProcNum;  // Number of available processes
int ProcRank; // Rank of current process

void quickSort(int[], int, int);
int *arrayInitializtion(int);
void dummyQuickSort(int[], int, int, int *);

class ProcessTree {
  public:
    ProcessTree *left;
    ProcessTree *right;
    int val;

    ProcessTree(ProcessTree *left, ProcessTree *right, int val) {
        this->left = left;
        this->right = right;
        this->val = val;
    }

    ProcessTree *findProcess(int ind, ProcessTree **father) {
        if (this->val == ind)
            return this;
        else {
            *father = this;
            if (left != nullptr) {
                ProcessTree *res = left->findProcess(ind, father);
                if (res != nullptr)
                    return res;
            }
            *father = this;
            if (right != nullptr) {
                ProcessTree *res = right->findProcess(ind, father);
                if (res != nullptr)
                    return res;
            }
            return nullptr;
        }
    }
};

ProcessTree *getTree(int a, int b) {
    if (a == b) {
        ProcessTree *ret = (ProcessTree *)malloc(sizeof(ProcessTree));
        *ret = ProcessTree(nullptr, nullptr, a);
        return ret;
    } else if (a > b)
        return nullptr;
    else {
        ProcessTree *ret = (ProcessTree *)malloc(sizeof(ProcessTree));
        *ret = ProcessTree{
            getTree(a, a + ((b - a) / 2) - 1),
            getTree(a + ((b - a) / 2 + 1), b),
            a + ((b - a) / 2),
        };
        return ret;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
    MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
    int times = 10;
    int size = 1000000;
    int *arr;
    int arr_[size];
    ProcessTree processTree = *getTree(0, 5);
    ProcessTree *father = nullptr;
    processTree.findProcess(4, &father);

    ProcessTree currentProcess = *processTree.findProcess(ProcRank, &father);
    if (father->val == currentProcess.val) {

        int mes_mpi[times];
        int mes[times];
        for (int t = 0; t < times; t++) {
            arr = arrayInitializtion(size);
            for(int k = 0; k < size; k++) {
                arr_[k] = arr[k];
            }
            chrono::steady_clock::time_point start_mpi =
                chrono::steady_clock::now();

            int r = 0;
            dummyQuickSort(arr, 0, size, &r);
            bool left_ = false, right_ = false;

            if (currentProcess.left != nullptr) {
                int left = currentProcess.left->val;
                MPI_Send(&r, 1, MPI_INT, left, 0, MPI_COMM_WORLD);

                MPI_Send(arr, r, MPI_INT, left, 0, MPI_COMM_WORLD);
                left_ = true;
            } else {
                quickSort(arr, 0, r);
            }
            int s;
            if (currentProcess.right != nullptr) {
                int right = currentProcess.right->val;
                s = size - r;
                MPI_Send(&s, 1, MPI_INT, right, 0, MPI_COMM_WORLD);

                MPI_Send(arr + r, s, MPI_INT, right, 0, MPI_COMM_WORLD);
                right_ = true;
            } else {
                quickSort(arr, r, size);
            }
            if (left_) {
                MPI_Recv(arr, r, MPI_INT, currentProcess.left->val, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if (right_) {
                MPI_Recv(arr + r, s, MPI_INT, currentProcess.right->val, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            chrono::steady_clock::time_point end_mpi = chrono::steady_clock::now();

            chrono::steady_clock::time_point start =
                chrono::steady_clock::now();
            quickSort(arr_, 0, size);
            chrono::steady_clock::time_point end = chrono::steady_clock::now();
            int k = arr[0];
            bool isOk = true;
            for (int i = 0; i < size; i++) {
                if (k > arr[i])
                    isOk = false;
                k = arr[i];
            }
            k = arr_[0];
            for (int i = 0; i < size; i++) {
                if (k > arr_[i])
                    isOk = false;
                k = arr_[i];
            }
            cout << t << ") " << (isOk ? "OK" : "NOT OK") << endl;
            mes_mpi[t] = chrono::duration_cast<chrono::microseconds>(end_mpi - start_mpi)
                         .count();
            mes[t] = chrono::duration_cast<chrono::microseconds>(end - start)
            .count();
        }
        int s_m = 0;
        int s = 0;
        for (int i = 0; i < times; i++) {
            s += mes[i];
            s_m += mes_mpi[i];
        }
        cout << "Default: " << (double)s / (double)times << " [μs]\n";
        cout << "MPI: " << (double)s_m / (double)times << " [μs]\n";
        cout << "MPI is faster in " << (double)s / (double)s_m << " times\n";
    }

    else {
        for (int t = 0; t < times; t++) {
            int *size = (int *)malloc(sizeof(int));

            MPI_Recv(size, 1, MPI_INT, father->val, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int arr[*size];
            MPI_Recv(arr, *size, MPI_INT, father->val, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            int r = 0;
            dummyQuickSort(arr, 0, *size, &r);
            bool left_ = false, right_ = false;

            if (currentProcess.left != nullptr) {
                int left = currentProcess.left->val;
                MPI_Send(&r, 1, MPI_INT, left, 0, MPI_COMM_WORLD);

                MPI_Send(arr, r, MPI_INT, left, 0, MPI_COMM_WORLD);
                left_ = true;
            } else {
                quickSort(arr, 0, r);
            }
            int s;
            if (currentProcess.right != nullptr) {
                int right = currentProcess.right->val;
                s = *size - r;
                MPI_Send(&s, 1, MPI_INT, right, 0, MPI_COMM_WORLD);

                MPI_Send(arr + r, s, MPI_INT, right, 0, MPI_COMM_WORLD);
                right_ = true;
            } else {
                quickSort(arr, r, *size);
            }
            if (left_) {
                MPI_Recv(arr, r, MPI_INT, currentProcess.left->val, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            if (right_) {
                MPI_Recv(arr + r, s, MPI_INT, currentProcess.right->val, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            MPI_Send(arr, *size, MPI_INT, father->val, 0, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
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

void dummyQuickSort(int arr[], int s, int e, int *r) {
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

    *r = w;
}

int *arrayInitializtion(int size) {
    int *arr = new int[size];
    srand(time(0));
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 1000000;
    }

    return arr;
}
