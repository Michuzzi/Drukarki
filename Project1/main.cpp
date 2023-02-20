#include <iostream>
#include <thread>
#include <condition_variable>
#include <queue>
using namespace std;
const int NUM_PRINTERS = 4;

class PrinterMonitor {

private:
    queue<int> printers;
    queue<int> dataQueue[NUM_PRINTERS];
    mutex mtx;
    condition_variable cv;

public:
    PrinterMonitor() 
    {
        for (int i = 0; i < NUM_PRINTERS; i++) 
        { 
            printers.push(i);
        }
    }

    int getPrinter() 
    {
        unique_lock<mutex> locker(mtx);
        cv.wait(locker, [this] { return !printers.empty(); });
        int printer = printers.front();
        printers.pop();
        return printer;
    }

    void releasePrinter(int printer) 
    {
        unique_lock<mutex> locker(mtx);
        printers.push(printer);
        cout << "Printer " << printer << " released" << endl;
        cv.notify_one();
    }

    void printData(int printer, int data) 
    {
        cout << "Printer " << printer << " is printing data " << data << endl;
    }

    void addData(int printer, int data) 
    {
        unique_lock<mutex> locker(mtx);
        dataQueue[printer].push(data);
    }

    int getData(int printer) 
    {
        unique_lock<mutex> locker(mtx);

       if (!dataQueue[printer].empty())
        {
            int data = dataQueue[printer].front();
            dataQueue[printer].pop();
            return data;
        }
    }
};

void printerThread(PrinterMonitor& printMonitor, int printer) 
{
    while (true) 
    {
        int printerData = printMonitor.getData(printer);
        printMonitor.printData(printer, printerData);
        printMonitor.releasePrinter(printer);
    }
}

void dataThread(PrinterMonitor& dataMonitor) 
{
    int data = 0;
    while (true) 
    {
        int printer = dataMonitor.getPrinter();
        dataMonitor.addData(printer, data);
        data++;
       
    }
}

int main() 
{
    PrinterMonitor monitor;

    thread printersThreads[NUM_PRINTERS];
    for (int i = 0; i < NUM_PRINTERS; i++)
    {
        printersThreads[i] = thread(printerThread, ref(monitor), i);
    }

    thread dataThreads(dataThread, ref(monitor));

    for (int i = 0; i < NUM_PRINTERS; i++)
    {
        printersThreads[i].join();
    }

    dataThreads.join();

    return 0;
}

