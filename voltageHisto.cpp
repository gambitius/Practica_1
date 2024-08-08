/*
 *  Baseline Voltage Histogram Generator by Charly
 *  Version: 1.2
 *  Author: Carlos Flores Melendez
 *  Date: January 2024
 *  Andres Bello University - SAPHIR
 *  Chile
 *
 * This code is a ROOT macro that generates a Baseline Voltage Histogram
 * using events stored in a set of txt files. 
 * 
 * In order to use this code, is necessary to run the csvRead.cpp macro first.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "TH1D.h"
#include "TCanvas.h"

using namespace std;

// EDITABLE Variables
//---------------------------------------------------------------------------------------------------------
// Folder path
string filefolder = "/home/martinus/Escritorio/ledchar/3V5_BLUE_LED"; 

// Number of Events
size_t numberOfEvents = 256;

// First Time Point for Histogram:
double minTimeValue = 3000;

// Last Time Point for Histogram:
double maxTimeValue = 4000;

//---------------------------------------------------------------------------------------------------------

int voltageHisto() {

    // Vectors to store data
    vector<double> timeWindow;
    vector<double> voltages;

    // Variables
    bool error = false;
    size_t numColumns = 0;
    size_t lineCount = 0;
    size_t resolution = 0;
    size_t eventNumber = 1;
    double timeValue = 0;
    double voltageValue = 0;
    double minVoltage = numeric_limits<double>::max();
    double maxVoltage = numeric_limits<double>::lowest();

    cout << " " << endl;
    cout << "--- Baseline Voltage Histogram Generator ---";
    cout << " " << endl;

    // Open file stream for Time window file and read data ------------------------------------------------
    string timeWindowFilename = (filefolder + "/txt/Time_Window.txt");
    ifstream timeWindowFile(timeWindowFilename);
    if (!timeWindowFile.is_open()) {
        cerr << "Error: Could not open file " << timeWindowFilename << endl;
        error = true;
        return 1;
    } else {
        cout << "Reading " << timeWindowFilename << " ..." << endl;
    }

    // Read Time Data
    while (timeWindowFile >> timeValue) {
        timeWindow.push_back(timeValue);
        lineCount++;
        //cout << timeValue << endl;
    }

    // Set resolution
    resolution = lineCount;
    cout << "Time window reading OK" << endl;
    cout << " " << endl;
    cout << "Resolution: " << resolution << endl;
    lineCount = 0;

    // Open file stream for every event file and read data ------------------------------------------------
    cout << " " << endl;
    cout << "Reading events in " << filefolder + "/txt/" << " ..." << endl;
    for(eventNumber = 1; eventNumber <= numberOfEvents; ++eventNumber){
        string eventFilename = (filefolder + "/txt/Event" + to_string(eventNumber) + ".txt");
        ifstream eventFile(eventFilename);
        if (!eventFile.is_open()) {
            cerr << "Error: Could not open file " << eventFilename << endl;
            error = true;
            return 3;
        } else {
            //cout << "Reading txt file: " << eventFilename << " ...";
        }

        // Read Voltage Data
        while (eventFile >> voltageValue) {
            if(lineCount >= minTimeValue && lineCount <= maxTimeValue){
                voltages.push_back(voltageValue);
                minVoltage = min(minVoltage, voltageValue);
                maxVoltage = max(maxVoltage, voltageValue);
            }
            lineCount++;
            //cout << voltageValue << endl;
        }

        // Error check
        if(lineCount != resolution){
            cerr << "Error: Line count not match in " << eventFilename << endl;
            cerr << "Line count: " << lineCount << endl;
            error = true;
            return 4;
        } else {
            //cout << "Event " << eventNumber <<  " reading OK" << endl;
        }
        lineCount = 0;

        //Print progress
        if (eventNumber % 1 == 0) {
            cout << "\rEvents Processed: " << eventNumber;
            cout.flush();
        }
    }
    if(!error){
        cout << " " << endl;
        cout << "Events reading OK" << endl;
        cout << " " << endl;
    }

    // Create Voltage Histogram ---------------------------------------------------------------------------

    // Calculate bin number
    int binNumber = 2*sqrt(voltages.size());
    cout << " " << endl;
    cout << "Bin number: " << binNumber << endl;
    cout << " " << endl;

    // Create a TCanvas
    TCanvas *canvas = new TCanvas("canvas", "Voltage Histogram", 1920, 1080);
    
    // New Histrogram
    auto h1 = new TH1D("Voltage", ("Baseline Histogram - " + filefolder.substr(filefolder.find_last_of("/") + 1)).c_str(), binNumber, minVoltage, maxVoltage);
    
    // Fill the histogram with voltage data
    for (double value : voltages) {
        h1->Fill(value);
    }

    // Draw the histogram on the canvas
    h1->Draw();

    // Set axis label
    h1->GetXaxis()->SetTitle(("Volts"));

    // Save the canvas as a PNG file
    string pngFilename;
    pngFilename = filefolder + "/images/BaselineVoltage_" + filefolder.substr(filefolder.find_last_of("/") + 1).c_str() + ".png";
    canvas->SaveAs(pngFilename.c_str());

    return 0;
}
